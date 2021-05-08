#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"


#define ACCM_READ_INTERVAL CLOCK_SECOND/100

static process_event_t detection_event;
/* Declare our "main" process, the client process*/
PROCESS(client_process, "detection client");
PROCESS(accel_process, "Accel process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process, &accel_process);

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}

static struct etimer et;
PROCESS_THREAD(accel_process, ev, data) {
    PROCESS_BEGIN();
    static int16_t x;
    static int16_t prev_x = 0;
    while (1) {
        x = accm_read_axis(X_AXIS);
        printf("x: %d\n", x);
        printf("prev_x: %d\n", prev_x);
        if(abs(prev_x - x) > 70) process_post(&client_process, detection_event, NULL);
        etimer_set(&et, ACCM_READ_INTERVAL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        leds_off(LEDS_RED);
        prev_x = x;
        }
      PROCESS_END();
}
/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {
            char payload[] = "led1";

            PROCESS_BEGIN();

            /* Activate the button sensor. */
            SENSORS_ACTIVATE(button_sensor);
            
            accm_init();

            /* Initialize NullNet */
            nullnet_buf = (uint8_t *)&payload;
            nullnet_len = sizeof(payload);
            nullnet_set_input_callback(recv);

            /* Loop forever. */
            while (1) {
                    /* Wait until an event occurs. If the event has
                     * occured, ev will hold the type of event, and
                     * data will have additional information for the
                     * event. In the case of a sensors_event, data will
                     * point to the sensor that caused the event.
                     * Here we wait until the button was pressed. */
                    
		PROCESS_WAIT_EVENT_UNTIL( ev == detection_event ||
                       (ev == sensors_event && data == &button_sensor));

                if(data == &button_sensor){
                    leds_toggle(LEDS_GREEN); 
                    nullnet_buf =(uint8_t *)"led1";
                }  
                else {
                    leds_toggle(LEDS_RED); 
                    nullnet_buf =(uint8_t *)"led2";
                }

		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload, sizeof(payload));
                nullnet_len = sizeof(payload);

		/* Send the content of the packet buffer using the
		 * broadcast handle. */
		NETSTACK_NETWORK.output(NULL);
	}

	PROCESS_END();
}
