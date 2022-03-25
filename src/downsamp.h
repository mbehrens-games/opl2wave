/*******************************************************************************
** downsamp.h (downsampling functions)
*******************************************************************************/

#ifndef DOWNSAMP_H
#define DOWNSAMP_H

/* function declarations */
short int downsamp_apply_filter(short int* buffer, int size);
short int downsamp_perform_downsample(short int*  sample_buffer, 
                                      int         sample_buffer_size, 
                                      short int*  export_buffer, 
                                      int         export_buffer_size);

#endif
