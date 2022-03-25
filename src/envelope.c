/*******************************************************************************
** envelope.c (amplitude envelope)
*******************************************************************************/

#include <stdlib.h>

#include "envelope.h"

/* this table is adapted from fmopl.cpp in mame */
/* note that rates 62 & 63 double in attack mode */
static unsigned char S_opl_att_inc_table[64][8] =
  {{0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {0,1,0,1,0,1,0,1}, {0,1,0,1,1,1,0,1}, {0,1,1,1,0,1,1,1}, {0,1,1,1,1,1,1,1},
   {1,1,1,1,1,1,1,1}, {1,1,1,2,1,1,1,2}, {1,2,1,2,1,2,1,2}, {1,2,2,2,1,2,2,2},
   {2,2,2,2,2,2,2,2}, {2,2,2,4,2,2,2,4}, {2,4,2,4,2,4,2,4}, {2,4,4,4,2,4,4,4},
   {4,4,4,4,4,4,4,4}, {4,4,4,4,4,4,4,4}, {4,4,4,4,4,4,4,4}, {4,4,4,4,4,4,4,4}};

/* this table is adapted from fmopl.c in mame */
static char S_opl_level_scaling_table[16] =
  {0, 48, 64, 74, 80, 86, 90, 94, 96, 100, 102, 104, 106, 108, 110, 112};

/*******************************************************************************
** envelope_init()
*******************************************************************************/
short int envelope_init(envelope* e)
{
  if (e == NULL)
    return 1;

  e->state  = ENVELOPE_STATE_RELEASE;

  e->a_index  = 0;
  e->d_index  = 0;
  e->s_index  = 0;
  e->r_index  = 0;

  e->total_bound    = 511;
  e->sustain_bound  = 511;

  e->attenuation      = 511;
  e->period           = 1;
  e->cycles           = 0;
  e->increment_index  = 0;

  return 0;
}

/*******************************************************************************
** envelope_create()
*******************************************************************************/
envelope* envelope_create()
{
  envelope* e;

  e = malloc(sizeof(envelope));
  envelope_init(e);

  return e;
}

/*******************************************************************************
** envelope_deinit()
*******************************************************************************/
short int envelope_deinit(envelope* e)
{
  if (e == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** envelope_destroy()
*******************************************************************************/
short int envelope_destroy(envelope* e)
{
  if (e == NULL)
    return 1;

  envelope_deinit(e);
  free(e);

  return 0;
}

/*******************************************************************************
** envelope_setup()
*******************************************************************************/
short int envelope_setup( envelope* e, 
                          unsigned char ar, unsigned char dr, 
                          unsigned char sr, unsigned char rr, 
                          unsigned char sl, unsigned char tl, 
                          int rks, int lks, 
                          int fnum, int block, int kc)
{
  int lks_adjust;

  if (e == NULL)
    return 1;

  /* set adsr indices */
  e->a_index = 4 * ar;
  e->d_index = 4 * dr;
  e->s_index = 4 * sr;
  e->r_index = 4 * rr;

  /* apply rate scaling */
  /* rks 0 */
  if (rks == 0)
  {
    e->a_index += kc / 8;
    e->d_index += kc / 8;
    e->s_index += kc / 8;
    e->r_index += kc / 8;
  }
  /* rks 1, note select 0 */
  else if (rks == 1)
  {
    e->a_index += kc / 2;
    e->d_index += kc / 2;
    e->s_index += kc / 2;
    e->r_index += kc / 2;
  }
  /* rks 1, note select 1 */
  else if (rks == 2)
  {
    e->a_index += ((kc & 0x1C) >> 1) | (kc & 0x01);
    e->d_index += ((kc & 0x1C) >> 1) | (kc & 0x01);
    e->s_index += ((kc & 0x1C) >> 1) | (kc & 0x01);
    e->r_index += ((kc & 0x1C) >> 1) | (kc & 0x01);
  }

  /* bound adsr indices */
  if (e->a_index > 63)
    e->a_index = 63;
  if (e->d_index > 63)
    e->d_index = 63;
  if (e->s_index > 63)
    e->s_index = 63;
  if (e->r_index > 63)
    e->r_index = 63;

  /* set level scaling amount */
  lks_adjust = S_opl_level_scaling_table[(fnum & 0x780) >> 7]
                  - ((8 - block) * 16);

  if (lks_adjust < 0)
    lks_adjust = 0;

  lks_adjust = lks_adjust >> 1;

  if (lks == 1)
    tl += lks_adjust >> 2;
  else if (lks == 2)
    tl += lks_adjust >> 1;
  else if (lks == 3)
    tl += lks_adjust;

  /* bound tl */
  if (tl > 127)
    tl = 127;

  /* initialize attentuation */
  e->attenuation = 511;

  /* set sustain bound (9 bit bounds) */
  if (sl == 15)
    e->sustain_bound = 511;
  else
    e->sustain_bound = sl << 4;

  if (e->sustain_bound < 0)
    e->sustain_bound = 0;
  else if (e->sustain_bound > 511)
    e->sustain_bound = 511;

  /* set total bound (9 bit bounds) */
  e->total_bound = tl << 2;

  if (e->total_bound < 0)
    e->total_bound = 0;
  else if (e->total_bound > 511)
    e->total_bound = 511;

  /* set default state */
  e->state = ENVELOPE_STATE_RELEASE;

  return 0;
}

/*******************************************************************************
** envelope_change_state()
*******************************************************************************/
short int envelope_change_state(envelope* e, int state)
{
  if (e == NULL)
    return 1;

  /* make sure this is a valid state */
  if ((state != ENVELOPE_STATE_ATTACK)  && (state != ENVELOPE_STATE_DECAY)  &&
      (state != ENVELOPE_STATE_SUSTAIN) && (state != ENVELOPE_STATE_RELEASE))
    return 1;

  e->state = state;

  /* update period */
  if (e->state == ENVELOPE_STATE_ATTACK)
  {
    if (e->a_index <= 47)
      e->period = 1 << (12 - (e->a_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_DECAY)
  {
    if (e->d_index <= 47)
      e->period = 1 << (12 - (e->d_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_SUSTAIN)
  {
    if (e->s_index <= 47)
      e->period = 1 << (12 - (e->s_index / 4));
    else
      e->period = 1;
  }
  else if (e->state == ENVELOPE_STATE_RELEASE)
  {
    if (e->r_index <= 47)
      e->period = 1 << (12 - (e->r_index / 4));
    else
      e->period = 1;
  }

  return 0;
}

/*******************************************************************************
** envelope_update()
*******************************************************************************/
short int envelope_update(envelope* e)
{
  short int increment;

  if (e == NULL)
    return 1;

  /* increment cycles */
  e->cycles++;

  while (e->cycles >= e->period)
  {
    e->cycles -= e->period;
    e->increment_index = (e->increment_index + 1) % 8;

    /* attack */
    if (e->state == ENVELOPE_STATE_ATTACK)
    {
      increment = S_opl_att_inc_table[e->a_index][e->increment_index];

      if (e->a_index >= 62)
        increment *= 2;

      /* if ar is 0, the operator assesses the situation */
      if (e->a_index >= 4)
        e->attenuation += (~e->attenuation * increment) >> 3;

      if (e->attenuation <= 0)
      {
        e->attenuation = 0;
        envelope_change_state(e, ENVELOPE_STATE_DECAY);
      }
    }
    /* decay */
    else if (e->state == ENVELOPE_STATE_DECAY)
    {
      increment = S_opl_att_inc_table[e->d_index][e->increment_index];
      e->attenuation += increment;

      if (e->attenuation >= e->sustain_bound)
        envelope_change_state(e, ENVELOPE_STATE_SUSTAIN);
    }
    /* sustain */
    else if (e->state == ENVELOPE_STATE_SUSTAIN)
    {
      /* if sr is 0, the envelope holds at the sustain level  */
      /* otherwise, sr is ignored and the envelope uses rr    */
      if (e->s_index >= 4)
      {
        increment = S_opl_att_inc_table[e->r_index][e->increment_index];
        e->attenuation += increment;
      }

      if (e->attenuation >= 511)
        e->attenuation = 511;
    }
    /* release */
    else if (e->state == ENVELOPE_STATE_RELEASE)
    {
      increment = S_opl_att_inc_table[e->r_index][e->increment_index];
      e->attenuation += increment;

      if (e->attenuation >= 511)
        e->attenuation = 511;
    }
  }

  return 0;
}

