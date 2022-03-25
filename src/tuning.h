/*******************************************************************************
** tuning.h (tuning systems)
*******************************************************************************/

#ifndef TUNING_H
#define TUNING_H

#define TUNING_PI_OVER_TWO  1.5707963267949
#define TUNING_SQRT_TWO     1.414213562373095
#define TUNING_SQRT_THREE   1.732050807568877

enum
{
  TUNING_SYSTEM_12_ET,
  TUNING_SYSTEM_PYTHAGOREAN,
  TUNING_SYSTEM_QC_MEANTONE,
  TUNING_SYSTEM_JUST,
  TUNING_SYSTEM_WERCKMEISTER_III,
  TUNING_SYSTEM_WERCKMEISTER_IV,
  TUNING_SYSTEM_WERCKMEISTER_V,
  TUNING_SYSTEM_WERCKMEISTER_VI,
  TUNING_SYSTEM_RENOLD_I
};

enum
{
  TUNING_FORK_A440,
  TUNING_FORK_A432,
  TUNING_FORK_C256,
  TUNING_FORK_AMIGA
};

/* function declarations */
short int tuning_setup();
int       tuning_compute_phase_increment(char note);
int       tuning_compute_detune_coarse( char block, int fnum, 
                                        unsigned char dt2);

#endif
