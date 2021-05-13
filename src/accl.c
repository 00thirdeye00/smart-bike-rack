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
#define RNG_SAFE_HIGH	20
#define RNG_SAFE_LOW	40
#define RNG_USAFE_LOW	60
#define RNG_USAFE_HIGH	80

#define X_DAT	0
#define Y_DAT	1
#define Z_DAT	2

static process_event_t xxx;
static process_event_t detection_event;

uint8_t err_acl_cnt;
uint32_t	accel_data[3][50] = {0};	//[0]->x, [1]->y, [2]->z 

enum ACCEL_STATES {
	SAFE_HIGH,
	SAFE_LOW,
	USAFE_LOW,
	USAFE_HIGH,
};

ACCEL_STATES prev_accl_state, curr_accl_state;

/* Declare our "main" process, the client process*/
PROCESS(client_process, "detection client");
PROCESS(accel_process, "Accel process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process, &accel_process);


accel_statee = SAFE_HIGH;

/* function to return current state  */
ACCEL_STATES current_state(void){
	return curr_accl_state;
}

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



/*---------------------------------------------------------------------------*/
/* accelerometer free fall detection callback */

void
accm_ff_cb(uint8_t reg){
	leds_on(LEDS_BLUE);
	process_post(&led_process, ledOff_event, NULL);
	printf("~~[%u] Freefall detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
	print_int(reg);
}

/*---------------------------------------------------------------------------*/
/* accelerometer tap and double tap detection callback */

void
accm_tap_cb(uint8_t reg){
	process_post(&led_process, ledOff_event, NULL);
	if(reg & ADXL345_INT_DOUBLETAP){
		leds_on(LEDS_GREEN);
		printf("~~[%u] DoubleTap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
	} else {
		leds_on(LEDS_RED);
		printf("~~[%u] Tap detected! (0x%02X) -- ", ((uint16_t) clock_time())/128, reg);
	}
	print_int(reg);
}

/*---------------------------------------------------------------------------*/
/* accelerometer process  */

static struct etimer et;
PROCESS_THREAD(accel_process, ev, data) {
	PROCESS_BEGIN();
	{		
		/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
		accm_init();

		/* Register the callback functions for each interrupt */
		ACCM_REGISTER_INT1_CB(accm_ff_cb);
		ACCM_REGISTER_INT2_CB(accm_tap_cb);
		
		printf("before while accel process\n");

		while (1) {
			// READ X_AXIS TODO add y,z axis
			accel_read = accm_read_axis(X_AXIS);
			printf("x: %d\n", accel_read);
			printf("prev_x: %d\n", prev_accel);
			printf("state: %d\n", prev_accl_state);

			if(err_acl_cnt == 0){
				accel_read();
				etimer_set(&et, CLOCK_SECOND/100);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
			else if(err_acl_cnt > 0){
				accel_read();
				etimer_set(&et, CLOCK_SECOND/10);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			}
			printf("in while accel process\n");
		}
	}	
	PROCESS_END();
}

///* Our main process. */
//PROCESS_THREAD(client_process, ev, data) {
//
//	PROCESS_BEGIN();
//
//	accm_init();
//
//	/* Loop forever. */
//	while (1) {
//		/* Wait until an event occurs. If the event has
//		 * occured, ev will hold the type of event, and
//		 * data will have additional information for the
//		 * event. In the case of a sensors_event, data will
//		 * point to the sensor that caused the event.
//		 * Here we wait until the button was pressed. */
//
//		PROCESS_WAIT_EVENT_UNTIL( ev == detection_event ||
//				(ev == sensors_event && data == &button_sensor));
//		printf("rx: detection event!! \n");
//	}
//
//	PROCESS_END();
//}
