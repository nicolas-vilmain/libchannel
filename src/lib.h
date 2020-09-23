/*
 *  Author: Vilmain Nicolas
 *  Contact: nicolas.vilmain@gmail.com
 *
 *  This file is part of channel_c.
 *
 *  channel_c is free software: you can redistribute it and/or modify
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

#ifndef __LIB_CHANNEL_H__
#define __LIB_CHANNEL_H__

#include <pthread.h>

/* adapt this with your favorite thread library */
typedef pthread_t thread_type;
typedef pthread_mutex_t thread_mutex_t;
#define thread_create(x,y,z) pthread_create(x, NULL, y, z)
#define thread_exit() pthread_exit(NULL)
#define thread_cancel(y) pthread_cancel(y)
#define thread_wait(y) pthread_join(y, NULL)
#define mutex_init(y) pthread_mutex_init(y, NULL)
#define mutex_destroy(y) pthread_mutex_destroy(y)
#define mutex_lock(y) pthread_mutex_lock(y)
#define mutex_unlock(y) pthread_mutex_unlock(y)

#endif /* not have __LIB_CHANNEL_H__ */
