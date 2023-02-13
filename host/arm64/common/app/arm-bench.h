/*
 * Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>
 */

struct timer_host {
    struct timespec t0;
    struct timespec t1;
    float t_meas;   
};

typedef struct timer_host             timer_host;
