/*
 *alarm..h
 *
 *1. raise alarm
 *2. reset alarm
 *
 * */

#ifndef ALARM_H
#define ALARM_H

//shared global variables

bool al_flag;

/* raise alarm function  */
void al_raise(void);

/* reset alarm function  */
void al_reset(void);


#endif //ALARM_H
