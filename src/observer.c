/*
 * \author	Guru Mehar Rachaputi
 *			Karl Wallentin
 *
 * /

/* observer file
 *
 *	1.	read accelerometer sensor current state
 *	2.	read distance sensor current state
 *	3.	raise alarm
 *	4.	send data to root
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"
#include "accl.h"

#define OBS_INTERVAL CLOCK_SECOND/100
#define ACCEL_RANGE 71


static process_event_t xxx;
static process_event_t detection_event;

uint32_t	accel_data[3][50] = {0};	//[0]->x, [1]->y, [2]->z 

enum ACCEL_STATES {
	SAFE_HIGH,
	SAFE_LOW,
	USAFE_LOW,
	USAFE_HIGH,
};


extern ACCEL_STATES current_state(void);

/* Declare our "main"/"observer" process, the client process*/
PROCESS(obs_process, "Observer process");
/* The observer process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&obs_process);


/* function to read accelerometer axis and change state  */
void accel_read(void){

	static uint16_t i;
	static us_count;

	//	static int16_t accel_x_old;
	//	static int16_t accel_y_old;
	//	static int16_t accel_z_old;
	static int16_t accel_x_new;
	//static int16_t accel_y_new;
	//static int16_t accel_z_new;
	//	static int16_t prev_accel;

	//	prev_accl_state = SAFE_HIGH;

	accel_x_new = accm_read_axis(X_AXIS);
	//accel_y_new = accm_read_axis(Y_AXIS);
	//accel_z_new = accm_read_axis(Z_AXIS);	

	// READ X_AXIS TODO add y,z axis

	if((accel_x_new > 0) && (accel_x_new <= RNG_SAFE_HIGH)){
		if(err_acl_cnt > 0){
			err_acl_cnt -= 1;
		}
		curr_accl_state = SAFE_HIGH;
	}
	else if((accel_x_new > RNG_SAFE_HIGH) && (accel_x_new <= RNG_SAFE_LOW)){
		err_acl_cnt += 1;
		curr_accl_state = SAFE_LOW;
	}
	else if((accel_x_new > RNG_SAFE_LOW) && (accel_x_new <= RNG_USAFE_LOW)){
		err_acl_cnt += 1;
		curr_accl_state = USAFE_LOW;
	}
	else if((accel_x_new > RNG_USAFE_LOW) && (accel_x_new <= RNG_USAFE_HIGH)){
		err_acl_cnt += 1;
		curr_accl_state = USAFE_HIGH;
	}


	if(curr_accl_state == USAFE_HIGH){ 
		if(prev_accl_state == USAFE_HIGH){
			us_count += 1;
			if(us_count > 5){
				//trigger alarm
			}
		}
		else{
			us_count = 0;
		}
	}

	accel_data[X_DAT][i++] = accel_x_new;
	//accel_y_old = accel_y_new;
	//accel_z_old = accel_z_new;

	prev_accl_state = curr_accl_state;
}

/* Observer process  */
static struct etimer et;

/* Our main process. */
PROCESS_THREAD(obs_process, ev, data) {

	PROCESS_BEGIN();

	/* log data function TBD  */
	//log_data();

	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */

		if(accel_state == USAFE_HIGH){
			do{
				alarm_raise();
			}while(!alarm_reset);

			etimer_set(&et, CLOCK_SECOND/100);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		}
		else if(){
			
			
		}

		PROCESS_WAIT_EVENT_UNTIL( ev == detection_event ||
				(ev == sensors_event && data == &button_sensor));
		printf("rx: detection event!! \n");
	}

	PROCESS_END();
}
