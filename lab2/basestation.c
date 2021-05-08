#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define LED_INT_ONTIME  CLOCK_SECOND*10

/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
PROCESS(led_process, "ledprocess");
/* The basestation process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process,&led_process);
bool led1 = 0;
bool led2 = 0;



static struct etimer ledETimer;
PROCESS_THREAD(led_process, ev, data) {
      PROCESS_BEGIN();
      while(1) {
            PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
            etimer_set(&ledETimer, LED_INT_ONTIME);
            PROCESS_YIELD_UNTIL(etimer_expired(&ledETimer));
            leds_off(LEDS_ALL);
            led1 = 0;
            led2 = 0;
        }
      PROCESS_END();
 }


/* Holds the number of packets received. */
//static int count = 0;
/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */

static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
    char rec_data[sizeof(uint8_t)];

    //char old_data[sizeof(uint8_t)];
    //    count = count + 1 % 3 ;
    /* 0bxxxxx allows us to write binary values */
    /* for example, 0b10 is 2 */
    
    strcpy(rec_data, data);
    leds_off(LEDS_ALL);

    if(strcmp(rec_data,"led2") ) {
        //count = count + 1 % 3 ;
        led1 = 1;
        leds_on(0b10);
    }
    if(strcmp(rec_data,"led1") ) {
        //count = count + 1 % 3 ;
        led2 = 1;
        leds_on(0b01);
    }

    if((led1 && led2)) {
        leds_on(LEDS_ALL);
    }
    
    process_poll(&led_process);
}


/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();

	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}
