/* include headers  */
#include <stdio.h>

#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"
#include "lock.h"



//timer to check alarm flag
static	struct	etimer led_lock_timer;	

//static	uint8_t	counter;

PROCESS(leds_lock, "LED LOCK SIM");
AUTOSTART_PROCESSES(&leds_lock);

void lk_lock(void){
	lk_flag = true;
}

void lk_unlock(void){
	lk_flag = false;
}



PROCESS_THREAD(leds_lock, "LED LOCK SIM"){

	PROCESS_BEGIN();

	etimer_set(led_lock_timer, CLOCK_SECOND);

	while(lk_flag){
		//PROCESS_YIELD();
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_lock_timer));
		leds_on(LEDS_ALL);
		etimer_set(led_lock_timer, CLOCK_SECOND/2);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_lock_timer));
		leds_off(LEDS_ALL);
		etimer_set(led_lock_timer, CLOCK_SECOND/2);
	}

	PROCESS_END();
}
