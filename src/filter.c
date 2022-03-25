/*******************************************************************************
** filter.c (filter)
*******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "clock.h"
#include "global.h"
#include "filter.h"

/*******************************************************************************
** filter_init()
*******************************************************************************/
short int filter_init(filter* fltr)
{
  if (fltr == NULL)
    return 1;

  fltr->cutoff  = 0;
  fltr->q       = 0.5f;

  fltr->b_0   = 0.0f;
  fltr->b_1   = 0.0f;
  fltr->b_2   = 0.0f;
  fltr->a_1   = 0.0f;
  fltr->a_2   = 0.0f;

  fltr->x[0]  = 0;
  fltr->x[1]  = 0;
  fltr->x[2]  = 0;
  fltr->y[0]  = 0;
  fltr->y[1]  = 0;
  fltr->y[2]  = 0;

  return 0;
}

/*******************************************************************************
** filter_create()
*******************************************************************************/
filter* filter_create()
{
  filter* fltr;

  fltr = malloc(sizeof(filter));
  filter_init(fltr);

  return fltr;
}

/*******************************************************************************
** filter_deinit()
*******************************************************************************/
short int filter_deinit(filter* fltr)
{
  if (fltr == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** filter_destroy()
*******************************************************************************/
short int filter_destroy(filter* fltr)
{
  if (fltr == NULL)
    return 1;

  filter_deinit(fltr);
  free(fltr);

  return 0;
}

/*******************************************************************************
** filter_setup()
*******************************************************************************/
short int filter_setup(filter* fltr)
{
  float omega_0_delta_t;
  float alpha;

  if (fltr == NULL)
    return 1;

  if (fltr->cutoff == 0)
    return 0;

  if ((fltr->cutoff < 2500) || (fltr->cutoff > 5000))
    return 1;

  /* set resonance */
  /* q = 1/sqrt(2) gives a 2nd order butterworth filter */
  fltr->q = 0.70710678118655;

  /* filter coefficient calculation */
  /* special thanks to Robert Bristow-Johnson's Audio EQ Cookbook */

  /* initialize variables */
  omega_0_delta_t = TWO_PI * fltr->cutoff * ((double) ADLIB_DELTA_T_NANOSECONDS / ONE_BILLION);
  alpha = sin(omega_0_delta_t) / (2 * fltr->q);

  /* 2nd order iir lowpass filter */
  fltr->b_0 = (1 - cos(omega_0_delta_t)) / (2 * (1 + alpha));
  fltr->b_1 = 2 * fltr->b_0;
  fltr->b_2 = fltr->b_0;
  fltr->a_1 = (-2 * cos(omega_0_delta_t)) / (1 + alpha);
  fltr->a_2 = (1 - alpha) / (1 + alpha);

  /* initialize sample buffers */
  fltr->x[0] = 0;
  fltr->x[1] = 0;
  fltr->x[2] = 0;
  fltr->y[0] = 0;
  fltr->y[1] = 0;
  fltr->y[2] = 0;

  return 0;
}

/*******************************************************************************
** filter_update()
*******************************************************************************/
short int filter_update(filter* fltr, short int input)
{
  int level;

  if (fltr == NULL)
    return 1;

  if (fltr->cutoff == 0)
    return 0;

  /* update filter */
  fltr->x[2] = fltr->x[1];
  fltr->x[1] = fltr->x[0];
  fltr->x[0] = input;

  fltr->y[2] = fltr->y[1];
  fltr->y[1] = fltr->y[0];

  level = (int) (((fltr->b_0 * fltr->x[0]) +
                  (fltr->b_1 * fltr->x[1]) +
                  (fltr->b_2 * fltr->x[2]) -
                  (fltr->a_1 * fltr->y[1]) -
                  (fltr->a_2 * fltr->y[2])) + 0.5);

  if (level > 32767)
    level = 32767;
  else if (level < -32768)
    level = -32768;

  fltr->y[0] = (short int) level;

  return 0;
}

