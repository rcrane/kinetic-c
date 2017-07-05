/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 *
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

#include "listener_helper.h"
#include "listener_task.h"
#include "listener_cmd.h"
#include "syscall.h"
#include "atomic.h"

#include <time.h>
#include <assert.h>

#ifdef TEST
uint8_t msg_buf[sizeof(uint8_t)];
#endif

listener_msg *ListenerHelper_GetFreeMsg(listener *l) {
    struct bus *b = l->bus;

    BUS_LOG_SNPRINTF(b, 4, LOG_LISTENER, b->udata, 128, "get_free_msg -- in use: %d", l->msgs_in_use);
    int loopcounter = 0;
    int16_t miu = 0;
    
    for (;;) {
        loopcounter++;
        miu = l->msgs_in_use;
        
        while(miu >= MAX_QUEUE_MESSAGES){
                     struct timespec ts = {
                     .tv_sec = 0,
                     .tv_nsec = 100L * miu,
                    };
                    //nanosleep(&ts, NULL);
                    //sched_yield();
                    miu = l->msgs_in_use;
                    loopcounter++;
        }

        listener_msg *head = l->msg_freelist;

        if (head == NULL) {
            //fprintf(stderr, "No free messages ListenerHelper_GetFreeMsg %d\n", loopcounter);
            //BUS_LOG(b, 3, LOG_LISTENER, "No free messages!", b->udata);
            return NULL;
            //sched_yield();
            
        } else if (ATOMIC_BOOL_COMPARE_AND_SWAP(&l->msg_freelist, head, head->next)) {

                if (ATOMIC_BOOL_COMPARE_AND_SWAP(&(head->type), MSG_NONE, MSG_HEADINUSE)){
                        __sync_fetch_and_add(&l->msgs_in_use, 1);
                        //BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 64, "got free msg: %u", head->id);

                        BUS_ASSERT(b, b->udata, head->type == MSG_HEADINUSE);
                        memset(&head->u, 0, sizeof(head->u));

                        if(loopcounter > 20){
                            fprintf(stderr, "Contention in ListenerHelper_GetFreeMsg %d\n", loopcounter);
                        }
                        return head;
                }

                //sched_yield();
                
        }
    }
}

bool ListenerHelper_PushMessage(struct listener *l, listener_msg *msg, int *reply_fd) {
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, msg);

    #ifndef TEST
    uint8_t msg_buf[sizeof(uint8_t)];
    #endif
    msg_buf[0] = msg->id;

    if (reply_fd) { *reply_fd = msg->pipes[0]; }

    ListenerCmd_msg_handler(l, msg);
    return true;
}

rx_info_t *ListenerHelper_GetFreeRXInfo(struct listener *l) {
    struct bus *b = l->bus;
    struct rx_info_t *head = NULL;

    for(;;){

        head = l->rx_info_freelist;
    
        if (head == NULL) {
            BUS_LOG(b, 6, LOG_SENDER, "No rx_info cells left!", b->udata);
            fprintf(stderr, "No free messages!\n");
            struct timespec ts = {
                     .tv_sec = 0,
                     .tv_nsec = 100L * l->rx_info_in_use,
            };
            //nanosleep(&ts, NULL);

        } else if(ATOMIC_BOOL_COMPARE_AND_SWAP(&l->rx_info_freelist, head, head->next)) {
            __sync_synchronize(); // not sure if this is really necessary
            BUS_ASSERT(b, b->udata, head->state == RIS_INACTIVE);
            assert(ATOMIC_BOOL_COMPARE_AND_SWAP(&(head->state), RIS_INACTIVE, RIS_HEADINUSE));
            head->next = NULL;
            __sync_fetch_and_add(&l->rx_info_in_use, 1);

            BUS_LOG(l->bus, 4, LOG_LISTENER, "reserving RX info", l->bus->udata);

            if (l->rx_info_max_used < head->id) {
                BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "rx_info_max_used <- %d", head->id);
                l->rx_info_max_used = head->id;
                BUS_ASSERT(b, b->udata, l->rx_info_max_used < MAX_PENDING_MESSAGES);
            }

            BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "got free rx_info_t %d (%p)", head->id, (void *)head);
            BUS_ASSERT(b, b->udata, head == &l->rx_info[head->id]);

            return head;
        }
    }
}


rx_info_t *ListenerHelper_FindInfoBySequenceID(listener *l, int fd, int64_t seq_id, int wanted_state) {
    
    struct bus *b = l->bus;
    for (int i = 0; i <= MAX_PENDING_MESSAGES; i++) { // the max used counter may not be trustworthy

        rx_info_t *info = &l->rx_info[i];

        if( ( (wanted_state > -1) && (info->state == wanted_state) ) || (wanted_state == -1) ){ // increase efficiency

        switch (info->state) {
        case RIS_HEADINUSE:
        case RIS_INACTIVE:
            break;            /* skip */
        case RIS_HOLD:
        {
            BUS_LOG_SNPRINTF(b, 4, LOG_MEMORY, b->udata, 128,  "find_info_by_sequence_id: info (%p) at +%d: <fd:%d, seq_id:%lld>", (void*)info, info->id, fd, (long long)seq_id);

            if (info->u.hold.fd == fd && info->u.hold.seq_id == seq_id) {
                return info;
            }
            break;
        }
        case RIS_EXPECT:
        {
            if(info->u.expect.box){
                struct boxed_msg *box = info->u.expect.box;
                BUS_LOG_SNPRINTF(b, 4, LOG_MEMORY, b->udata, 128, "find_info_by_sequence_id: info (%p) at +%d [s %d]: box is %p", (void*)info, info->id, info->u.expect.error, (void*)box);
            
                if (box != NULL && box->out_seq_id == seq_id && box->fd == fd) {
                    return info;
                }
            }
            break;
        }
            
        default:
            break;
//            BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 64, "match fail %d on line %d", info->state, __LINE__);
//            BUS_ASSERT(b, b->udata, false);
        }
        }
    }

    if(seq_id > 0){
        // seq_id may be -1. Those info are never in the list.
        BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 64, "match fail for %p %d %ld on line %d", l, fd, seq_id , __LINE__);
    }

    if (b->log_level > 5 || 0) {
        BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 64,
            "==== Could not find <fd:%d, seq_id:%lld>, dumping table ====\n",
            fd, (long long)seq_id);
        ListenerTask_DumpRXInfoTable(l);
    }

    /* Not found. Probably an unsolicited status message. */
    return NULL;
}
