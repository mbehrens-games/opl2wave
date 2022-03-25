/*******************************************************************************
** lfo.h (vibrato / tremolo)
*******************************************************************************/

#ifndef LFO_H
#define LFO_H

enum
{
  LFO_MODE_VIBRATO,
  LFO_MODE_TREMOLO
};

#define LFO_NUM_OPS           2

typedef struct lfo
{
  /* mode (vibrato, tremolo) */
  int           mode;

  /* cycle counter, period, table index */
  int           cycles;
  int           period;
  unsigned char index;

  /* top bits of fnum, table rows, current levels */
  unsigned char fnum;
  unsigned char row[LFO_NUM_OPS];
  short int     level[LFO_NUM_OPS];
} lfo;

/* function declarations */
short int   lfo_init(lfo* l);
lfo*        lfo_create();
short int   lfo_deinit(lfo* l);
short int   lfo_destroy(lfo* l);

short int   lfo_update(lfo* l);

short int   lfo_generate_tables();

#endif
