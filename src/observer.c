/*
 * \author	Guru Mehar Rachaputi
 *			Karl Wallentin
 *

 * observer file
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
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"
#include "dev/button-sensor.h"
#include "sys/log.h"
#include "sys/node-id.h"
#include "lock.h"
#include "alarm.h"

#include "/home/wcnes/smart-bike-rack/src/accl.h"
#include "/home/wcnes/smart-bike-rack/src/dist.h"

#define OBS_INTERVAL CLOCK_SECOND/100
//#define ACCEL_RANGE 71

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (5 * CLOCK_SECOND)

static struct simple_udp_connection client_conn, server_conn;
//static process_event_t detection_event;

extern uint32_t	accel_data[3][50];	//[0]->x, [1]->y, [2]->z 

struct Data_pkt 
{
    clock_time_t timestamp;
    ACCEL_STATES state_accl;
    DIST_STATES  state_dist;
};


//extern ACCEL_STATES current_state(void);

/* Declare our "main"/"observer" process, the client process*/
PROCESS(obs_process, "Observer process");
/* The observer process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&dist_process, &accel_process, &obs_process);

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  uint64_t local_time_clock_ticks = tsch_get_network_uptime_ticks();
  //uint64_t remote_time_clock_ticks;
  struct Data_pkt data_rx;
  
  memcpy(&data_rx, data, sizeof(struct Data_pkt));
  LOG_INFO("Received from ");
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO(" \n");
  printf("data received: %d at time: %lu \n", (int)data_rx.state_accl, (unsigned long)(local_time_clock_ticks - data_rx.timestamp));
  printf("dist_state rx: %d \n", (int)data_rx.state_dist);
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
static uint16_t us_count = 0;    
void check_accl_state(void) {
    ACCEL_STATES UNSAFE_HIGH_STATE = UNSAFE_HIGH;
    ACCEL_STATES curr_accl_state;
    ACCEL_STATES prev_accl_state;
    curr_accl_state = current_state();
    if(curr_accl_state == UNSAFE_HIGH_STATE){ 
            if(prev_accl_state == UNSAFE_HIGH){
                    us_count += 1;
                    if(us_count > 5){
                        //trigger alarm
                        al_raise();
                    }
            }
            else{
                us_count = 0;
            }
    }
    prev_accl_state = curr_accl_state;
		if(accel_state == USAFE_HIGH){
			do{
			    alarm_raise();
			}while(!alarm_reset);

			etimer_set(&et, CLOCK_SECOND/100);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		}
}
*/
/*---------------------------------------------------------------------------*/

/* Observer process  */
PROCESS_THREAD(obs_process, ev, data) {

	static struct etimer periodic_timer;
	int is_coordinator;
	uip_ipaddr_t dest_ipaddr;

	PROCESS_BEGIN();
        SENSORS_ACTIVATE(button_sensor);

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

	// With random_rand()
        //etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	etimer_set(&periodic_timer, SEND_INTERVAL);

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

                leds_toggle(LEDS_RED);  
                ACCEL_STATES accl_state;
                accl_state = current_state();
                //printf("accl_state: %d \n", accl_state);

                DIST_STATES dist_state;
                dist_state = current_state_dist();
                //printf("dist_state: %d \n", dist_state);

                if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
                  clock_time_t network_uptime;
                  network_uptime = tsch_get_network_uptime_ticks();
                  struct Data_pkt data_tx = {
                      network_uptime,
                      accl_state,
                      dist_state
                  };
                  simple_udp_sendto(&client_conn, &data_tx, sizeof(data_tx), &dest_ipaddr);
                  LOG_INFO("Sent accl_state %d and dist_state %d to ", (int)accl_state, (int)dist_state);
                  LOG_INFO_6ADDR(&dest_ipaddr);
                  LOG_INFO(" at time %d to ", (int)network_uptime);
                  LOG_INFO_("\n");
                } 
                else {
                  LOG_INFO("Not reachable yet\n");
                }

                etimer_set(&periodic_timer, SEND_INTERVAL);

		//PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event || data == &button_sensor);

                /*
                 * If button is pressed on node, check lock flag and then change state accordingly
                 * */
                /*
	        if(data == &button_sensor) {
                    if(lk_flag) {
                        lk_unlock();
                    }
                    else {
                        lk_lock();
                    }
		}
                */
		//printf("rx: detection event!! \n");
	}

	PROCESS_END();
}
