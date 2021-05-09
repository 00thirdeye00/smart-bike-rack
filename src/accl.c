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

// X/Y/Z ranges to be confirmed
#define SAFE			1	
#define SAFE_HIG_RNG	15
#define SAFE_LOW_RNG	30
#define USAFE_LOW_RNG	45
#define USAFE_HIG_RNG	60


static process_event_t xxx;
static process_event_t detection_event;

uint32_t	accel_data[3] = {0};	//[0]->x, [1]->y, [2]->z 
uint8_t		err_acl_cnt = 0;

enum ACCEL_STATES {
    SAFE_HIGH,
    SAFE_LOW,
    USAFE_LOW,
    USAFE_HIGH,
};

/* Declare our "main" process, the client process*/
PROCESS(client_process, "detection client");
PROCESS(accel_process, "Accel process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process, &accel_process);

 
ACCEL_SAFE accel_state = SAFE_HIGH;

/*
void accel_read(accel_enum xyz_state){
	    
	static int16_t accel_x_old;
	static int16_t accel_y_old;
	static int16_t accel_z_old;
	static int16_t accel_x_new;
	static int16_t accel_y_new;
	static int16_t accel_z_new;
//	static int16_t prev_accel;

//	accel_state = SAFE_HIGH;

	accel_x_new = accm_read_axis(X_AXIS);
    accel_y_new = accm_read_axis(Y_AXIS);
    accel_z_new = accm_read_axis(Z_AXIS);	
	
        // READ X_AXIS TODO add y,z axis

	switch(xyz_state) {
        case SAFE_HIGH:
			if(	abs(accel_x_new > SAFE_HIG_RNG) || 
				abs(accel_y_new > accel_y_old) ||
				abs(accel_z_new > accel_z_old) ){ 	
				
				if( err_acl_cnt == 0) {
					accel_state = safe_low;
					// wait for timer interrupt 100 ms
					etimer_set(&et, CLOCK_SECOND/100);
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					//accel_state(accel_state);
				}
				else {
					err_acl_cnt = err_acl_cnt - 1;
					// wait for timer interrupt 10 ms
					etimer_set(&et, CLOCK_SECOND/10);
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
				}
			}
            break;
        case SAFE_HIGH: 
            break;
        case USAFE_LOW: 
            break;
        case USAFE_HIGH: 
            break;
		default:
			accel_state = SAFE;
			break;
    }
	accel_x_old = accel_x_new;
	accel_y_old = accel_y_new;
	accel_z_old = accel_z_new;
}
*/
static struct etimer et;
PROCESS_THREAD(accel_process, ev, data) {
    PROCESS_BEGIN();
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
                //accel_state(accel_state);
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
