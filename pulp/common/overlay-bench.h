/*
 * Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>
 */

#include <hero-target.h>

struct timer_host {
    struct timespec t0;
    struct timespec t1;
    float t_meas;   
};

struct load_struct {
    hero_dma_job_t dma_job_in;
    uint32_t clk_0;
    uint32_t clk_1;
};

struct compute_struct {
    uint32_t clk_0;
    uint32_t clk_1;
};

struct store_struct {
    hero_dma_job_t dma_job_out;
    uint32_t clk_0;
    uint32_t clk_1;
};

struct hwpe_progr_struct {
    uint32_t clk_0;
    uint32_t clk_1;
};

// struct eu_struct {
//     int req_eu;
//     uint32_t clk_0;
//     uint32_t clk_1;
// };

struct hwpe_addr_gen_struct {
    /* Basic addressing */
    unsigned trans_size;  
    unsigned line_stride; 
    unsigned line_length; 
    unsigned feat_stride; 
    unsigned feat_length; 
    unsigned feat_roll; 
    unsigned loop_outer; 
    unsigned realign_type; 
    /* Data tiling */
    unsigned tile_size;
    /* Double buffering */
    unsigned buffer_select;
    /* Spatial parallelism */
    unsigned port_offset; 
    unsigned step;
};

typedef struct timer_host               timer_host;
typedef struct load_struct              load_struct;
typedef struct compute_struct           compute_struct;
typedef struct store_struct             store_struct;
typedef struct hwpe_progr_struct        hwpe_progr_struct;
typedef struct eu_struct                eu_struct;
typedef struct hwpe_addr_gen_struct     hwpe_addr_gen_struct;