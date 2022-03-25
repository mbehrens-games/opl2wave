/*******************************************************************************
** channel.h (instrument channel)
*******************************************************************************/

#ifndef CHANNEL_H
#define CHANNEL_H

#include "envelope.h"
#include "lfo.h"

#define CHANNEL_NUM_OPS 2

enum
{
  CHANNEL_WAVEFORM_SINE,
  CHANNEL_WAVEFORM_HALF_RECT,
  CHANNEL_WAVEFORM_FULL_RECT,
  CHANNEL_WAVEFORM_QUARTER
};

typedef struct channel
{
  /* envelope settings */
  unsigned char   ar[CHANNEL_NUM_OPS];  /* 0-15 */
  unsigned char   dr[CHANNEL_NUM_OPS];  /* 0-15 */
  unsigned char   sr[CHANNEL_NUM_OPS];  /* 0-15 */
  unsigned char   rr[CHANNEL_NUM_OPS];  /* 0-15 */
  unsigned char   tl[CHANNEL_NUM_OPS];  /* 0-63 */
  unsigned char   sl[CHANNEL_NUM_OPS];  /* 0-15 */

  int             rks[CHANNEL_NUM_OPS];
  int             lks[CHANNEL_NUM_OPS];

  /* multiple, detune (fine/coarse) */
  int             multiple[CHANNEL_NUM_OPS];
  int             detune_coarse[CHANNEL_NUM_OPS];
  int             detune_fine[CHANNEL_NUM_OPS];

  /* pms, ams */
  int             pms[CHANNEL_NUM_OPS];
  int             ams[CHANNEL_NUM_OPS];

  /* algorithm, waveform */
  int             waveform[CHANNEL_NUM_OPS];
  int             algorithm;

  /* feedback, resultant shift amount */
  /* 1: PI/16   */
  /* 2: PI/8    */
  /* 3: PI/4    */
  /* 4: PI/2    */
  /* 5: PI      */
  /* 6: 2 PI    */
  /* 7: 4 PI    */
  int             fb;
  short int       fb_shift;

  /* note, block/fnum/keycode, increment */
  char            note;
  char            block;
  short int       fnum;
  int             kc;
  int             increment[CHANNEL_NUM_OPS];

  /* phases */
  int             phase[CHANNEL_NUM_OPS];

  /* envelopes */
  envelope        env[CHANNEL_NUM_OPS];

  /* lfos */
  lfo             vibrato;
  lfo             tremolo;

  /* vibrato / tremolo levels */
  char            pmd;
  char            amd;

  /* levels */
  int             feedin1;
  int             feedin2;
  short int       level;
} channel;

/* function declarations */
short int   channel_init(channel* c);
channel*    channel_create();
short int   channel_deinit(channel* c);
short int   channel_destroy(channel* c);

short int   channel_is_inactive(channel* c);
short int   channel_setup(channel* c);
short int   channel_key_on(channel* c);
short int   channel_key_off(channel* c);
short int   channel_update(channel* c);

#endif
