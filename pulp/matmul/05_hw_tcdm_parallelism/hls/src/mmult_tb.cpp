/*
 *
 * Author: Gianluca Bellocchi <gianluca.bellocchi@unimore.it>
 * 
 */

#include <iostream>
#include <stdlib.h>

#include "mmult.h"

#ifndef NUM_TIMES
#define NUM_TIMES 2  
#endif

int main(int argc, char** argv)
{   
    data_t mat_dim =  DATA_SIZE;
    data_t block_dim =  BLOCK_SIZE;
    data_t fsm_calls = FSM_CALLS;
    data_t partit_dim = PARTITION_SIZE;
    data_t paralllel_streams = PARALLEL_STREAMS;

    size_t square_matrix_size_bytes = sizeof(data_t) * mat_dim * mat_dim;
    size_t rect_matrix_size_bytes = sizeof(data_t) * mat_dim * block_dim;
    size_t little_square_matrix_size_bytes = sizeof(data_t) * block_dim * block_dim;

    data_t *in1 = (data_t *) malloc(square_matrix_size_bytes);
    data_t *in2 = (data_t *) malloc(square_matrix_size_bytes);
    data_t *hw_result = (data_t *) malloc(square_matrix_size_bytes);
    data_t *sw_result = (data_t *) malloc(square_matrix_size_bytes);

    bool match = true;

    //Create test data
    for (int i = 0; i < mat_dim * mat_dim; i++) {
        in1[i] = 1; //rand() % mat_dim;
        in2[i] = 1; //rand() % mat_dim;
        sw_result[i] = 0;
        hw_result[i] = 0;
    }
         
    //Launch the software solution
    mmult_sw( in1, in2, sw_result, mat_dim);

    //Launch the hardware solution

    /* TCDM buffers. */
    data_t in1_buffer[block_dim * mat_dim];
    data_t in2_buffer[mat_dim * block_dim];
    data_t out_buffer[block_dim * block_dim];

    data_t in1_0_r[partit_dim];
    data_t in1_1_r[partit_dim];
    data_t in1_2_r[partit_dim];
    data_t in1_3_r[partit_dim];
    data_t in1_4_r[partit_dim];
    data_t in1_5_r[partit_dim];
    data_t in1_6_r[partit_dim];
    data_t in1_7_r[partit_dim];
    data_t in1_8_r[partit_dim];
    data_t in1_9_r[partit_dim];
    data_t in1_10_r[partit_dim];
    data_t in1_11_r[partit_dim];
    data_t in1_12_r[partit_dim];
    data_t in1_13_r[partit_dim];
    data_t in1_14_r[partit_dim];
    data_t in1_15_r[partit_dim];

    data_t in2_0_c[partit_dim];
    data_t in2_1_c[partit_dim];
    data_t in2_2_c[partit_dim];
    data_t in2_3_c[partit_dim];
    data_t in2_4_c[partit_dim];
    data_t in2_5_c[partit_dim];
    data_t in2_6_c[partit_dim];
    data_t in2_7_c[partit_dim];
    data_t in2_8_c[partit_dim];
    data_t in2_9_c[partit_dim];
    data_t in2_10_c[partit_dim];
    data_t in2_11_c[partit_dim];
    data_t in2_12_c[partit_dim];
    data_t in2_13_c[partit_dim];
    data_t in2_14_c[partit_dim];
    data_t in2_15_c[partit_dim];

    data_t res=0;

    /* Select in1 row block. */
    for(int jj = 0; jj < mat_dim; jj += block_dim){
        /* Select in2 block column. */
        for(int kk = 0; kk < mat_dim; kk += block_dim){

            /* Software prefetching (Double-buffered in the Overlay). */
            for(int iter=0, i=0, j=0; iter<mat_dim*block_dim; iter++,j++){
                if( j== mat_dim){ j = 0; i++; }  
                in1_buffer[i*mat_dim+j] = in1[iter + jj*mat_dim];
                in2_buffer[j*block_dim+i] = in2[j*mat_dim + i + kk];
            }

            /* Hardware accelerator. */
            for (int u = 0 ; u < block_dim ; u++){
                for(int v = 0 ; v < block_dim ; v++){

                    for(int s=0; s<partit_dim; s++){  
                        in1_0_r[s]   = in1_buffer[u*mat_dim + s + 0*paralllel_streams];
                        in1_1_r[s]   = in1_buffer[u*mat_dim + s + 1*paralllel_streams];
                        in1_2_r[s]   = in1_buffer[u*mat_dim + s + 2*paralllel_streams];
                        in1_3_r[s]   = in1_buffer[u*mat_dim + s + 3*paralllel_streams];
                        in1_4_r[s]   = in1_buffer[u*mat_dim + s + 4*paralllel_streams];
                        in1_5_r[s]   = in1_buffer[u*mat_dim + s + 5*paralllel_streams];
                        in1_6_r[s]   = in1_buffer[u*mat_dim + s + 6*paralllel_streams];
                        in1_7_r[s]   = in1_buffer[u*mat_dim + s + 7*paralllel_streams];
                        in1_8_r[s]   = in1_buffer[u*mat_dim + s + 8*paralllel_streams];
                        in1_9_r[s]   = in1_buffer[u*mat_dim + s + 9*paralllel_streams];
                        in1_10_r[s]  = in1_buffer[u*mat_dim + s + 10*paralllel_streams];
                        in1_11_r[s]  = in1_buffer[u*mat_dim + s + 11*paralllel_streams];
                        in1_12_r[s]  = in1_buffer[u*mat_dim + s + 12*paralllel_streams];
                        in1_13_r[s]  = in1_buffer[u*mat_dim + s + 13*paralllel_streams];
                        in1_14_r[s]  = in1_buffer[u*mat_dim + s + 14*paralllel_streams];
                        in1_15_r[s]  = in1_buffer[u*mat_dim + s + 15*paralllel_streams];

                        in2_0_c[s]   = in2_buffer[v + s*block_dim + 0*paralllel_streams*block_dim];
                        in2_1_c[s]   = in2_buffer[v + s*block_dim + 1*paralllel_streams*block_dim];
                        in2_2_c[s]   = in2_buffer[v + s*block_dim + 2*paralllel_streams*block_dim];
                        in2_3_c[s]   = in2_buffer[v + s*block_dim + 3*paralllel_streams*block_dim];
                        in2_4_c[s]   = in2_buffer[v + s*block_dim + 4*paralllel_streams*block_dim];
                        in2_5_c[s]   = in2_buffer[v + s*block_dim + 5*paralllel_streams*block_dim];
                        in2_6_c[s]   = in2_buffer[v + s*block_dim + 6*paralllel_streams*block_dim];
                        in2_7_c[s]   = in2_buffer[v + s*block_dim + 7*paralllel_streams*block_dim];
                        in2_8_c[s]   = in2_buffer[v + s*block_dim + 8*paralllel_streams*block_dim];
                        in2_9_c[s]   = in2_buffer[v + s*block_dim + 9*paralllel_streams*block_dim];
                        in2_10_c[s]  = in2_buffer[v + s*block_dim + 10*paralllel_streams*block_dim];
                        in2_11_c[s]  = in2_buffer[v + s*block_dim + 11*paralllel_streams*block_dim];
                        in2_12_c[s]  = in2_buffer[v + s*block_dim + 12*paralllel_streams*block_dim];
                        in2_13_c[s]  = in2_buffer[v + s*block_dim + 13*paralllel_streams*block_dim];
                        in2_14_c[s]  = in2_buffer[v + s*block_dim + 14*paralllel_streams*block_dim];
                        in2_15_c[s]  = in2_buffer[v + s*block_dim + 15*paralllel_streams*block_dim];
                    }

                    mmult_hw(
                        in1_0_r,
                        in1_1_r,
                        in1_2_r,
                        in1_3_r,
                        in1_4_r,
                        in1_5_r,
                        in1_6_r,
                        in1_7_r,
                        in1_8_r,
                        in1_9_r,
                        in1_10_r,
                        in1_11_r,
                        in1_12_r,
                        in1_13_r,
                        in1_14_r,
                        in1_15_r,
                        in2_0_c,
                        in2_1_c,
                        in2_2_c,
                        in2_3_c,
                        in2_4_c,
                        in2_5_c,
                        in2_6_c,
                        in2_7_c,
                        in2_8_c,
                        in2_9_c,
                        in2_10_c,
                        in2_11_c,
                        in2_12_c,
                        in2_13_c,
                        in2_14_c,
                        in2_15_c,
                        res);

                    out_buffer[u*block_dim+v] = res; 

                    /* Write results. */
                    hw_result[u*mat_dim+v + jj*mat_dim + kk] = out_buffer[u*block_dim+v];
                }
            }
        }
    }

    //Compare the results of hardware to the software 
    for(int i=0; i< mat_dim * mat_dim; i++)
    {
        if( sw_result[i] != hw_result[i] )
        {
            std::cout << "Results Mismatch on " << "Row:" << i/mat_dim << "Col:" << i - (i/mat_dim)*mat_dim << std::endl;
            std::cout << "CPU output:" << sw_result[i] <<"\t Hardware output:" << hw_result[i] << std::endl;
            match = false;
            break;
        }
    }

    free(in1);
    free(in2);
    free(hw_result);
    free(sw_result);

    std::cout << "\n\nTEST " << (match? "PASSED\n\n": "FAILED\n\n") << std::endl;
    return(match? EXIT_SUCCESS: EXIT_FAILURE);
}



