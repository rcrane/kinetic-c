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

#include <unistd.h>
#include <err.h>
#include <assert.h>
#include "syscall.h"
#include <string.h>
#include "atomic.h"

#include "listener_cmd.h"
#include "listener_cmd_internal.h"
#include "listener_task.h"
#include "listener_helper.h"

static void add_socket(listener *l, connection_info *ci, int notify_fd);
static void remove_socket(listener *l, int fd, int notify_fd);
static void hold_response(listener *l, int fd, int64_t seq_id, int16_t timeout_sec, int notify_fd);
static void expect_response(listener *l, boxed_msg *box);
static void shutdown(listener *l, int notify_fd);

#ifdef TEST
uint8_t reply_buf[sizeof(uint8_t) + sizeof(uint16_t)];
uint8_t cmd_buf[LISTENER_CMD_BUF_SIZE];
#endif

void ListenerCmd_NotifyCaller(listener *l, int fd) {

    //this method is used for thread sync during connection setup

    if (fd == -1) { return; }
    #ifndef TEST
    uint8_t reply_buf[sizeof(uint8_t) + sizeof(uint16_t)];
    #endif
    reply_buf[0] = LISTENER_MSG_TAG;

    /* reply_buf[1:2] is little-endian backpressure.  */
    uint16_t backpressure = ListenerTask_GetBackpressure(l);
    reply_buf[1] = (uint8_t)(backpressure & 0xff);
    reply_buf[2] = (uint8_t)((backpressure >> 8) & 0xff);
    struct bus *b = l->bus;

    for (;;) {
        BUS_LOG_SNPRINTF(b, 6, LOG_LISTENER, b->udata, 128,
            "NotifyCaller on %d with backpressure %u",
            fd, backpressure);;

        ssize_t wres = syscall_write(fd, reply_buf, sizeof(reply_buf));
        if (wres == sizeof(reply_buf)) { break; }
        if (wres == -1) {
            if (errno == EINTR) {
                errno = 0;
                continue;
            } else {
                err(1, "write");
            }
        }
    }
}

void ListenerCmd_CheckIncomingMessages(listener *l, int *res) {
    struct bus *b = l->bus;
    short events = l->fds[INCOMING_MSG_PIPE_ID].revents;

    if (events & (POLLERR | POLLHUP | POLLNVAL)) {  /* hangup/error */
        BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 128,
            "hangup on listener incoming command pipe: %d", events);
        return;
    }

    if (events & POLLIN) {
        #ifndef TEST
        char cmd_buf[LISTENER_CMD_BUF_SIZE];
        #endif
        for (;;) {
            ssize_t rd = syscall_read(l->fds[INCOMING_MSG_PIPE_ID].fd, cmd_buf, sizeof(cmd_buf));
            if (rd == -1) {
                if (errno == EINTR) {
                    errno = 0;
                    continue;
                } else {
                    BUS_LOG_SNPRINTF(b, 6, LOG_LISTENER, b->udata, 128,
                        "check_and_flush_incoming_msg_pipe: %s", strerror(errno));
                    errno = 0;
                    break;
                }
            } else {
                for (ssize_t i = 0; i < rd; i++) {
                    uint8_t msg_id = cmd_buf[i];
                    listener_msg *msg = &l->msgs[msg_id];
                    assert(msg);
                    ListenerCmd_msg_handler(l, msg);
                    fprintf(stderr, "ListenerCmd_CheckIncomingMessages\n");
                    //ListenerTask_ReleaseMsg(l, msg);
                }
                (*res)--;
                break;
            }
        }
    }
}

void ListenerCmd_msg_handler(listener *l, listener_msg *pmsg) {
    struct bus *b = l->bus;
    // fprintf(stderr, "ListenerCmd_msg_handler: %p\n", pmsg);
    BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 128, "Handling message -- %p, type %d", (void*)pmsg, pmsg->type);

    __sync_bool_compare_and_swap(&(l->is_idle), true, false);

    listener_msg msg = *pmsg;
    switch (msg.type) {

    case MSG_ADD_SOCKET:
        add_socket(l, msg.u.add_socket.info, msg.u.add_socket.notify_fd);
        break;
    case MSG_REMOVE_SOCKET:
        remove_socket(l, msg.u.remove_socket.fd, msg.u.remove_socket.notify_fd);
        break;
    case MSG_HOLD_RESPONSE:
        hold_response(l, msg.u.hold.fd, msg.u.hold.seq_id,
            msg.u.hold.timeout_sec, msg.u.hold.notify_fd);
        break;
    case MSG_EXPECT_RESPONSE:
        expect_response(l, msg.u.expect.box);
        break;
    case MSG_SHUTDOWN:
        shutdown(l, msg.u.shutdown.notify_fd);
        break;

    case MSG_NONE:
        break; // TODO
    default:
        BUS_ASSERT(b, b->udata, false);
        break;
    }
    ListenerTask_ReleaseMsg(l, pmsg); 
    //moved to ListenerCmd_CheckIncomingMessages
}

/* Swap poll and connection info for tracked sockets, by array offset. */
static void swap(listener *l, int a, int b) {
    struct pollfd a_pfd = l->fds[a + INCOMING_MSG_PIPE];
    struct pollfd b_pfd = l->fds[b + INCOMING_MSG_PIPE];
    connection_info *a_ci = l->fd_info[a];
    connection_info *b_ci = l->fd_info[b];

    l->fds[b + INCOMING_MSG_PIPE] = a_pfd;
    l->fds[a + INCOMING_MSG_PIPE] = b_pfd;

    l->fd_info[a] = b_ci;
    l->fd_info[b] = a_ci;
}

static void add_socket(listener *l, connection_info *ci, int notify_fd) {
    /* TODO: if epoll, just register with the OS. */
    struct bus *b = l->bus;
    BUS_LOG(b, 3, LOG_LISTENER, "adding socket", b->udata);

    if (l->tracked_fds == MAX_FDS) {
        /* error: full */
        BUS_LOG(b, 3, LOG_LISTENER, "FULL", b->udata);
        ListenerCmd_NotifyCaller(l, notify_fd);
        return;
    }
    for (int i = 0; i < l->tracked_fds; i++) {
        if (l->fds[i + INCOMING_MSG_PIPE].fd == ci->fd) {
            free(ci);
            ListenerCmd_NotifyCaller(l, notify_fd);
            return;             /* already present */
        }
    }

    int id = l->tracked_fds;
    l->fd_info[id] = ci;
    l->fds[id + INCOMING_MSG_PIPE].fd = ci->fd; //atomic fds
    l->fds[id + INCOMING_MSG_PIPE].events = POLLIN;

    /* If there are any inactive FDs, we need to swap the new last FD
     * and the first inactive FD so that the active and inactive FDs
     * remain contiguous. */
    if (l->inactive_fds > 0) {
        int first_inactive = l->tracked_fds - l->inactive_fds;
        swap(l, id, first_inactive);
    }

    __sync_fetch_and_add(&(l->tracked_fds),1);

    for (int i = 0; i < l->tracked_fds; i++) {
        if (l->fds[i + INCOMING_MSG_PIPE].events & POLLIN) {
            assert(i < l->tracked_fds - l->inactive_fds);
        } else {
            assert(i >= l->tracked_fds - l->inactive_fds);
        }
    }

    /* Prime the pump by sinking 0 bytes and getting a size to expect. */
    bus_sink_cb_res_t sink_res = b->sink_cb(l->read_buf, 0, ci->udata);
    BUS_ASSERT(b, b->udata, sink_res.full_msg_buffer == NULL);  // should have nothing to handle yet
    ci->to_read_size = sink_res.next_read;

    if (!ListenerTask_GrowReadBuf(l, ci->to_read_size)) {
        free(ci);
        ListenerCmd_NotifyCaller(l, notify_fd);
        return;             /* alloc failure */
    }

    BUS_LOG(b, 3, LOG_LISTENER, "added socket", b->udata);
    ListenerCmd_NotifyCaller(l, notify_fd);
}

static void remove_socket(listener *l, int fd, int notify_fd) {
    struct bus *b = l->bus;
    BUS_LOG_SNPRINTF(b, 2, LOG_LISTENER, b->udata, 128,
        "removing socket %d", fd);

    /* Don't really close it, just drop info about it in the listener.
     * The client thread will actually free the structure, close SSL, etc. */
    for (int id = 0; id < l->tracked_fds; id++) {
        struct pollfd removing_pfd = l->fds[id + INCOMING_MSG_PIPE];
        if (removing_pfd.fd == fd) {
            bool is_active = (removing_pfd.events & POLLIN) > 0;
            if (l->tracked_fds > 1) {
                int last_active = l->tracked_fds - l->inactive_fds - 1;

                /* If removing active node and it isn't the last active one, swap them */
                if (is_active && id != last_active) {
                    assert(id < last_active);
                    swap(l, id, last_active);
                    id = last_active;
                }

                /* If node (which is either last active node or inactive) is not at the end,
                 * and there are inactive nodes, swap it with the last.*/
                int last = l->tracked_fds - 1;
                if (id < last) {
                    swap(l, id, last);
                    id = last;
                }

                /* The node is now at the end of the array. */
            }

            __sync_fetch_and_sub(&l->tracked_fds, 1);
            
            if (!is_active) {
                 __sync_fetch_and_sub(&l->inactive_fds, 1);
            }
        }
    }
    /* CI will be freed by the client thread. */
    ListenerCmd_NotifyCaller(l, notify_fd);
}

static void hold_response(listener *l, int fd, int64_t seq_id, int16_t timeout_sec, int notify_fd) {
    struct bus *b = l->bus;
    BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "hold_response <fd:%d, seq_id:%lld>", fd, (long long)seq_id);

    rx_info_t *info = ListenerHelper_GetFreeRXInfo(l);
    if (info == NULL) {
        BUS_LOG_SNPRINTF(b, 0, LOG_LISTENER, b->udata, 128, "failed to get free rx_info for <fd:%d, seq_id:%lld>, dropping it", fd, (long long)seq_id);
//      ListenerCmd_NotifyCaller(l, notify_fd); // The 'caller' actually calls the method directly now
        return;
    }
    
    BUS_ASSERT(b, b->udata, info);

    BUS_LOG_SNPRINTF(b, 5, LOG_LISTENER, b->udata, 128, "setting info %p(+%d) to hold response <fd:%d, seq_id:%lld>", (void *)info, info->id, fd, (long long)seq_id);
    info->u.expect.box = NULL;
    info->u.hold.error = RX_ERROR_NONE;
    info->timeout_sec = timeout_sec;
    info->u.hold.fd = fd;
    info->u.hold.seq_id = seq_id;
    info->u.hold.has_result = false;
    memset(&info->u.hold.result, 0, sizeof(info->u.hold.result));
    assert(ATOMIC_BOOL_COMPARE_AND_SWAP(&(info->state), RIS_HEADINUSE, RIS_HOLD));
//  ListenerCmd_NotifyCaller(l, notify_fd); // The 'caller' actually calls the method directly now
}

static void expect_response(listener *l, struct boxed_msg *box) {
    struct bus *b = l->bus;
    BUS_ASSERT(b, b->udata, box);
    BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 128,
        "notifying to expect response <box:%p, fd:%d, seq_id:%lld>",
        (void *)box, box->fd, (long long)box->out_seq_id);

    /* If there's a pending HOLD message, convert it. */
    rx_info_t *info = ListenerHelper_FindInfoBySequenceID(l, box->fd, box->out_seq_id, RIS_HOLD);
    if (info && info->state == RIS_HOLD) {
        BUS_ASSERT(b, b->udata, info->state == RIS_HOLD);

        info->u.expect.box = NULL;

        assert(info->u.hold.error != -1);

        if (info->u.hold.error == RX_ERROR_NONE && info->u.hold.has_result) {
            bus_unpack_cb_res_t result = info->u.hold.result;

            BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 256,
                "converting HOLD to EXPECT for info %d (%p) with result, attempting delivery <box:%p, fd:%d, seq_id:%lld>",
                info->id, (void *)info,
                (void *)box, info->u.hold.fd, (long long)info->u.hold.seq_id);

            info->u.expect.box = box;
            assert(info->u.expect.box);
            info->u.expect.has_result = true;
            info->u.expect.result = result;
            if(!info->u.expect.result.ok){
                fprintf(stderr,"!info->u.expect.result.ok");
                void* death = NULL;   
                char assertion[] = "stop";
                memcpy ( death, assertion, 5 );
            }
            rx_info_state currentstate = RIS_EXPECT; __atomic_store(&(info->state), &currentstate, __ATOMIC_RELAXED);
            int s = RX_ERROR_DELIVERING; __atomic_store(&(info->u.expect.error), &s, __ATOMIC_RELAXED);
            assert(info->u.expect.box);
            BUS_ASSERT(b, b->udata, box->result.status != BUS_SEND_UNDEFINED);

            ListenerTask_AttemptDelivery(l, info);
        } else if (info->u.hold.error != RX_ERROR_NONE) {
            BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 256, "info %p (%d) with <box:%p, fd:%d, seq_id:%lld> has error %d",
                (void *)info, info->id,
                (void *)box, info->u.hold.fd, (long long)info->u.hold.seq_id, info->u.hold.error);

            rx_error_t error = info->u.hold.error;
            bus_unpack_cb_res_t result = info->u.hold.result;
            __atomic_store(&(info->u.expect.error), &error, __ATOMIC_RELAXED);
            info->u.expect.result = result;
            assert(info->u.expect.result.ok);
            info->u.expect.box = box;
            assert(info->u.expect.box);
            info->state = RIS_EXPECT;
            ListenerTask_NotifyMessageFailure(l, info, BUS_SEND_RX_FAILURE);
        } else {
            BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 256, "converting HOLD to EXPECT info %d (%p), attempting delivery <box:%p, fd:%d, seq_id:%lld>",
                info->id, (void *)info,
                (void *)box, info->u.hold.fd, (long long)info->u.hold.seq_id);

            info->u.expect.box = box;
            assert(info->u.expect.box);
            int s = RX_ERROR_NONE; __atomic_store(&(info->u.expect.error), &s, __ATOMIC_RELAXED);
            info->u.expect.has_result = false;
            /* Switch over to client's transferred timeout */
            info->timeout_sec = box->timeout_sec;
            info->state = RIS_EXPECT; // TODO use atomic store?
        }
    } else if (info && info->state == RIS_EXPECT) {
        /* Multiple identical EXPECTs should never happen, outside of
         * memory corruption in the queue. */
        assert(false);
    } else if (info && info->state == RIS_HEADINUSE) {
        // ROB: RIS_HEADINUSE is a transisiton state -> info is not yet completely enqueued 
    }else {
        /* should never happen; just drop the message */
        //BUS_LOG_SNPRINTF(b, 3, LOG_LISTENER, b->udata, 256, "Couldn't find info for box %p", (void *)box);
        fprintf(stderr, "Couldn't find info for box %p\n", (void *)box);
    }
}

static void shutdown(listener *l, int notify_fd) {
    int val = notify_fd;
    __atomic_store(&(l->shutdown_notify_fd), &val, __ATOMIC_RELAXED);
}
