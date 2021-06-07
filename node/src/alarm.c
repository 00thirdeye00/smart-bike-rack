/* include headers  */
#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"

#include <stdio.h>

//timer to check alarm flag
static	struct	etimer led_timer;	

//static	uint8_t	counter;

PROCESS(leds_alarm, "LED ALARM SIM");
AUTOSTART_PROCESSES(&leds_alarm);

void al_raise(void){
	al_flag = true;
}

void al_raise(void){
	al_flag = false;
}

//void alarm_ledon(void){
//	leds_on(LEDS_ALL);	
//}
//
//void alarm_ledoff(void){
//	leds_off(LEDS_ALL);	
//}



PROCESS_THREAD(leds_alarm, "LED ALARM SIM"){

	PROCESS_BEGIN();

	etimer_set(led_timer, CLOCK_SECOND);

	while(al_flag){
		//PROCESS_YIELD();
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_timer));
		leds_on(LEDS_ALL);
		etimer_set(led_timer, CLOCK_SECOND/2);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&led_timer));
		leds_off(LEDS_ALL);
		etimer_set(led_timer, CLOCK_SECOND/2);
	}

	PROCESS_END();
}








void al_raise(void){
	/* simulate alarm here using LEDs */
}

void al_reset(void){

	/* simulate alarm reset using LEDs */
}
