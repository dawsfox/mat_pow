/**
 * @file e_darts_mutex.h
 * @author Jose M Monsalve
 * @date 2 June 2017
 * @brief own implementation of mutex based on the e_SDK mutex implementation
 * @todo Add copyright
 *
 * Mutex are required in several parts of the runtime.
 * It is required to always have the full address of
 * the mutex in order to lock it or unlock it from anywhere
 * in the chip
 *
 * This file borrows heavily from the original eSDK
 *
 */
#ifndef E_DARTS_MUTEX_H
#define E_DARTS_MUTEX_H

typedef volatile unsigned darts_mutex_t;
#define DARTS_MUTEX_NULL (0)

/**
 * @brief Locking a mutex given its location. The location
 * must include core ID.
 *
 * @todo exponential backoff. Find the best parameters
 *
 * @param mutex. It is the darts_mutex_t to be locked.
 * Address must be complete. this is including code row and col ID
 *
 * @return the coreID of the owner of the lock. -1 if the mutex was sent
 * without core ID.
 *
 */

int darts_mutex_lock( volatile darts_mutex_t *mutex );

/**
 * @brief unlocking a mutex. The pointer must have all the
 * core ID.
 *
 * @param mutex. It is the darts_mutex_t to be unlocked.
 * Address must be complete. this is including code row and col ID
 *
 * @return 0 if it was unlocked successfully. -1 if the mutex was sent
 * without core ID.
 *
 */

int darts_mutex_unlock( volatile  darts_mutex_t *mutex );

#endif /* E_DARTS_MUTEX_H */
