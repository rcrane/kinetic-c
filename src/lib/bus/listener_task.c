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

#include "listener_task.h"
#include "listener_task_internal.h"
#include "util.h"
#include "syscall.h"
#include <sys/prctl.h>

#include <assert.h>
#include "listener_cmd.h"
#include "listener_io.h"
#include "atomic.h"

//#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef TEST
    struct timeval now;
    struct timeval cur;
    size_t backpressure = 0;
    int poll_res = 0;
    #define WHILE if
#else
    #define WHILE while
#endif

static void tick_handler(listener *l);
static void clean_up_completed_info(listener *l, rx_info_t *info);
static void retry_delivery(listener *l, rx_info_t *info);
static void observe_backpressure(listener *l, size_t backpressure);
static void ListenerTask_CountMessages(struct listener* l);

void *ListenerTask_MainLoop(void *arg) {
    
    listener *self = (listener *)arg;
    assert(self);
    struct bus *b = self->bus;
    prctl (PR_SET_NAME, "kinlistener", 0, 0, 0);
    #ifndef TEST
    struct timeval now;
    #endif
    time_t last_sec = (time_t)-1;  // always trigger first time

    /* The listener thread has full control over its execution -- the
     * only thing other threads can do is reserve messages from l->msgs,
     * write commands into them, and then commit them by writing their
     * msg->id into the incoming command ID pipe. All cross-thread
     * communication is managed at the command interface, so it doesn't
     * need any internal locking. */

	int sdnfd = 0;
    __atomic_load(&(self->shutdown_notify_fd), &sdnfd, __ATOMIC_RELAXED);
    int maxiterations = 1000/LISTENER_TASK_TIMEOUT_DELAY;
    int iterations = 0;

    WHILE (sdnfd == LISTENER_NO_FD) {
        iterations++;
        
        if (iterations >= maxiterations) {
            iterations = 0;
            tick_handler(self);
        }

        bool idle = true;
         __atomic_load(&(self->is_idle), &idle, __ATOMIC_RELAXED);
        int delay = (idle ? INFINITE_DELAY : LISTENER_TASK_TIMEOUT_DELAY);

        #ifndef TEST
        int poll_res = 0;
        #endif

        uint16_t to_poll = 0;
        __atomic_load(&(self->tracked_fds), &to_poll, __ATOMIC_RELAXED);

        to_poll = to_poll - self->inactive_fds + INCOMING_MSG_PIPE;

        poll_res = syscall_poll(self->fds, to_poll, delay); // atomic fds

        BUS_LOG_SNPRINTF(b, (poll_res == 0 ? 6 : 4), LOG_LISTENER, b->udata, 64, "poll res %d", poll_res);

        if (poll_res < 0) {
            if (Util_IsResumableIOError(errno)) {
                errno = 0;
            } else {
                /* unrecoverable poll error -- FD count is bad
                 * or FDS is a bad pointer. */
                BUS_ASSERT(b, b->udata, false);
            }
        } else if (poll_res > 0) {
            ListenerCmd_CheckIncomingMessages(self, &poll_res);
            if (poll_res > 0) {
                ListenerIO_AttemptRecv(self, poll_res);
            }
        } else {
            /* nothing to do */
        }
        __atomic_load(&(self->shutdown_notify_fd), &sdnfd, __ATOMIC_RELAXED);
    }

    __atomic_load(&(self->shutdown_notify_fd), &sdnfd, __ATOMIC_RELAXED);
    /* (This will always be true, except when testing.) */
    if (sdnfd != LISTENER_NO_FD) {
        BUS_LOG(b, 3, LOG_LISTENER, "shutting down", b->udata);

        if (self->tracked_fds > 0) {
            BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 64,
                "%d connections still open!", self->tracked_fds);
        }
    
        ListenerCmd_NotifyCaller(self, self->shutdown_notify_fd);
        sdnfd = LISTENER_SHUTDOWN_COMPLETE_FD;
        __atomic_store(&(self->shutdown_notify_fd), &sdnfd, __ATOMIC_RELAXED);
        
    }

    BUS_LOG(b, 3, LOG_LISTENER, "shutting down...", b->udata);
    return NULL;
}

static void tick_handler(listener *l) {
    struct bus *b = l->bus;
    bool any_work = false;

    BUS_LOG_SNPRINTF(b, 2, LOG_LISTENER, b->udata, 128,
        "tick... %p: %d of %d msgs in use, %d of %d rx_info in use, %d tracked_fds",
        (void*)l, l->msgs_in_use, MAX_QUEUE_MESSAGES,
        l->rx_info_in_use, MAX_PENDING_MESSAGES, l->tracked_fds);

    if (b->log_level > 5) {  /* dump msg table types */
        for (int i = 0; i < l->msgs_in_use; i++) {
            printf(" -- msg %d: type %d\n", i, l->msgs[i].type);
        }
    }

    if (b->log_level > 5 || 0) { ListenerTask_DumpRXInfoTable(l); }

    int rxs = -5;
    rx_info_state currentstate = RIS_INACTIVE;
    //rx_info_max_used should probably be accessed atomically
    for (int i = 0; i <= l->rx_info_max_used; i++) {
        rx_info_t *info = &l->rx_info[i];

        __atomic_load(&(info->state), &currentstate, __ATOMIC_RELAXED);

        switch (currentstate) {
        case RIS_HEADINUSE:
        case RIS_INACTIVE:
            break;
        case RIS_HOLD:
            any_work = true;
            /* Check timeout */
            if (info->timeout_sec == 1) {
                fprintf(stderr, "Hold Timeout -> ListenerTask_ReleaseRXInfo %d %lld %d\n",  info->u.hold.fd, (long long)info->u.hold.seq_id, info->u.hold.error );
                ListenerTask_ReleaseRXInfo(l, info);
            } else {
                BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 64,
                    "decrementing countdown on info %p [%u]: %ld",
                    (void*)info, info->id, info->timeout_sec - 1);
		
                info->timeout_sec--;
            }
            break;
        case RIS_EXPECT:
            any_work = true;
            __atomic_load(&(info->u.expect.error), &rxs, __ATOMIC_RELAXED);
	        assert(rxs != -5);
	        if (rxs == RX_ERROR_READY_FOR_DELIVERY) {
                BUS_LOG(b, 4, LOG_LISTENER, "retrying RX event delivery", b->udata);
                fprintf(stderr, "trying RX delivery: %p\n",info->u.expect.box);
                retry_delivery(l, info);
	        } else if (rxs == RX_ERROR_DELIVERING){
	   	    // not doing anything
            } else if (rxs == RX_ERROR_DONE) {
                BUS_LOG_SNPRINTF(b, 4, LOG_LISTENER, b->udata, 64, "cleaning up completed RX event at info %p", (void*)info);
                clean_up_completed_info(l, info);
            } else if (rxs != RX_ERROR_NONE) {
                BUS_LOG_SNPRINTF(b, 1, LOG_LISTENER, b->udata, 64, "notifying of rx failure -- error %d (info %p)", info->u.expect.error, (void*)info);
                ListenerTask_NotifyMessageFailure(l, info, BUS_SEND_RX_FAILURE);
            } else if (info->timeout_sec == 1) {
                
                struct boxed_msg *box = info->u.expect.box;
                
                (void)box;
                
                //fprintf(stderr, "EXPECT Timeout -> ListenerTask_ReleaseRXInfo %d %lld %d %s\n", info->u.hold.fd, (long long)info->u.hold.seq_id, info->u.hold.error , (char*)info->u.expect.box->out_msg);
                ListenerTask_NotifyMessageFailure(l, info, BUS_SEND_RX_TIMEOUT);
            } else {
                BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 64,
                    "decrementing countdown on info %p [%u]: %ld",
                    (void*)info, info->id, info->timeout_sec - 1);

                info->timeout_sec--;
            }
            break;
        default:
            BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 64, "match fail %d on line %d", info->state, __LINE__);
            BUS_ASSERT(b, b->udata, false);
        }
    }
    if (!any_work) { 
    	__sync_bool_compare_and_swap(&(l->is_idle), false, true);
    }
}

void ListenerTask_DumpRXInfoTable(listener *l) {
    for (int i = 0; i <= l->rx_info_max_used; i++) {
        rx_info_t *info = &l->rx_info[i];

        printf(" -- state: %d, info[%d]: timeout %ld",
            info->state, info->id, info->timeout_sec);
        switch (l->rx_info[i].state) {
        
        case RIS_HOLD:
            printf(", fd %d, seq_id %lld, has_result? %d\n",
                info->u.hold.fd, (long long)info->u.hold.seq_id, info->u.hold.has_result);
            break;
        case RIS_EXPECT:
        {
            struct boxed_msg *box = info->u.expect.box;
            printf(", box %p (fd:%d, seq_id:%lld), error %d, has_result? %d\n",
                (void *)box, box ? box->fd : -1, box ? (long long)box->out_seq_id : -1,
                info->u.expect.error, info->u.expect.has_result);
            break;
        }
        case RIS_INACTIVE:
            printf(", INACTIVE (next: %d)\n", info->next ? info->next->id : -1);
            break;
        
        case RIS_HEADINUSE:
            printf(", INUSE (next: %d)\n", info->next ? info->next->id : -1);
            break;
        }
    }
}

static void retry_delivery(listener *l, rx_info_t *info) {
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, info->state == RIS_EXPECT);
    //info->state = RIS_HEADINUSE; pseudo lock
    BUS_ASSERT(b, b->udata, info->u.expect.error == RX_ERROR_READY_FOR_DELIVERY);
    BUS_ASSERT(b, b->udata, info->u.expect.box);

    struct boxed_msg *box = info->u.expect.box;
    info->u.expect.box = NULL;       /* release */

    BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "releasing box %p at line %d", (void*)box, __LINE__);
    BUS_ASSERT(b, b->udata, box->result.status == BUS_SEND_SUCCESS);

    #ifndef TEST
    size_t backpressure = 0;
    #endif

    if (Bus_ProcessBoxedMessage(l->bus, box, &backpressure)) {
        info->u.expect.box = NULL;
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "successfully delivered box %p (seq_id %lld) from info %d at line %d (retry)",
            (void*)box, (long long)box->out_seq_id, info->id, __LINE__);

        info->u.expect.error = RX_ERROR_DONE;
        ListenerTask_ReleaseRXInfo(l, info);
    } else {
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "returning box %p at line %d", (void*)box, __LINE__);
        assert(box);
        info->u.expect.box = box;    /* retry in tick_handler */
    }

    observe_backpressure(l, backpressure);
    //info->state = RIS_EXPECT;
}

static void clean_up_completed_info(listener *l, rx_info_t *info) {
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, info->state == RIS_EXPECT || info->state == RIS_HEADINUSE);
    BUS_ASSERT(b, b->udata, info->u.expect.error == RX_ERROR_DONE);

    BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "info %p, box is %p at line %d", (void*)info, (void*)info->u.expect.box, __LINE__);

    #ifndef TEST
    size_t backpressure = 0;
    #endif

    if (info->u.expect.box) {

        struct boxed_msg *box = info->u.expect.box;
        
        if (box->result.status != BUS_SEND_SUCCESS) {
            printf("*** info %d: info->timeout %ld\n",
                info->id, info->timeout_sec);
            printf("    info->error %d\n", info->u.expect.error);
            printf("    info->box == %p\n", (void*)box);
            printf("    info->box->result.status == %d\n", box->result.status);
            printf("    info->box->fd %d\n", box->fd);
            printf("    info->box->out_seq_id %lld\n", (long long)box->out_seq_id);
            printf("    info->box->out_msg %p\n", (void*)box->out_msg);

        }

        BUS_ASSERT(b, b->udata, box->result.status == BUS_SEND_SUCCESS);
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "releasing box %p at line %d", (void*)box, __LINE__);

        info->u.expect.box = NULL;       /* release */

        if (Bus_ProcessBoxedMessage(l->bus, box, &backpressure)) {
            info->u.expect.box = NULL;
            ListenerTask_ReleaseRXInfo(l, info);
        } else {
            BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "returning box %p at line %d", (void*)box, __LINE__);
            assert(box);

            //RIS_HEADINUSE --> RIS_EXPECT
            rx_info_state currentstate = RIS_EXPECT; __atomic_store(&(info->state), &currentstate, __ATOMIC_RELAXED);

            info->u.expect.box = box;    /* retry in tick_handler */
        }
    } else {                    /* already processed, just release it */
        ListenerTask_ReleaseRXInfo(l, info);
    }

    observe_backpressure(l, backpressure);
}

void ListenerTask_NotifyMessageFailure(listener *l, rx_info_t *info, bus_send_status_t status) {
    #ifndef TEST
    size_t backpressure = 0;
    #endif
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, info->state == RIS_EXPECT);
    BUS_ASSERT(b, b->udata, info->u.expect.box);

    BUS_ASSERT(b, b->udata, status != BUS_SEND_UNDEFINED);
    info->u.expect.box->result.status = status;

    boxed_msg *box = info->u.expect.box;
    info->u.expect.box = NULL;
    BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "releasing box %p at line %d", (void*)box, __LINE__);

    if (Bus_ProcessBoxedMessage(l->bus, box, &backpressure)) {
        info->u.expect.box = NULL;
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "delivered box %p with failure message %d at line %d (info %p)", (void*)box, status, __LINE__, (void*)info);
        info->u.expect.error = RX_ERROR_DONE;
        ListenerTask_ReleaseRXInfo(l, info);
    } else {
        /* RetuBus_RegisterSocket to info, will be released on retry. */
        assert(box);
        info->u.expect.box = box;
    }

    observe_backpressure(l, backpressure);
}

static connection_info *get_connection_info(struct listener *l, int fd) {
    struct bus *b = l->bus;
    for (int i = 0; i < l->tracked_fds; i++) {
        connection_info *ci = l->fd_info[i];
        BUS_ASSERT(b, b->udata, ci);
        if (ci->fd == fd) { return ci; }
    }
    return NULL;
}


void ListenerTask_ReleaseRXInfo(struct listener *l, rx_info_t *info) {
    //ListenerTask_CountMessages(l);
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, info);
    BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128,"releasing RX info %d (%p), state %d", info->id, (void *)info, info->state);
    BUS_ASSERT(b, b->udata, info->id < MAX_PENDING_MESSAGES);
    BUS_ASSERT(b, b->udata, info == &l->rx_info[info->id]);

    rx_info_state newstate = RIS_HEADINUSE;
    rx_info_state oldstate = RIS_INACTIVE;
    __atomic_load(&(info->state), &oldstate, __ATOMIC_RELAXED);
    __atomic_store(&(info->state), &newstate, __ATOMIC_RELAXED); // pseudo lock
    
    switch (oldstate) {
    case RIS_HOLD:
    	
        BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, " -- releasing HOLD: has result? %d", info->u.hold.has_result);
        if (info->u.hold.has_result) {
            /* If we have a message that timed out, we need to free it,
             * but don't know how. We should never get here, because it
             * means the client finished sending the message, but the
             * listener never got the handler callback. */

            if (info->u.hold.result.ok) {
                void *msg = info->u.hold.result.u.success.msg;
                int64_t seq_id = info->u.hold.result.u.success.seq_id;

                connection_info *ci = get_connection_info(l, info->u.hold.fd);
                if (ci && b->unexpected_msg_cb) {
                    BUS_LOG_SNPRINTF(b, 1, LOG_LISTENER, b->udata, 128, "CALLING UNEXPECTED_MSG_CB ON RESULT %p", (void *)&info->u.hold.result);
                    b->unexpected_msg_cb(msg, seq_id, b->udata, ci->udata);
                } else {
                    BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 128, "LEAKING RESULT %p", (void *)&info->u.hold.result);
                }
            }
        }
        break;
        
    case RIS_EXPECT:
        if(info->u.expect.error == RX_ERROR_NONE && info->u.expect.box != NULL){
            // Not sure, listenerTask thinks it is RIS_HOLD but when we get here it is EXPECT
            // Concurrency bug: Multiple listenerTasks on the same msg.
            // But this message back to prior state and hope for the best
            __atomic_store(&(info->state), &oldstate, __ATOMIC_RELAXED); // pseudo lock
            return;
        }
    case RIS_HEADINUSE: // see  ListenerTask_AttemptDelivery (line 613)
        BUS_ASSERT(b, b->udata, info->u.expect.error == RX_ERROR_DONE);
        BUS_ASSERT(b, b->udata, info->u.expect.box == NULL);
        break;
    default:
    case RIS_INACTIVE:
        BUS_ASSERT(b, b->udata, false);
    }

    /* Set to no longer active and push on the freelist. */
    BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "releasing rx_info_t %d (%p), was %d", info->id, (void *)info, info->state);
    BUS_ASSERT(b, b->udata, info->state != RIS_INACTIVE);

    memset(&info->u, 0, sizeof(info->u));


    // small integrity check:
    assert(l->rx_info_freelist->id < MAX_PENDING_MESSAGES);
    //assert(l->rx_info_freelist->next->id < MAX_PENDING_MESSAGES);
    
    newstate = RIS_INACTIVE;
    __atomic_store(&(info->state), &newstate, __ATOMIC_RELAXED); // pseudo lock
    
    do{
        info->next = l->rx_info_freelist;
        assert(info->next->id < MAX_PENDING_MESSAGES);
    }while( false == (ATOMIC_BOOL_COMPARE_AND_SWAP(&l->rx_info_freelist, info->next, info)));

    // small integrity check:
    assert(l->rx_info_freelist->id < MAX_PENDING_MESSAGES);
    //assert(l->rx_info_freelist->next->id < MAX_PENDING_MESSAGES);
    
    if(info->id > 0 && l->rx_info_max_used == info->id){
        ATOMIC_BOOL_COMPARE_AND_SWAP(&l->rx_info_max_used, info->id, info->id-1);
        // This needs improvement! See below:
    }
    //fprintf(stderr, "rx_info_max_used %d\n",l->rx_info_max_used);
/*
    if (l->rx_info_max_used == info->id && info->id > 0) {
        BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "rx_info_max_used--, from %d to %d", l->rx_info_max_used, l->rx_info_max_used - 1);
        while (l->rx_info[l->rx_info_max_used].state == RIS_INACTIVE) {
            __sync_fetch_and_sub(&l->rx_info_max_used, 1); // l->rx_info_max_used--;
            if (l->rx_info_max_used == 0) { 
                break;
            }
        }
        BUS_ASSERT(b, b->udata, l->rx_info_max_used < MAX_PENDING_MESSAGES);
    }
*/
    
    __sync_fetch_and_sub(&l->rx_info_in_use, 1);//    l->rx_info_in_use--;
}

static void ListenerTask_CountMessages(struct listener* l){
    int count = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;

    rx_info_t *info = l->rx_info_freelist;
    
    if(info){
	    do{
	    	fprintf(stderr, "%d-", info->id);
	        info = info->next;
	        count++;
	    }while(info != NULL && info != l->rx_info_freelist);
	    fprintf(stderr, "\n\n");
	}

    for (int i = 0; i < MAX_PENDING_MESSAGES ; i++) {
    	info = &(l->rx_info[i]);
    	if(info && info->state != 3){
       		fprintf(stderr, "%d-", info->id);
	      	count2++;
	  	}
	}
	fprintf(stderr, "\n\n");    
	

    listener_msg* msg = l->msg_freelist;
    
    if(msg){
		do{
			fprintf(stderr, "%d-", msg->id);
	        msg = msg->next;
	        count4++;
	    }while(msg != l->msg_freelist && msg != NULL);
	    fprintf(stderr, "\n\n");
	}


	for (int i = 0; i < MAX_QUEUE_MESSAGES ; i++) {
    	msg = &(l->msgs[i]);
    	if(msg && msg->type != 0){
			fprintf(stderr, "%d-%d--", msg->id, msg->type);    
	        count3++;
	    }
	}
	fprintf(stderr, "\n\n");  

    if(count+count4 != MAX_QUEUE_MESSAGES+MAX_PENDING_MESSAGES) {
    	fprintf(stderr, "Current count: %d+%d+%d+%d=%d\n", count, count2, count3, count4, count+count2+count3+count4);
    }
    
}

void ListenerTask_ReleaseMsg(struct listener *l, listener_msg *msg) {
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, msg->id < MAX_QUEUE_MESSAGES);
    //fprintf(stderr, "ListenerTask_ReleaseMsg: %p\n", msg);
    msg->type = MSG_NONE;

    for (;;) {
        listener_msg *fl = l->msg_freelist;
        msg->next = fl;
        if (ATOMIC_BOOL_COMPARE_AND_SWAP(&l->msg_freelist, fl, msg)) {
                __sync_fetch_and_sub(&l->msgs_in_use, 1);
                BUS_ASSERT(b, b->udata, l->msgs_in_use >= 0);
                BUS_LOG(b, 3, LOG_LISTENER, "Releasing msg", b->udata);
                return;
        }
    }
}

bool ListenerTask_GrowReadBuf(listener *l, size_t nsize) {
    if (nsize < l->read_buf_size) { return true; }

    uint8_t *nbuf = realloc(l->read_buf, nsize);
    if (nbuf) {
        struct bus *b = l->bus;
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128,
            "Read buffer realloc success, %p (%zd) to %p (%zd)",
            l->read_buf, l->read_buf_size,
            nbuf, nsize);
        l->read_buf = nbuf;
        l->read_buf_size = nsize;
        return true;
    } else {
        return false;
    }
}

void ListenerTask_AttemptDelivery(listener *l, struct rx_info_t *info) {
    struct bus *b = l->bus;
    //fprintf(stderr, "attmpt delivery: %p\n",info->u.expect.box);
    struct boxed_msg *box = info->u.expect.box;
    assert(box);
    info->u.expect.box = NULL;  /* release */

    BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 64, "attempting delivery of %p", (void*)box);
    BUS_LOG_SNPRINTF(b, 5, LOG_MEMORY, b->udata, 128, "releasing box %p at line %d", (void*)box, __LINE__);

    bus_msg_result_t *result = &box->result;

    if (result->status != BUS_SEND_SUCCESS) {
    
	    if(false == __sync_bool_compare_and_swap(&(result->status), BUS_SEND_REQUEST_COMPLETE, BUS_SEND_SUCCESS)){
	    	fprintf(stderr, "unexpected status for completed RX event at info: %d\n", result->status);
    	    BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 128, "unexpected status for completed RX event at info +%d, box %p, status %d", info->id, (void *)box, result->status);
    	}
    }

    bus_unpack_cb_res_t unpacked_result = { .ok = false, .u = 0, };
    
    switch (info->state) {
	    case RIS_EXPECT:
	        BUS_ASSERT(b, b->udata, info->u.expect.has_result);
	        unpacked_result = info->u.expect.result;
	        break;
	    default:
	    case RIS_HOLD:
	    case RIS_INACTIVE:
	        BUS_ASSERT(b, b->udata, false);
   }
   
    rx_info_state newstate = RIS_HEADINUSE; __atomic_store(&(info->state), &newstate, __ATOMIC_RELAXED); 
	       
    BUS_ASSERT(b, b->udata, unpacked_result.ok);
    int64_t seq_id = unpacked_result.u.success.seq_id;
    void *opaque_msg = unpacked_result.u.success.msg;
    result->u.response.seq_id = seq_id;
    result->u.response.opaque_msg = opaque_msg;

    #ifndef TEST
    size_t backpressure = 0;
    #endif
    if (Bus_ProcessBoxedMessage(b, box, &backpressure)) {
        /* success */
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 256, "successfully delivered box %p (seq_id:%lld), marking info %d as DONE", (void*)box, (long long)seq_id, info->id);
        int s = RX_ERROR_DONE; __atomic_store(&(info->u.expect.error), &s, __ATOMIC_RELAXED);
        BUS_LOG_SNPRINTF(b, 4, LOG_LISTENER, b->udata, 128, "initial clean-up attempt for completed RX event at info +%d", info->id);
        clean_up_completed_info(l, info);
        info = NULL; /* drop out of scope, likely to be stale */
    } else {
        BUS_LOG_SNPRINTF(b, 3, LOG_MEMORY, b->udata, 128, "returning box %p at line %d", (void*)box, __LINE__);
	    int s = RX_ERROR_READY_FOR_DELIVERY; __atomic_store(&(info->u.expect.error), &s, __ATOMIC_RELAXED);
        rx_info_state newstate = RIS_EXPECT; __atomic_store(&(info->state), &newstate, __ATOMIC_RELAXED); 
        info->u.expect.box = box; /* retry in tick_handler */
    }
    observe_backpressure(l, backpressure);
}

static void observe_backpressure(listener *l, size_t backpressure) {
    size_t cur = l->upstream_backpressure;
    l->upstream_backpressure = (cur + backpressure) / 2;
}

// might not be used anymore
uint16_t ListenerTask_GetBackpressure(struct listener *l) {
    uint16_t msg_fill_pressure = 0;
    
    if (l->msgs_in_use < 0.25 * MAX_QUEUE_MESSAGES) {
        msg_fill_pressure = 0;
    } else if (l->msgs_in_use < 0.5 * MAX_QUEUE_MESSAGES) {
        msg_fill_pressure = MSG_BP_1QTR * 2 * l->msgs_in_use;
    } else if (l->msgs_in_use < 0.75 * MAX_QUEUE_MESSAGES) {
        msg_fill_pressure = MSG_BP_HALF * 10 * l->msgs_in_use;
    } else {
        msg_fill_pressure = MSG_BP_3QTR * 100 * l->msgs_in_use;
    }

    uint16_t rx_info_fill_pressure = 0;
    if (l->rx_info_in_use < 0.25 * MAX_PENDING_MESSAGES) {
        rx_info_fill_pressure = 0;
    } else if (l->rx_info_in_use < 0.5 * MAX_PENDING_MESSAGES) {
        rx_info_fill_pressure = RX_INFO_BP_1QTR * l->rx_info_in_use;
    } else if (l->rx_info_in_use < 0.75 * MAX_PENDING_MESSAGES) {
        rx_info_fill_pressure = RX_INFO_BP_HALF * l->rx_info_in_use;
    } else {
        rx_info_fill_pressure = RX_INFO_BP_3QTR * l->rx_info_in_use;
    }

    uint16_t threadpool_fill_pressure = THREADPOOL_BP * l->upstream_backpressure;

    struct bus *b = l->bus;
    BUS_LOG_SNPRINTF(b, 6, LOG_SENDER, b->udata, 64,
        "lbp: %u, %u (iu %u), %u",
        msg_fill_pressure, rx_info_fill_pressure, l->rx_info_in_use, threadpool_fill_pressure);

    return msg_fill_pressure + rx_info_fill_pressure  + threadpool_fill_pressure;
}
