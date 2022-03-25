/*******************************************************************************
** lfo.c (vibrato / tremolo)
*******************************************************************************/

#include <stdio.h>    /* testing */
#include <stdlib.h>

#include "global.h"
#include "lfo.h"

/* these tables are adapted from fmopl.cpp in mame */

/* opl vibrato                          */
/* fnum: 8 levels from the top 3 bits   */
/* rows: 2 vibrato depths               */
/* index: 8 steps in the triangle wave  */
static char S_opl_vib_table[8][2][8];

/* opl tremolo                          */
/* rows: 2 tremolo depths               */
/* index: 52 steps in the triangle wave */
static char S_opl_trem_table[2][52];

#define LFO_UPDATE_OP_LEVEL(num)                                               \
  if (l->mode == LFO_MODE_VIBRATO)                                             \
    l->level[num] = S_opl_vib_table[l->fnum][l->row[num]][l->index];           \
  else if (l->mode == LFO_MODE_TREMOLO)                                        \
    l->level[num] = S_opl_trem_table[l->row[num]][l->index];

/*******************************************************************************
** lfo_init()
*******************************************************************************/
short int lfo_init(lfo* l)
{
  int i;

  if (l == NULL)
    return 1;

  l->mode     = LFO_MODE_VIBRATO;

  l->cycles   = 0;
  l->period   = 0;
  l->index    = 0;

  l->fnum     = 0;

  for (i = 0; i < LFO_NUM_OPS; i++)
  {
    l->row[i]   = 0;
    l->level[i] = 0;
  }

  return 0;
}

/*******************************************************************************
** lfo_create()
*******************************************************************************/
lfo* lfo_create()
{
  lfo* l;

  l = malloc(sizeof(lfo));
  lfo_init(l);

  return l;
}

/*******************************************************************************
** lfo_deinit()
*******************************************************************************/
short int lfo_deinit(lfo* l)
{
  if (l == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** lfo_destroy()
*******************************************************************************/
short int lfo_destroy(lfo* l)
{
  if (l == NULL)
    return 1;

  lfo_deinit(l);
  free(l);

  return 0;
}

/*******************************************************************************
** lfo_update()
*******************************************************************************/
short int lfo_update(lfo* l)
{
  if (l == NULL)
    return 1;

  /* increment cycles */
  l->cycles++;

  /* update index */
  if (l->mode == LFO_MODE_VIBRATO)
  {
    if (l->cycles >= 1024)
    {
      l->index = (l->index + 1) % 8;
      l->cycles -= 1024;
    }
  }
  else if (l->mode == LFO_MODE_TREMOLO)
  {
    if ((l->index == 0) && (l->cycles >= 64 * 7))
    {
      l->index++;
      l->cycles -= 64 * 7;
    }
    else if ((l->index == 26) && (l->cycles >= 64 * 3))
    {
      l->index++;
      l->cycles -= 64 * 3;
    }
    else if ((l->index != 0) && (l->index != 26) && (l->cycles >= 64 * 4))
    {
      l->index = (l->index + 1) % 52;
      l->cycles -= 64 * 4;
    }
  }

  /* update levels */
  LFO_UPDATE_OP_LEVEL(0)
  LFO_UPDATE_OP_LEVEL(1)

  return 0;
}

/*******************************************************************************
** lfo_generate_tables()
*******************************************************************************/
short int lfo_generate_tables()
{
  int i;
  int j;
  int k;

  /* opl vibrato table */
  for (i = 0; i < 8; i++)
  {
    S_opl_vib_table[i][1][0] = (char) i;
    S_opl_vib_table[i][1][1] = S_opl_vib_table[i][1][0] / 2;
    S_opl_vib_table[i][1][2] = 0;
    S_opl_vib_table[i][1][3] = -S_opl_vib_table[i][1][1];
    S_opl_vib_table[i][1][4] = -S_opl_vib_table[i][1][0];
    S_opl_vib_table[i][1][5] = S_opl_vib_table[i][1][3];
    S_opl_vib_table[i][1][6] = 0;
    S_opl_vib_table[i][1][7] = S_opl_vib_table[i][1][1];

    for (k = 0; k < 8; k++)
      S_opl_vib_table[i][0][k] = S_opl_vib_table[i][1][k] / 2;

    /* multiply the values by 2 to shift the 10 bit opl fnum to 11 bits */
    for (k = 0; k < 8; k++)
    {
      S_opl_vib_table[i][0][k] *= 2;
      S_opl_vib_table[i][1][k] *= 2;
    }
  }

  /* opl tremolo table */
  for (j = 0; j < 26; j++)
  {
    S_opl_trem_table[1][j] = (char) j;
    S_opl_trem_table[1][26 + j] = (char) (26 - j);
  }

  for (j = 0; j < 52; j++)
  {
    S_opl_trem_table[0][j] = S_opl_trem_table[1][j] / 4;
  }

  return 0;
}
