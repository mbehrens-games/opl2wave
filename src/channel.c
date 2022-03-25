/*******************************************************************************
** channel.c (instrument channel)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "channel.h"
#include "clock.h"
#include "envelope.h"
#include "global.h"
#include "lfo.h"
#include "tuning.h"

/* this table is adapted from information on the spritesmind.net forums */
static char S_ym2612_detune_table[32][3] =
            { {0,1,2},    {0,1,2},    {0,1,2},    {0,1,2},
              {1,2,2},    {1,2,3},    {1,2,3},    {1,2,3},
              {1,2,4},    {1,3,4},    {1,3,4},    {1,3,5},
              {2,4,5},    {2,4,6},    {2,4,6},    {2,5,7},
              {2,5,8},    {3,6,8},    {3,6,9},    {3,7,10},
              {4,8,11},   {4,8,12},   {4,9,13},   {5,10,14},
              {5,11,16},  {6,12,17},  {6,13,19},  {7,14,20},
              {8,16,22},  {8,16,22},  {8,16,22},  {8,16,22}};

/* feedback table */
static short int S_fb_shift_table[8] = {0, 8, 7, 6, 5, 4, 3, 2};

#define CHANNEL_OP_IS_INACTIVE(num)                                            \
  (c->env[num].attenuation == 511) &&                                          \
  (c->env[num].state != ENVELOPE_STATE_ATTACK)

#define CHANNEL_UPDATE_OP_ENVELOPE(num)                                        \
  envelope_update(&c->env[num]);                                               \
                                                                               \
  op_env_index[num] = c->env[num].attenuation;                                 \
  op_env_index[num] += c->env[num].total_bound;                                \
  op_env_index[num] += ((c->tremolo.level[num] * c->amd) >> 7);                \
                                                                               \
  if (op_env_index[num] > 511)                                                 \
    op_env_index[num] = 511;                                                   \
                                                                               \
  op_env_index[num] = op_env_index[num] << 3;

#define CHANNEL_UPDATE_OP_LEVEL(num)                                           \
  c->phase[num] += c->increment[num] + op_vib_level[num];                      \
  c->phase[num] &= 0xFFFFFFF;                                                  \
                                                                               \
  op_level[num] =                                                              \
    globals_wavetable_lookup( (c->phase[num] >> 18) + phase_shift,             \
                              op_env_index[num], c->waveform[num]);

#define CHANNEL_RESET_PHASE_SHIFT()                                            \
  phase_shift = 0;

#define CHANNEL_SET_PHASE_SHIFT_FROM_FEEDBACK()                                \
  if (c->fb_shift > 0)                                                         \
    phase_shift = (c->feedin1 + c->feedin2) >> c->fb_shift;                    \
  else                                                                         \
    phase_shift = 0;

#define CHANNEL_CYCLE_FEEDBACK()                                               \
  c->feedin2 = c->feedin1;                                                     \
  c->feedin1 = op_level[0];

/* level is shifted right to 13 bits, then the upper 3 bits are masked off.   */
#define CHANNEL_SET_PHASE_SHIFT_FROM_ONE_OP(num)                               \
  phase_shift = (op_level[num] >> 3) & 0x3FF;

#define CHANNEL_SET_PHASE_SHIFT_FROM_TWO_OPS(num1, num2)                       \
  phase_shift = ((op_level[num1] >> 3) & 0x3FF) +                              \
                ((op_level[num2] >> 3) & 0x3FF);

#define CHANNEL_SET_PHASE_SHIFT_FROM_THREE_OPS(num1, num2, num3)               \
  phase_shift = ((op_level[num1] >> 3) & 0x3FF) +                              \
                ((op_level[num2] >> 3) & 0x3FF) +                              \
                ((op_level[num3] >> 3) & 0x3FF);

/*******************************************************************************
** channel_init()
*******************************************************************************/
short int channel_init(channel* c)
{
  int i;

  if (c == NULL)
    return 1;

  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    c->ar[i]  = 0;
    c->dr[i]  = 0;
    c->sr[i]  = 0;
    c->rr[i]  = 0;
    c->tl[i]  = 63;
    c->sl[i]  = 0;

    c->rks[i] = 0;
    c->lks[i] = 0;

    c->multiple[i]      = 1;
    c->detune_coarse[i] = 0;
    c->detune_fine[i] = 0;

    c->pms[i] = 0;
    c->ams[i] = 0;

    c->waveform[i] = 0;
  }

  c->algorithm      = 0;

  c->fb             = 0;
  c->fb_shift       = 0;

  c->note   = 0;
  c->block  = 0;
  c->fnum   = 0;
  c->kc     = 0;

  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    c->increment[i] = 0;
    c->phase[i] = 0;
    envelope_init(&c->env[i]);
  }

  lfo_init(&c->vibrato);
  lfo_init(&c->tremolo);

  c->pmd      = 0;
  c->amd      = 0;

  c->feedin1  = 0;
  c->feedin2  = 0;
  c->level    = 0;

  return 0;
}

/*******************************************************************************
** channel_create()
*******************************************************************************/
channel* channel_create()
{
  channel* c;

  c = malloc(sizeof(channel));
  channel_init(c);

  return c;
}

/*******************************************************************************
** channel_deinit()
*******************************************************************************/
short int channel_deinit(channel* c)
{
  if (c == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** channel_destroy()
*******************************************************************************/
short int channel_destroy(channel* c)
{
  if (c == NULL)
    return 1;

  channel_deinit(c);
  free(c);

  return 0;
}

/*******************************************************************************
** channel_is_inactive()
*******************************************************************************/
short int channel_is_inactive(channel* c)
{
  if (c == NULL)
    return 1;

  /* check if carriers are inactive (in release mode with max attenuation) */
  if (c->algorithm == 0)
  {
    if (CHANNEL_OP_IS_INACTIVE(1))
      return 1;
  }
  else if (c->algorithm == 1)
  {
    if (CHANNEL_OP_IS_INACTIVE(0) &&
        CHANNEL_OP_IS_INACTIVE(1))
      return 1;
  }

  return 0;
}

/*******************************************************************************
** channel_setup()
*******************************************************************************/
short int channel_setup(channel* c)
{
  int i;

  if (c == NULL)
    return 1;

  /* setup lfos */
  c->vibrato.mode = LFO_MODE_VIBRATO;
  c->tremolo.mode = LFO_MODE_TREMOLO;

  /* set up tremolo lfo */
  /* select rows */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    if ((c->ams[i] >= 0) && (c->ams[i] <= 1))
      c->tremolo.row[i] = c->ams[i];
    else
      c->tremolo.row[i] = 0;
  }

  /* set up vibrato lfo */
  /* select rows */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    if ((c->pms[i] >= 0) && (c->pms[i] <= 1))
      c->vibrato.row[i] = c->pms[i];
    else
      c->vibrato.row[i] = 0;
  }

  /* feedback */
  if ((c->fb >= 0) && (c->fb <= 7))
    c->fb_shift = S_fb_shift_table[c->fb];
  else
    c->fb_shift = 0;

  /* additional right shift by 3 is incorporated into the fb_shift variable   */
  /* to get the 16 bit output back to the 13 bits of the opl2 op output.      */
  if (c->fb_shift > 0)
    c->fb_shift += 3;

  return 0;
}

/*******************************************************************************
** channel_key_on()
*******************************************************************************/
short int channel_key_on(channel* c)
{
  int i;

  if (c == NULL)
    return 1;

  /* if note is out of the range A0 - C8, ignore */
  if ((c->note < 21) || (c->note > 108))
    return 0;

  /* set phase increments */
  c->increment[0] = tuning_compute_phase_increment(c->note);

  for (i = 1; i < CHANNEL_NUM_OPS; i++)
    c->increment[i] = c->increment[0];

  /* compute block and fnum */
  /* ym2612: 3 bit block, 11 bit fnum */
  if (c->increment[0] >= 0x1000000)
  {
    c->block = 7;
    c->fnum = (c->increment[0] >> 8) >> 6;
  }
  else if (c->increment[0] >= 0x0800000)
  {
    c->block = 6;
    c->fnum = (c->increment[0] >> 8) >> 5;
  }
  else if (c->increment[0] >= 0x0400000)
  {
    c->block = 5;
    c->fnum = (c->increment[0] >> 8) >> 4;
  }
  else if (c->increment[0] >= 0x0200000)
  {
    c->block = 4;
    c->fnum = (c->increment[0] >> 8) >> 3;
  }
  else if (c->increment[0] >= 0x0100000)
  {
    c->block = 3;
    c->fnum = (c->increment[0] >> 8) >> 2;
  }
  else if (c->increment[0] >= 0x0080000)
  {
    c->block = 2;
    c->fnum = (c->increment[0] >> 8) >> 1;
  }
  else if (c->increment[0] >= 0x0040000)
  {
    c->block = 1;
    c->fnum = (c->increment[0] >> 8);
  }
  else
  {
    c->block = 0;
    c->fnum = (c->increment[0] >> 8) << 1;
  }

  if (c->fnum <= 0)
    c->fnum = 1;
  else if (c->fnum > 2047)
    c->fnum = 2047;

  /* compute keycode */
  c->kc = ((c->block & 0x0007) << 2) + ((c->fnum >> 9) & 0x0003);

  if (c->kc < 0)
    c->kc = 0;
  else if (c->kc > 31)
    c->kc = 31;

  /* set vibrato lfo fnum */
  c->vibrato.fnum = ((c->fnum >> 8) & 0x07);

  /* set up envelopes */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    envelope_setup( &c->env[i], 
                    c->ar[i], c->dr[i], 
                    c->sr[i], c->rr[i], 
                    c->sl[i], c->tl[i], 
                    c->rks[i], c->lks[i], 
                    c->fnum, c->block, c->kc);
  }

  /* adjust increments */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    /* coarse detune */
    if (c->detune_coarse[i] != 0)
    {
      c->increment[i] += 
        tuning_compute_detune_coarse(c->block, c->fnum, c->detune_coarse[i]);
    }

    /* fine detune */
    if (c->detune_fine[i] == 1)
      c->increment[i] += (S_ym2612_detune_table[c->kc][0] << 8);
    else if (c->detune_fine[i] == 2)
      c->increment[i] += (S_ym2612_detune_table[c->kc][1] << 8);
    else if (c->detune_fine[i] == 3)
      c->increment[i] += (S_ym2612_detune_table[c->kc][2] << 8);
    else if (c->detune_fine[i] == 5)
      c->increment[i] -= (S_ym2612_detune_table[c->kc][0] << 8);
    else if (c->detune_fine[i] == 6)
      c->increment[i] -= (S_ym2612_detune_table[c->kc][1] << 8);
    else if (c->detune_fine[i] == 7)
      c->increment[i] -= (S_ym2612_detune_table[c->kc][2] << 8);

    /* multiple */
    if (c->multiple[i] == 0)
      c->increment[i] /= 2;
    else if (c->multiple[i] >= 2)
      c->increment[i] *= c->multiple[i];

    c->increment[i] &= 0xFFFFFFF;
  }

  /* reset phases */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
    c->phase[i] = 0;

  /* reset lfo phases */
  c->vibrato.cycles = 0;
  c->vibrato.index = 0;

  c->tremolo.cycles = 0;
  c->tremolo.index = 0;

  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    c->vibrato.level[i] = 0;
    c->tremolo.level[i] = 0;
  }

  /* trigger envelopes */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
    envelope_change_state(&c->env[i], ENVELOPE_STATE_ATTACK);

  return 0;
}

/*******************************************************************************
** channel_key_off()
*******************************************************************************/
short int channel_key_off(channel* c)
{
  int i;

  if (c == NULL)
    return 1;

  /* release envelopes */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
    envelope_change_state(&c->env[i], ENVELOPE_STATE_RELEASE);

  return 0;
}

/*******************************************************************************
** channel_update()
*******************************************************************************/
short int channel_update(channel* c)
{
  int       i;
  int       phase_shift;

  int       level;

  short int op_level[CHANNEL_NUM_OPS];
  short int op_env_index[CHANNEL_NUM_OPS];
  int       op_vib_level[CHANNEL_NUM_OPS];

  if (c == NULL)
    return 1;

  /* update lfos */
  lfo_update(&c->vibrato);
  lfo_update(&c->tremolo);

  /* update individual vibrato levels for each op */
  for (i = 0; i < CHANNEL_NUM_OPS; i++)
  {
    if (c->block == 0)
      op_vib_level[i] = c->vibrato.level[i] >> 1;
    else if (c->block == 1)
      op_vib_level[i] = c->vibrato.level[i];
    else if (c->block == 2)
      op_vib_level[i] = c->vibrato.level[i] << 1;
    else if (c->block == 3)
      op_vib_level[i] = c->vibrato.level[i] << 2;
    else if (c->block == 4)
      op_vib_level[i] = c->vibrato.level[i] << 3;
    else if (c->block == 5)
      op_vib_level[i] = c->vibrato.level[i] << 4;
    else if (c->block == 6)
      op_vib_level[i] = c->vibrato.level[i] << 5;
    else if (c->block == 7)
      op_vib_level[i] = c->vibrato.level[i] << 6;
    else
      op_vib_level[i] = 0;

    if (c->multiple[i] == 0)
      op_vib_level[i] /= 2;
    else if (c->multiple[i] >= 2)
      op_vib_level[i] *= c->multiple[i];

    op_vib_level[i] = (op_vib_level[i] * c->pmd) >> 7;

    op_vib_level[i] = op_vib_level[i] << 8;
  }

  /* update envelopes */
  CHANNEL_UPDATE_OP_ENVELOPE(0)
  CHANNEL_UPDATE_OP_ENVELOPE(1)

  /* update op1 and feedback */
  CHANNEL_SET_PHASE_SHIFT_FROM_FEEDBACK()
  CHANNEL_UPDATE_OP_LEVEL(0)
  CHANNEL_CYCLE_FEEDBACK()

  /* compute output level based on algorithm */
  if (c->algorithm == 0)
  {
    CHANNEL_SET_PHASE_SHIFT_FROM_ONE_OP(0)
    CHANNEL_UPDATE_OP_LEVEL(1)
    level = op_level[1];
  }
  else if (c->algorithm == 1)
  {
    CHANNEL_RESET_PHASE_SHIFT()
    CHANNEL_UPDATE_OP_LEVEL(1)
    level = op_level[0] + op_level[1];
  }
  else
    level = 0;

  /* clip level if necessary */
  if (level > 32767)
    level = 32767;
  else if (level < -32768)
    level = -32768;

  /* set channel level */
  c->level = (short int) level;

  return 0;
}

