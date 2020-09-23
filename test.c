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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "channel.h"

struct example {
    int i;
    char *str;
    struct channel *reply;
};

void test_channel_with_auto_listener(void);
void test_channel_with_my_listener(void);
void test_loop(struct channel *chan_reply, struct channel *chan_listen);
void * my_listener(void *arg);
int listener(void *arg, void *data);
void listener_free_message(void *arg);

int
main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    test_channel_with_auto_listener();
    test_channel_with_my_listener();
    return 0;
}

void
test_channel_with_auto_listener(void)
{
    struct channel *chan_reply = NULL;
    struct channel *chan_listen = NULL;

    chan_reply = channel_new(NULL);
    chan_listen = channel_with_listener(&listener_free_message, chan_reply, &listener, 3);
    test_loop(chan_reply, chan_listen);
    channel_close(chan_reply);
    channel_close(chan_listen);
}

int
listener(void *arg, void *data)
{
    struct example *ex = data;
    struct channel *reply = arg;

    printf("auto listener recevied: %s - %d\n", ex->str, ex->i);
    channel_write(reply, "ok");
    return CHANNEL_LISTENER_CONT;
}

void
test_channel_with_my_listener(void)
{
    int i;
    pthread_t th[3];
    struct channel *chan_reply;
    struct channel *chan_listen;

    chan_reply = channel_new(NULL);
    chan_listen = channel_new(&listener_free_message);
    for (i = 0; i < 3; i++) {
        if (pthread_create(&th[i], NULL, &my_listener, chan_listen) < 0) {
            fprintf(stderr, "failed to start new thread: %s\n", strerror(errno));
            exit(-1);
        }
    }
    // start 3 thread listener
    test_loop(chan_reply, chan_listen);
    for (i = 0; i < 3; i++) {
        pthread_cancel(th[i]);
        pthread_join(th[i], NULL);
    }
    /* not close before stop threads litener ! or init channel var at NULL */
    channel_close(chan_reply);
    channel_close(chan_listen);
}

void
test_loop(struct channel *chan_reply, struct channel *chan_listen)
{
    int i;
    const char *reply;
    struct example *ex;

    for (i = 0; i < 10; i++) {
        ex = malloc(sizeof(struct example));
        ex->reply = chan_reply;
        ex->str = strdup("test");
        ex->i = i;
        channel_write(chan_listen, ex);
        reply = (const char *) channel_read_nonblock(chan_reply, 1);
        if (!reply) {
            puts("Nothing to read !");
            exit(-1);
        }
        printf("reply: %s\n", reply);
    }
}

void *
my_listener(void *arg)
{
    struct example *ex;
    struct channel *chan = arg;

    for (;;) {
        ex = channel_read(chan);
        if (ex) {
            printf("my_listener recevied: %s - %d\n", ex->str, ex->i);
            channel_write(ex->reply, "ok");
            listener_free_message(ex);
        }
    }
}

void
listener_free_message(void *arg)
{
    struct example *ex = arg;
    free(ex->str);
    free(ex);
}
