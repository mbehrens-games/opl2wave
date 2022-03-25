/*******************************************************************************
** global.h (global variables)
*******************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "channel.h"
#include "filter.h"

#define DB_STEP     0.01171875
#define ONE_BILLION 1000000000

#define PI          3.14159265358979323846
#define TWO_PI      6.28318530717958647693

#define DOWNSAMPLING_M_MAX 512

extern short int  G_db_to_linear[];
extern short int  G_wavetable[];
extern float      G_downsampling_kernel[];

extern channel    G_channel;
extern filter     G_filter;

extern float      G_export_length;
extern int        G_export_sampling;
extern int        G_export_period;
extern int        G_export_bitres;

extern int        G_downsampling_m;
extern int        G_downsampling_bound;

extern int        G_tuning_system;
extern int        G_tuning_fork;

extern float      G_note_length;

/* function declarations */
short int globals_init();
short int globals_deinit();

short int globals_compute_sinc_filter();
short int globals_wavetable_lookup(int phase, int env_index, int waveform);

#endif
