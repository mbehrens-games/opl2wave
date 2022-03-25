/*******************************************************************************
** clock.h (clock rates)
*******************************************************************************/

#ifndef CLOCK_H
#define CLOCK_H

/* Adlib Clock is the NTSC color burst frequency 3579545 / 72     */
/* Phase Increment is 2^28 (size of phase counter) / Adlib Clock  */
#define ADLIB_CLOCK                   49716
#define ADLIB_1HZ_PHASE_INCREMENT     5399.37758468098801
#define ADLIB_DELTA_T_NANOSECONDS     20114

#endif
