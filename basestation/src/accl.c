/* accl.c  */


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
//#include "alarm.h"


#define ACCM_READ_INTERVAL CLOCK_SECOND/100
#define ACCEL_RANGE 70

// X/Y/Z ranges to be confirmed
#define SAFE			1	
#define RNG_SAFE_HIGH	10
#define RNG_SAFE_LOW	20
#define RNG_UNSAFE_LOW	30
#define RNG_UNSAFE_HIGH	40

#define X_DAT	0
#define Y_DAT	1
#define Z_DAT	2

//static process_event_t xxx;

uint8_t	err_acl_cnt;
uint32_t accel_data[3][50] = {{0}};	//[0]->x, [1]->y, [2]->z 


static ACCEL_STATES curr_accl_state; //prev_accl_state

/* Declare our "main" process, the client process*/
PROCESS(accel_process, "Accel process");
/* The client process should be started automatically when
 * the node has booted. */

//AUTOSTART_PROCESSES(&accel_process);


ACCEL_STATES accel_state = SAFE_HIGH;

/* function to return current state  */
ACCEL_STATES current_state(void){
	return curr_accl_state;
}

/* function to read accelerometer axis and change state  */
void accel_read(void){

	static uint16_t i;

	//	static int16_t accel_x_old;
	//	static int16_t accel_y_old;
	//	static int16_t accel_z_old;
	static int16_t accel_x_new;
	//static int16_t accel_y_new;
	//static int16_t accel_z_new;
	//	static int16_t prev_accel;

	//	prev_accl_state = SAFE_HIGH;

	accel_x_new = abs(accm_read_axis(X_AXIS));
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
	else if((accel_x_new > RNG_SAFE_LOW) && (accel_x_new <= RNG_UNSAFE_LOW)){
		err_acl_cnt += 1;
		curr_accl_state = UNSAFE_LOW;
	}
	else if((accel_x_new > RNG_UNSAFE_LOW) && (accel_x_new <= RNG_UNSAFE_HIGH)){
		err_acl_cnt += 1;
		curr_accl_state = UNSAFE_HIGH;
	}

	if(i == 60){
		i %= 60;
		//adding x axis data to 2d data buffer
		accel_data[X_DAT][i++] = accel_x_new;


		//accel_y_old = accel_y_new;
		//accel_z_old = accel_z_new;
	}
	//prev_accl_state = curr_accl_state;

}

uint16_t accel_data_avg(void){
	uint16_t accel_avg = 0;
	int i;
	for(i = 0; i < sizeof(accel_data[X_DAT]); i++){
		accel_avg += accel_data[X_DAT][i]; 
	}
	return accel_avg/60;
}

/* accelerometer process  */

static struct etimer et;
PROCESS_THREAD(accel_process, ev, data) {
	PROCESS_BEGIN();
	{		

		/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
		//accm_init();


		while (1) {
			// READ X_AXIS TODO add y,z axis
			accel_read();

			//	printf("x: %d\n", accel_read);
			//	printf("prev_x: %d\n", prev_accel);
			//	printf("state: %d\n", prev_accl_state);

			if(err_acl_cnt == 0){
				//accel_read();
				etimer_set(&et, CLOCK_SECOND/10);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
			else if(err_acl_cnt > 0){
				//accel_read();
				etimer_set(&et, CLOCK_SECOND/100);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
		}
	}	
	PROCESS_END();
}

