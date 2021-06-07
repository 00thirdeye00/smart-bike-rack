/* dist.h  */
#ifndef DIST_H
#define DIST_H


PROCESS_NAME(dist_process);

/* distance states  */
typedef enum dist_states {
    UNDETECTED,
    DETECTED
}DIST_STATES;


/* function to return current accelerometer state  */
DIST_STATES current_state_dist(void);





#endif //DIST_H
