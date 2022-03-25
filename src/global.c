/*******************************************************************************
** global.c (global variables)
*******************************************************************************/

#include <stdio.h>
#include <math.h>

#include "channel.h"
#include "clock.h"
#include "filter.h"
#include "global.h"
#include "lfo.h"
#include "tuning.h"

/* global variables */
short int G_db_to_linear[8192];
short int G_wavetable[256];
float     G_downsampling_kernel[(DOWNSAMPLING_M_MAX / 2) + 1];

channel   G_channel;
filter    G_filter;

float     G_export_length;
int       G_export_sampling;
int       G_export_period;
int       G_export_bitres;

int       G_downsampling_m;
int       G_downsampling_bound;

int       G_tuning_system;
int       G_tuning_fork;

float     G_note_length;

/*******************************************************************************
** globals_init()
*******************************************************************************/
short int globals_init()
{
  int     i;
  double  val;

  /* ym2612 - 10 bit envelope (shifted to 12 bit), 12 bit sine, 13 bit sum    */
  /* 10 bit db: 24, 12, 6, 3, 1.5, 0.75, 0.375, 0.1875, 0.09375, 0.046875     */
  /* 12 bit db: adds on 0.0234375, 0.01171875 in back                         */
  /* 13 bit db: adds on 48 in front                                           */

  /* db to linear scale conversion */
  G_db_to_linear[0] = 32767;

  for (i = 1; i < 8191; i++)
  {
    G_db_to_linear[i] = 
      (short int) (32767.0f * exp(-log(10) * (DB_STEP / 10) * i));
  }

  G_db_to_linear[8191] = 0;

  /* wavetable */
  G_wavetable[0] = 4095;

  for (i = 1; i < 255; i++)
  {
    val = sin(TWO_PI * (i / 1024.0f));
    G_wavetable[i] = (short int) (10 * (log(1 / val) / log(10)) / DB_STEP);
  }

  G_wavetable[255] = 0;

  /* downsampling filter kernel initialized to zeroes */
  for (i = 0; i < (DOWNSAMPLING_M_MAX / 2) + 1; i++)
  {
    G_downsampling_kernel[i] = 0.0f;
  }

  /* initialize variables */
  channel_init(&G_channel);
  filter_init(&G_filter);

  G_export_length = 0.0f;
  G_export_sampling = 44100;
  G_export_period = 22676;
  G_export_bitres = 16;

  G_downsampling_m = 128;
  G_downsampling_bound = (G_downsampling_m / 2) + 1;

  G_tuning_fork = TUNING_SYSTEM_12_ET;
  G_tuning_fork = TUNING_FORK_A440;

  G_note_length = 0.0f;

  return 0;
}

/*******************************************************************************
** globals_deinit()
*******************************************************************************/
short int globals_deinit()
{
  channel_deinit(&G_channel);
  filter_deinit(&G_filter);

  return 0;
}

/*******************************************************************************
** globals_compute_sinc_filter()
*******************************************************************************/
short int globals_compute_sinc_filter()
{
  int i;

  float fc;
  float sum;

  /* the cutoff frequency is a fraction of the sampling rate */
  fc = (G_export_sampling / 2.0f) / ADLIB_CLOCK;

  /* shift fc so that the transition band ends at the intended frequency */
  fc -= 2.0f / (float) G_downsampling_m;

  /* The equation is from Steven W. Smith's The Scientist and Engineer's  */
  /* Guide to Digital Signal Processing, page 290 (Ch. 16)                */

  /* sinc filter */
  for (i = 0; i < G_downsampling_bound; i++)
  {
    if (i == G_downsampling_m / 2)
      G_downsampling_kernel[i] = TWO_PI * fc;
    else
    {
      G_downsampling_kernel[i] = sin(TWO_PI * fc * (i - G_downsampling_m / 2)) / (i - G_downsampling_m / 2);
      G_downsampling_kernel[i] *= 0.42  - 0.5 * cos(TWO_PI * i / (float) G_downsampling_m)
                                  + 0.08 * cos(2 * TWO_PI * i / (float) G_downsampling_m);
    }
  }

  /* normalization */
  sum = G_downsampling_kernel[G_downsampling_m / 2];

  for (i = 0; i < G_downsampling_bound - 1; i++)
    sum += 2 * G_downsampling_kernel[i];

  for (i = 0; i < G_downsampling_bound; i++)
    G_downsampling_kernel[i] /= sum;

#if 0
  /* testing: check filter values */
  for (i = 0; i < G_downsampling_bound; i++)
    fprintf(stdout, "Sinc Filter value: %f\n", G_downsampling_kernel[i]);
#endif

  return 0;
}

/*******************************************************************************
** globals_wavetable_lookup()
*******************************************************************************/
short int globals_wavetable_lookup(int phase, int env_index, int waveform)
{
  short int level;

  /* bound phase */
  phase &= 0x3FF;

  /* determine enveloped sine wave value */
  if (phase < 256)
    level = G_db_to_linear[G_wavetable[phase] + env_index];
  else if (phase < 512)
    level = G_db_to_linear[G_wavetable[511 - phase] + env_index];
  else if (phase < 768)
    level = -G_db_to_linear[G_wavetable[phase - 512] + env_index];
  else
    level = -G_db_to_linear[G_wavetable[1023 - phase] + env_index];

  /* apply waveform modifier */
  if (waveform == CHANNEL_WAVEFORM_HALF_RECT)
  {
    if (phase >= 512)
      level = 0;
  }
  else if (waveform == CHANNEL_WAVEFORM_FULL_RECT)
  {
    if (phase >= 512)
      level = -level;
  }
  else if (waveform == CHANNEL_WAVEFORM_QUARTER)
  {
    if ((phase >= 256) && (phase < 512))
      level = 0;
    else if ((phase >= 512) && (phase < 768))
      level = -level;
    else if ((phase >= 768) && (phase < 1024))
      level = 0;
  }

  return level;
}

