#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/z1-phidgets.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"

#include "dist.h"

#define ACCM_READ_INTERVAL CLOCK_SECOND/100
#define DIST_RANGE 700


static int64_t dist_data;
uint8_t err_dist_cnt;

PROCESS(dist_process, "Distance process");


DIST_STATES dist_state = UNDETECTED;

/* function to return current state  */
DIST_STATES current_state_dist(void){
	return dist_state;
}

void dist_read(void) {
    static int16_t dist_read;

    dist_read = phidgets.value(PHIDGET5V_2);
    //printf("dist_read: %d \n", dist_read);

    if( dist_read  < DIST_RANGE ) {
        if(err_dist_cnt == 0) {
            dist_state = UNDETECTED;
        } 
        else if( err_dist_cnt > 0 ) {
            err_dist_cnt -= 1;
        }
    }
    else {
        err_dist_cnt += 1;
        if( err_dist_cnt >= 50) {
            dist_state = DETECTED;
        }
    }
    dist_data = dist_data + dist_read;
}

static struct etimer et_dist;
PROCESS_THREAD(dist_process, ev, data) {
    PROCESS_BEGIN();
    SENSORS_ACTIVATE(phidgets);
    while (1) {

    dist_read();

    if( err_dist_cnt == 0) {
        // wait for timer interrupt 1000 ms
        etimer_set(&et_dist, CLOCK_SECOND/10);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_dist));
    }
    else if(err_dist_cnt > 0) {
        // wait for timer interrupt 100 ms
        etimer_set(&et_dist, CLOCK_SECOND/100);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_dist));
    }
    }

    PROCESS_END();
}
