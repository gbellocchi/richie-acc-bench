/*
 * Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>
 */

struct timer_host {
    struct timespec t0;
    struct timespec t1;
    float t_meas;   
};

struct timer_xil_exec {
    float t_meas_progr;  
    float t_meas_compute; 
};

typedef struct timer_host             timer_host;
typedef struct timer_xil_exec         timer_xil_exec;