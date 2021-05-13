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


static process_event_t detection_event;

extern uint32_t	accel_data[3][50];	//[0]->x, [1]->y, [2]->z 


//extern ACCEL_STATES current_state(void);

/* Declare our "main"/"observer" process, the client process*/
PROCESS(obs_process, "Observer process");
/* The observer process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&obs_process);


/* Observer process  */

PROCESS_THREAD(obs_process, ev, data) {

	static struct etimer periodic_timer;
	int is_coordinator;
	uip_ipaddr_t dest_ipaddr;

	PROCESS_BEGIN();

	is_coordinator = 0;

#if CONTIKI_TARGET_COOJA || CONTIKI_TARGET_Z1
	is_coordinator = (node_id == 1);
#endif

	if(is_coordinator){
		NETSTACK_ROUTING.root_start();	
	}
	
	simple_udp_register(&server_conn, UDP_SERVER_PORT, NULL,
			UDP_CLIENT_PORT, udp_rx_callback);
	
	simple_udp_register(&client_conn, UDP_CLIENT_PORT, NULL,
		UDP_SERVER_PORT, NULL);

	NETSTACK_MAC.on();

	etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

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

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		
		if(tsch_is_coordinator){
			break;
		}
		

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
