/*
 * lock.h
 *
 *1. lock function
 *2. unlock function
 *
 * */

#ifndef LOCK_H
#define LOCK_H

bool lk_flag;

/* lock function  */
void lk_lock(void);

/* unlock function */
void lk_unlock(void);



#endif //LOCK_H
