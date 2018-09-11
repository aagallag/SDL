/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
//#include "../../SDL_internal.h"

/* An implementation of condition variables using semaphores and mutexes */
/*
   This implementation borrows heavily from the BeOS condition variable
   implementation, written by Christopher Tate and Owen Smith.  Thanks!
 */
#include "SDL_config.h"

#include "SDL_thread.h"

#include <switch.h>

struct SDL_cond
{
    CondVar cond;
};

struct SDL_mutex 
{
    RMutex mutex;
};

/* Create a condition variable */
SDL_cond *
SDL_CreateCond(void)
{
    SDL_cond *cond;

    cond = (SDL_cond *) SDL_malloc(sizeof(*cond));
    if (cond) {
        condvarInit(&cond->cond);
    }
    else {
        SDL_OutOfMemory();
    }
    return (cond);
}

/* Destroy a condition variable */
void
SDL_DestroyCond(SDL_cond * cond)
{
    if (cond) {
        SDL_free(cond);
    }
}

/* Restart one of the threads that are waiting on the condition variable */
int
SDL_CondSignal(SDL_cond * cond)
{
    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }
    condvarWakeOne(&cond->cond);
    return 0;
}

/* Restart all threads that are waiting on the condition variable */
int
SDL_CondBroadcast(SDL_cond * cond)
{
    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    condvarWakeAll(&cond->cond);
    return 0;
}

/* Wait on the condition variable for at most 'ms' milliseconds.
   The mutex must be locked before entering this function!
   The mutex is unlocked during the wait, and locked again after the wait.

Typical use:

Thread A:
    SDL_LockMutex(lock);
    while ( ! condition ) {
        SDL_CondWait(cond, lock);
    }
    SDL_UnlockMutex(lock);

Thread B:
    SDL_LockMutex(lock);
    ...
    condition = true;
    ...
    SDL_CondSignal(cond);
    SDL_UnlockMutex(lock);
 */
int
SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    int retval;
    uint32_t mutex_state[2];

    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    // backup mutext state
    mutex_state[0] = mutex->mutex.thread_tag;
    mutex_state[1] = mutex->mutex.counter;
    mutex->mutex.thread_tag = 0;
    mutex->mutex.counter = 0;

    retval = condvarWaitTimeout(&cond->cond, &mutex->mutex.lock, ms * 1000000);

    mutex->mutex.thread_tag = mutex_state[0];
    mutex->mutex.counter = mutex_state[1];

    return retval;
}

/* Wait on the condition variable forever */
int
SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
    int retval;
    uint32_t mutex_state[2];

    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    // backup mutext state
    mutex_state[0] = mutex->mutex.thread_tag;
    mutex_state[1] = mutex->mutex.counter;
    mutex->mutex.thread_tag = 0;
    mutex->mutex.counter = 0;

    retval =  condvarWait(&cond->cond, &mutex->mutex.lock);

    mutex->mutex.thread_tag = mutex_state[0];
    mutex->mutex.counter = mutex_state[1];

    return retval;
}
