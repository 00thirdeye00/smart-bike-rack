/* accl.h  */
#ifndef ACCL_H
#define ACCL_H




/* accelerometer states  */
enum ACCEL_STATES {
	SAFE_HIGH,
	SAFE_LOW,
	USAFE_LOW,
	USAFE_HIGH,
};


/* function to return current accelerometer state  */
ACCEL_STATES current_state(void);





#endif //ACCL_H
