/* accl.h  */
#ifndef ACCL_H
#define ACCL_H

PROCESS_NAME(accel_process);


/* accelerometer states  */
typedef enum accel_states {
	SAFE_HIGH,
	SAFE_LOW,
	UNSAFE_LOW,
	UNSAFE_HIGH
}ACCEL_STATES;


/* function to return current accelerometer state  */
ACCEL_STATES current_state(void);
uint16_t accel_data_avg(void);




#endif //ACCL_H
