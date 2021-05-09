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
#define ACCEL_RANGE 70

#define X_AXIS_LOW	50
#define Y_AXIS_LOW	50
#define Z_AXIS_LOW	50
#define X_AXIS_MID	75
#define Y_AXIS_MID	75
#define Z_AXIS_MID	75
#define X_AXIS_HIGH	100
#define Y_AXIS_HIGH	100
#define Z_AXIS_HIGH	100


static process_event_t detection_event;
/* Declare our "main" process, the client process*/
PROCESS(client_process, "detection client");
PROCESS(accel_process, "Accel process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process, &accel_process);


typedef enum accel_enum {
    safe_low,
    safe_med,
    unsafe_low,
    unsafe_high,
}accel_enum;
accel_enum accel_state;
uint8_t err_acl_cnt = 0;

/*
void state_function(accel_enum xyz_state) {
    switch(xyz_state) {
        case safe_low: 
            break;
        case safe_med: 
            break;
        case unsafe_low: 
            break;
        case unsafe_high: 
            break;
    }
}
*/
static struct etimer et;
PROCESS_THREAD(accel_process, ev, data) {
    PROCESS_BEGIN();
    static int16_t accel_read;
    static int64_t accel_data;
    static int16_t prev_accel = 0;
    while (1) {
        // READ X_AXIS TODO add y,z axis
        accel_read = accm_read_axis(X_AXIS);
        printf("x: %d\n", accel_read);
        printf("prev_x: %d\n", prev_accel);
        printf("state: %d\n", accel_state);

        // TODO extend ACCEL_RANGE to include severel ranges
        // TODO implement check of range as interrupt
        if(abs(prev_accel - accel_read) > ACCEL_RANGE) {
            if( err_acl_cnt == 0) {
                accel_state = safe_low;
                // wait for timer interrupt 100 ms
                etimer_set(&et, CLOCK_SECOND/100);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
                //state_function(accel_state);
            }
            else {
                err_acl_cnt = err_acl_cnt - 1;
                // wait for timer interrupt 10 ms
                etimer_set(&et, CLOCK_SECOND/10);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
            }
        }
        else {
            printf("OUTSIDE RANGE! \n");
            if(err_acl_cnt >= 50 ){
                process_post(&client_process, detection_event, NULL);
                accel_state = unsafe_high;
            }
            else {
                 etimer_set(&et, CLOCK_SECOND/10);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
                err_acl_cnt = err_acl_cnt + 1;
                accel_data = accel_data + accel_read; 
            }
        }
        prev_accel = accel_read;
        }
      PROCESS_END();
}
/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {

            PROCESS_BEGIN();

            accm_init();

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
                printf("rx: detection event!! \n");
	}

	PROCESS_END();
}
