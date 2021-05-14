/* accl.h  */
#ifndef ACCL_H
#define ACCL_H




/* accelerometer states  */
typedef enum accel_states {
	SAFE_HIGH,
	SAFE_LOW,
	USAFE_LOW,
	USAFE_HIGH,
}ACCEL_STATES;


/* function to return current accelerometer state  */
ACCEL_STATES current_state(void);





#endif //ACCL_H
