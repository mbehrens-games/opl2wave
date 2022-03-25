/*******************************************************************************
** envelope.h (amplitude envelope)
*******************************************************************************/

#ifndef ENVELOPE_H
#define ENVELOPE_H

enum
{
  ENVELOPE_STATE_ATTACK,
  ENVELOPE_STATE_DECAY,
  ENVELOPE_STATE_SUSTAIN,
  ENVELOPE_STATE_RELEASE
};

typedef struct envelope
{
  /* current state (a/d/s/r) */
  int           state;
  
  /* adsr indices to lookup table (0 to 63) */
  unsigned char a_index;
  unsigned char d_index;
  unsigned char s_index;
  unsigned char r_index;

  /* total level, sustain level (10 bit) */
  short int     total_bound;
  short int     sustain_bound;

  /* current attenuation, update period, cycle counter */
  short int     attenuation;
  short int     period;
  short int     cycles;
  unsigned char increment_index;
} envelope;

/* function declarations */
short int   envelope_init(envelope* e);
envelope*   envelope_create();
short int   envelope_deinit(envelope* e);
short int   envelope_destroy(envelope* e);

short int   envelope_setup( envelope* e, 
                            unsigned char ar, unsigned char dr, 
                            unsigned char sr, unsigned char rr, 
                            unsigned char sl, unsigned char tl, 
                            int rks, int lks, 
                            int fnum, int block, int kc);
short int   envelope_change_state(envelope* e, int state);
short int   envelope_update(envelope* e);

#endif
