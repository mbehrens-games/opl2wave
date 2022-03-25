/*******************************************************************************
** downsamp.c (downsampling functions)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "clock.h"
#include "downsamp.h"
#include "global.h"

/*******************************************************************************
** downsamp_apply_filter()
*******************************************************************************/
short int downsamp_apply_filter(short int* buffer, int size)
{
  int i;
  int j;

  int low_bound;
  int high_bound;

  float val;

  short int* tmp_buffer;

  /* allocate temporary buffer */
  tmp_buffer = malloc(sizeof(short int) * size);

  if (tmp_buffer == NULL)
    return 1;

  /* time domain convolution */
  for (i = 0; i < size; i++)
  {
    /* determine bounds */
    low_bound = i - (G_downsampling_m / 2);
    high_bound = i + (G_downsampling_m / 2);

    if (low_bound < 0)
      low_bound = 0;

    if (high_bound > size - 1)
      high_bound = size - 1;

    /* perform convolution with filter kernel */
    val = 0.0f;

    for (j = low_bound; j <= high_bound; j++)
    {
      if (j <= i)
        val += G_downsampling_kernel[j - i + (G_downsampling_m / 2)] * buffer[j];
      else
        val += G_downsampling_kernel[i - j + (G_downsampling_m / 2)] * buffer[j];
    }

    /* write sample to temporary buffer */
    tmp_buffer[i] = (short int) (val + 0.5);
  }

  /* copy temporary buffer to the sample buffer */
  memcpy(buffer, tmp_buffer, sizeof(short int) * size);

  /* free temporary buffer */
  if (tmp_buffer != NULL)
  {
    free(tmp_buffer);
    tmp_buffer = NULL;
  }

  return 0;
}

/*******************************************************************************
** downsamp_perform_downsample()
*******************************************************************************/
short int downsamp_perform_downsample(short int*  sample_buffer, 
                                      int         sample_buffer_size, 
                                      short int*  export_buffer, 
                                      int         export_buffer_size)
{
  int i;

  int sample_elapsed;
  int export_elapsed;

  int sample_index;

  float weight;

  /* initialize variables */
  sample_elapsed = 0;
  export_elapsed = 0;

  sample_index = 0;

  weight = 0.0f;

  /* linear interpolation */
  export_buffer[0] = sample_buffer[0];

  for (i = 1; i < export_buffer_size; i++)
  {
    export_elapsed += G_export_period;

    while (sample_elapsed + ADLIB_DELTA_T_NANOSECONDS < export_elapsed)
    {
      sample_elapsed += ADLIB_DELTA_T_NANOSECONDS;
      sample_index += 1;
    }

    if (sample_index >= sample_buffer_size - 1)
    {
      export_buffer[i] = sample_buffer[sample_buffer_size - 1];
      continue;
    }

    export_elapsed -= sample_elapsed;
    sample_elapsed = 0;

    weight = (float) export_elapsed / ADLIB_DELTA_T_NANOSECONDS;

    export_buffer[i] = (short int) (((1.0f - weight) * sample_buffer[sample_index]) + 
                                    (weight * sample_buffer[sample_index + 1]) + 0.5f);
  }

  return 0;
}

