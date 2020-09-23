/*
 *  Author: Vilmain Nicolas
 *  Contact: nicolas.vilmain@gmail.com
 *
 *  This file is part of libchannel.
 *
 *  libchannel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  monocle is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with monocle.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "lib.h"
#include "channel.h"

#define MICROSEC 1000000

#define channel_xfree(y)    \
    do {                    \
        if (y)              \
            free(y);        \
    } while (0)

enum channel_options {
    CHAN_OPT_LISTENER = (1 << 0),
};

static void *channel_listener_func(void *arg);
static void *channel_read_message(struct channel *chan);
static void channel_stop_listener(struct channel *chan);
static void channel_free(struct channel *chan);
static void channel_message_free_all(struct channel *chan);
static void *xcalloc(unsigned int size);

struct channel *
channel_new(void (*message_free)(void *))
{
    struct channel *chan;

    chan = xcalloc(sizeof(struct channel));
    chan->message_free_data = message_free;
    mutex_init(&chan->mutex);
    return chan;
}

struct channel *
channel_with_listener(void (*message_free)(void *),
                      void *listener_data,
                      int (*listener_func)(void *, void *),
                      unsigned int n_listener)
{
    unsigned int i;
    struct channel *chan;

    chan = channel_new(message_free);
    if (!n_listener)
        return chan;

    chan->options |= CHAN_OPT_LISTENER;
    chan->listener = xcalloc((n_listener + 1) * sizeof(thread_type));
    chan->argument = listener_data;
    chan->listener_func = listener_func;
    for (i = 0; i < n_listener; i++) {
        if (thread_create(&chan->listener[i],
                          channel_listener_func,
                          chan) < 0) {
            channel_close(chan);
            return NULL;
        }
        usleep(10000);
    }
    return chan;
}

static void *
channel_listener_func(void *arg)
{
    int ret;
    void *data;
    struct channel *chan;

    chan = arg;
    do {
        data = channel_read(chan);
        if (data) {
            ret = chan->listener_func(chan->argument, data);
            if (chan->message_free_data)
                chan->message_free_data(data);
        }
    } while (ret != CHANNEL_LISTENER_CLOSE);
    thread_exit();
}

void
channel_write(struct channel *chan, void *data)
{
    struct channel_message *message;
    struct channel_message *msgptr;

    if (!chan)
        return;

    message = xcalloc(sizeof(struct channel_message));
    message->data = data;

    mutex_lock(&chan->mutex);
    if (!chan->message)
        chan->message = message;
    else {
        msgptr = chan->message;
        while (msgptr->next)
            msgptr = msgptr->next;
        msgptr->next = message;
    }
    mutex_unlock(&chan->mutex);
}

void *
channel_read(struct channel *chan)
{
    void *data;

    for (;;) {
        if ((data = channel_read_message(chan)))
            break;
        usleep(50000);
    }
    return data;
}

void *
channel_read_nonblock(struct channel *chan, int sec_timeout)
{
    int sec;
    void *data;
    unsigned long msec;
    struct timeval tvstart;
    struct timeval tvstop;

    if (sec_timeout < 1)
        data = channel_read(chan);
    else {
        msec = 0;
        do {
            gettimeofday(&tvstart, NULL);
            if ((data = channel_read_message(chan)))
                break;
            usleep(50000);
            gettimeofday(&tvstop, NULL);
            msec += ((MICROSEC * tvstop.tv_sec) + tvstop.tv_usec) -
                 ((MICROSEC * tvstart.tv_sec) + tvstart.tv_usec);
            sec = msec / MICROSEC;
        } while (sec < sec_timeout);
    }
    return data;
}

static void *
channel_read_message(struct channel *chan)
{
    void *data = NULL;
    struct channel_message *message;

    mutex_lock(&chan->mutex);
    if (chan->message) {
        message = chan->message;
        chan->message = message->next;
        data = message->data;
        channel_xfree(message);
    }
    mutex_unlock(&chan->mutex);
    return data;
}

void
channel_close(struct channel *chan)
{
    if (chan) {
        if ((chan->options & CHAN_OPT_LISTENER))
            channel_stop_listener(chan);
        channel_free(chan);
    }
}

static void
channel_stop_listener(struct channel *chan)
{
    unsigned int i;

    for (i = 0; chan->listener[i]; i++) {
        thread_cancel(chan->listener[i]);
        thread_wait(chan->listener[i]);
    }
    channel_xfree(chan->listener);
}

static void
channel_free(struct channel *chan)
{
    mutex_lock(&chan->mutex);
    channel_message_free_all(chan);
    chan->message = NULL;
    mutex_unlock(&chan->mutex);
    mutex_destroy(&chan->mutex);
    channel_xfree(chan);
}

static void
channel_message_free_all(struct channel *chan)
{
    struct channel_message *ptrnext;
    struct channel_message *message;

    message = chan->message;
    while (message) {
        ptrnext = message->next;
        if (chan->message_free_data)
            chan->message_free_data(message->data);
        channel_xfree(message);
        message = ptrnext;
    }
}

static void *
xcalloc(unsigned int size)
{
    void *p;

    p = malloc(size);
    if (!p) {
        fprintf(stderr, "memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    memset(p, 0, size);
    return p;
}
