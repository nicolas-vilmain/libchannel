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

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "lib.h"

#define LIBCHANNEL_VERSION_MAJOR 1
#define LIBCHANNEL_VERSION_MINOR 0

enum channel_status {
    CHANNEL_LISTENER_CONT   = 0,
    CHANNEL_LISTENER_CLOSE  = 1337,
};

struct channel {
    unsigned int options;
    void *argument;
    int (*listener_func)(void *, void *);
    void (*message_free_data)(void *);
    thread_type *listener;
    thread_mutex_t mutex;
    struct channel_message *message;
};

struct channel_message {
    void *data;
    struct channel_message *next;
};

struct channel * channel_new(void (*message_free)(void *));
struct channel * channel_with_listener(void (*message_free)(void *),
                                       void *listener_data,
                                       int (*listener_func)(void *, void *),
                                       unsigned int n_listener);
void channel_write(struct channel *chan, void *data);
void * channel_read(struct channel *chan);
void * channel_read_nonblock(struct channel *chan, int timeout);
void channel_close(struct channel *chan);

#endif /* not have __CHANNEL_H__ */
