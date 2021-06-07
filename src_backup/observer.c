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
#include "accl.h"
#include "dist.h"

//#include "/home/wcnes/smart-bike-rack/src/accl.h"
//#include "/home/wcnes/smart-bike-rack/src/dist.h"

#define OBS_INTERVAL CLOCK_SECOND/100
//#define ACCEL_RANGE 71

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define CHECK_INTERVAL  (CLOCK_SECOND / 10)
#define SEND_INTERVAL	(CLOCK_SECOND * 60 )

static struct simple_udp_connection client_conn, server_conn;
//static process_event_t detection_event;

extern uint32_t	accel_data[3][50];	//[0]->x, [1]->y, [2]->z 

struct Data_pkt 
{
	clock_time_t timestamp;
	ACCEL_STATES state_accl;
	DIST_STATES  state_dist;
	uint16_t	accdata;
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
	
	uint16_t rssi_rcv_pkt = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	uint16_t lqi_rcv_pkt = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

	memcpy(&data_rx, data, sizeof(struct Data_pkt));
	LOG_INFO("Received from:\t\t ");
	LOG_INFO_6ADDR(sender_addr);
	printf("RSSI from received packet:\t\t%d\n", rssi_rcv_pkt);
	printf("LQI from received packet:\t\t%d\n", lqi_rcv_pkt);
	LOG_INFO(" \n");
	printf("data received:\t\t%d at time:\t\t%lu \n", (int)data_rx.state_accl, (unsigned long)(local_time_clock_ticks - data_rx.timestamp));
	printf("dist_state rx:\t\t%d \n", (int)data_rx.state_dist);
}
/*---------------------------------------------------------------------------*/
void route_to_root(void){
	
}


/*-------check accelerometer state-----*/
void check_accl_state(void) {
	static uint16_t us_count = 0;    

	//ACCEL_STATES UNSAFE_HIGH_STATE = UNSAFE_HIGH;
	ACCEL_STATES curr_state;	// prev_state;

	curr_state = current_state(); // read current state
	if(curr_state == UNSAFE_HIGH){ //curr_accl_state
		//if(prev_state == UNSAFE_HIGH){
		us_count += 1;
		if(us_count > 5){
			//trigger alarm
			//al_raise();
			us_count = 0;
		}
	}
		else{
			us_count = 0;
		}
		//}
		//	prev_state = curr_state;
}


/*----------send data to root------------------*/
void send_to_root(void){

	struct Data_pkt data_tx;
	uip_ipaddr_t dest_ipaddr;

	leds_on(LEDS_RED);  
	//ACCEL_STATES accl_state;
	data_tx.state_accl = current_state();
	printf("accl_state: %d \n", data_tx.state_accl);

	//DIST_STATES dist_state;
	data_tx.state_dist = current_state_dist();
	printf("dist_state: %d \n", data_tx.state_dist);

	data_tx.accdata = accel_data_avg();

	if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
		//clock_time_t network_uptime;
		data_tx.timestamp = tsch_get_network_uptime_ticks();
		//		data_tx = {
		//			network_uptime,
		//			accl_state,
		//			dist_state
		//			
		//		};
		simple_udp_sendto(&client_conn, &data_tx, sizeof(data_tx), &dest_ipaddr);
		LOG_INFO("Sent accl_state %d and dist_state %d to ", (int)data_tx.state_accl, (int)data_tx.state_accl);
		LOG_INFO_6ADDR(&dest_ipaddr);
		LOG_INFO(" at time %d to ", (int)data_tx.timestamp);
		LOG_INFO_("\n");
	} 
	else {
		LOG_INFO("Not reachable yet\n");
	}

	leds_on(LEDS_RED);  
}



/*---------------------------------------------------------------------------*/
/* Observer process  */
PROCESS_THREAD(obs_process, ev, data) {

	static struct etimer periodic_timer_check, periodic_timer_send;
	int is_coordinator;

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
	etimer_set(&periodic_timer_check, CHECK_INTERVAL);
	etimer_set(&periodic_timer_send, SEND_INTERVAL);

	/* log data function TBD  */
	//log_data();

#if BORDER_ROUTER_CONF_WEBSERVER
	PROCESS_NAME(webserver_nogui_process);
	process_start(&webserver_nogui_process, NULL);
#endif /* BORDER_ROUTER_CONF_WEBSERVER */


	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer_check));

		if(tsch_is_coordinator){
			break;
		}
		//	if(etimer_expired(&periodic_timer_check)){
		check_accl_state();

		//	}	

		if(etimer_expired(&periodic_timer_send)){

			send_to_root();
			etimer_set(&periodic_timer_send, SEND_INTERVAL);
		}

		etimer_set(&periodic_timer_check, CHECK_INTERVAL);


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


