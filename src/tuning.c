/*******************************************************************************
** tuning.c (tuning systems)
*******************************************************************************/

#include <math.h>

#include "clock.h"
#include "global.h"
#include "tuning.h"

/* phase increment tables               */
/* notes: 12 (c to b)                   */
static int S_opl2_tuning_table[12];

/* detune coarse table                  */
/* fnum: 11 bits                        */
/* coarse detunes: 3                    */
static int S_detune_coarse_table[11][3];

/* multipliers from c in 12 tone equal temperament */
static float S_tuning_mult_12_et[12] = 
        { 1.0,                  /* C  */
          1.059463094359293,    /* C# */
          1.122462048309375,    /* D  */
          1.189207115002721,    /* D# */
          1.259921049894870,    /* E  */
          1.334839854170037,    /* F  */
          1.414213562373095,    /* F# */
          1.498307076876678,    /* G  */
          1.587401051968203,    /* G# */
          1.681792830507429,    /* A  */
          1.781797436280675,    /* A# */
          1.887748625363391};   /* B  */

/* multipliers from c in pythagorean tuning */
static float S_tuning_mult_pythagorean[12] = 
        { 1.0,                  /* 1:1        C  */
          1.06787109375,        /* 2187:2048  C# */
          1.125,                /* 9:8        D  */
          1.18518518518519,     /* 32:27      Eb */
          1.265625,             /* 81:64      E  */
          1.33333333333333,     /* 4:3        F  */
          1.423828125,          /* 729:512    F# */
          1.5,                  /* 3:2        G  */
          1.601806640625,       /* 6561:4096  G# */
          1.6875,               /* 27:16      A  */
          1.77777777777778,     /* 16:9       Bb */
          1.8984375};           /* 243:128    B  */

/* multipliers from c in quarter comma meantone */
static float S_tuning_mult_qc_meantone[12] = 
        { 1.0,                  /* 1:1        C  */
          1.04490672652566,     /* 5^(7/4)/16 C# */
          1.1180339887499,      /* 5^(1/2):2  D  */
          1.19627902497696,     /* 4:5^(3/4)  Eb */
          1.25,                 /* 5:4        E  */
          1.33748060995284,     /* 2:5^(1/4)  F  */
          1.39754248593737,     /* 5^(3/2):8  F# */
          1.495348781221221,    /* 5^(1/4):1  G  */
          1.5625,               /* 25:16      G# */
          1.67185076244106,     /* 5^(3/4):2  A  */
          1.78885438199984,     /* 4:5^(1/2)  Bb */
          1.86918597652653};    /* 5^(5/4):4  B  */

/* multipliers from c in just intonation */
static float S_tuning_mult_just_intonation[12] = 
        { 1.0,                  /* 1:1        C  */
          1.06666666666667,     /* 16:15      C# */
          1.125,                /* 9:8        D  */
          1.2,                  /* 6:5        Eb */
          1.25,                 /* 5:4        E  */
          1.33333333333333,     /* 4:3        F  */
          1.40625,              /* 45:32      F# */
          1.5,                  /* 3:2        G  */
          1.6,                  /* 8:5        G# */
          1.66666666666667,     /* 5:3        A  */
          1.77777777777778,     /* 16:9       Bb */
          1.875};               /* 15:8       B  */

/* multipliers from c in werckmeister iii */
static float S_tuning_mult_werckmeister_iii[12] = 
        { 1.0,                  /* 1:1              C  */
          1.05349794238683,     /* 256:243          C# */
          1.1174033085417,      /* 64*2^(1/2):81    D  */
          1.18518518518519,     /* 32:27            D# */
          1.25282724872715,     /* 256*2^(1/4):243  E  */
          1.33333333333333,     /* 4:3              F  */
          1.40466392318244,     /* 1024:729         F# */
          1.49492696045105,     /* 8^(5/4):9        G  */
          1.58024691358025,     /* 128:81           G# */
          1.6704363316362,      /* 1024*2^(1/4):729 A  */
          1.77777777777778,     /* 16:9             Bb */
          1.87924087309072};    /* 128*2^(1/4):81   B  */

/* multipliers from c in werckmeister iv */
static float S_tuning_mult_werckmeister_iv[12] = 
        { 1.0,                  /* 1:1                  C  */
          1.04875001176028,     /* 16384*2^(1/3):19683  C# */
          1.11992982212878,     /* 8*2^(1/3):9          D  */
          1.18518518518519,     /* 32:27                D# */
          1.25424280649339,     /* 64*4^(1/3):81        E  */
          1.33333333333333,     /* 4:3                  F  */
          1.40466392318244,     /* 1024:729             F# */
          1.49323976283837,     /* 32*2^(1/3):27        G  */
          1.57312501764042,     /* 8192*2^(1/3):6561    G# */
          1.67232374199119,     /* 256*4^(1/3):243      A  */
          1.78582618346427,     /* 9:4*2^(1/3)          Bb */
          1.87288523090992};    /* 4096:2187            B  */

/* multipliers from c in werckmeister v */
static float S_tuning_mult_werckmeister_v[12] = 
        { 1.0,                  /* 1:1            C  */
          1.05707299111353,     /* 8*2^(1/4):9    C# */
          1.125,                /* 9:8            D  */
          1.189207115002721,    /* 2^(1/4):1      D# */
          1.25707872210942,     /* 8*2^(1/2):9    E  */
          1.33785800437806,     /* 9*2^(1/4):8    F  */
          1.414213562373095,    /* 2^(1/2):1      F# */
          1.5,                  /* 3:2            G  */
          1.58024691358025,     /* 128:81         G# */
          1.681792830507429,    /* 8^(1/4):1      A  */
          1.78381067250408,     /* 3:8^(1/4)      Bb */
          1.88561808316413};    /* 4*2^(1/2):3    B  */

/* multipliers from c in werckmeister vi */
static float S_tuning_mult_werckmeister_vi[12] = 
        { 1.0,                  /* 1:1      C  */
          1.05376344086022,     /* 98:93    C# */
          1.12,                 /* 28:25    D  */
          1.18787878787879,     /* 196:165  D# */
          1.25641025641026,     /* 49:39    E  */
          1.33333333333333,     /* 4:3      F  */
          1.41007194244604,     /* 196:139  F# */
          1.49618320610687,     /* 196:131  G  */
          1.58064516129032,     /* 49:31    G# */
          1.67521367521368,     /* 196:117  A  */
          1.78181818181818,     /* 98:55    Bb */
          1.88461538461538};    /* 49:26    B  */

/* multipliers from c in renold i tuning */
static float S_tuning_mult_renold_i[12] = 
        { 1.0,                  /* 1:1            C  */
          1.06066017177982,     /* 3*2^(1/2):4    C# */
          1.125,                /* 9:8            D  */
          1.1932426932523,      /* 27*2^(1/2):32  D# */
          1.265625,             /* 81:64          E  */
          1.33333333333333,     /* 4:3            F  */
          1.414213562373095,    /* 2^(1/2):1      F# */
          1.5,                  /* 3:2            G  */
          1.59099025766973,     /* 9*2^(1/2):8    G# */
          1.6875,               /* 27:16          A  */
          1.78986403987845,     /* 81*2^(1/2):64  A# */
          1.8984375};           /* 243:128        B  */

/*******************************************************************************
** tuning_setup()
*******************************************************************************/
short int tuning_setup()
{
  int i;

  float*  mult_table;

  /* determine multiplier table */
  if (G_tuning_system == TUNING_SYSTEM_12_ET)
    mult_table = S_tuning_mult_12_et;
  else if (G_tuning_system == TUNING_SYSTEM_PYTHAGOREAN)
    mult_table = S_tuning_mult_pythagorean;
  else if (G_tuning_system == TUNING_SYSTEM_QC_MEANTONE)
    mult_table = S_tuning_mult_qc_meantone;
  else if (G_tuning_system == TUNING_SYSTEM_JUST)
    mult_table = S_tuning_mult_just_intonation;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_III)
    mult_table = S_tuning_mult_werckmeister_iii;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_IV)
    mult_table = S_tuning_mult_werckmeister_iv;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_V)
    mult_table = S_tuning_mult_werckmeister_v;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_VI)
    mult_table = S_tuning_mult_werckmeister_vi;
  else if (G_tuning_system == TUNING_SYSTEM_RENOLD_I)
    mult_table = S_tuning_mult_renold_i;
  else
    mult_table = S_tuning_mult_12_et;

  /* compute phase increments / noise periods at tuning fork */
  if (G_tuning_fork == TUNING_FORK_C256)
  {
    S_opl2_tuning_table[0] = 
      (int) ((256 * ADLIB_1HZ_PHASE_INCREMENT) + 0.5);
  }
  else if (G_tuning_fork == TUNING_FORK_A440)
  {
    S_opl2_tuning_table[9] = 
      (int) ((440 * ADLIB_1HZ_PHASE_INCREMENT) + 0.5);
  }
  else if (G_tuning_fork == TUNING_FORK_A432)
  {
    S_opl2_tuning_table[9] = 
      (int) ((432 * ADLIB_1HZ_PHASE_INCREMENT) + 0.5);
  }
  else if (G_tuning_fork == TUNING_FORK_AMIGA)
  {
    S_opl2_tuning_table[0] = 
      (int) ((261.34375f * ADLIB_1HZ_PHASE_INCREMENT) + 0.5);
  }
  else
    return 1;

  /* compute phase increments / noise periods based on tuning system */
  if ((G_tuning_fork == TUNING_FORK_C256) ||
      (G_tuning_fork == TUNING_FORK_AMIGA))
  {
    for (i = 1; i < 12; i++)
    {
      S_opl2_tuning_table[i] = 
        (int) ((S_opl2_tuning_table[0] * mult_table[i]) + 0.5);
    }
  }
  else if ( (G_tuning_fork == TUNING_FORK_A440) ||
            (G_tuning_fork == TUNING_FORK_A432))
  {
    for (i = 0; i < 12; i++)
    {
      if (i == 9)
        continue;

      S_opl2_tuning_table[i] = (int) 
        ((S_opl2_tuning_table[9] * mult_table[i] / mult_table[9]) + 0.5);
    }
  }

  /* compute detune coarse amounts */
  for (i = 0; i < 11; i++)
  {
    S_detune_coarse_table[i][0] = 
      ((int) (((1 << (8 + i)) * TUNING_PI_OVER_TWO) + 0.5)) - (1 << (8 + i));
    S_detune_coarse_table[i][1] = 
      ((int) (((1 << (8 + i)) * TUNING_SQRT_TWO) + 0.5)) - (1 << (8 + i));
    S_detune_coarse_table[i][2] = 
      ((int) (((1 << (8 + i)) * TUNING_SQRT_THREE) + 0.5)) - (1 << (8 + i));
  }

  return 0;
}

/*******************************************************************************
** tuning_compute_phase_increment()
*******************************************************************************/
int tuning_compute_phase_increment(char note)
{
  int row;
  int octave;
  int increment;

  /* if note is out of the range A0 - C8, ignore */
  if ((note < 21) || (note > 108))
    return 0;

  /* compute octave and array indices (note 60 is C4) */
  octave = (note / 12) - 1;
  row = note % 12;

  /* lookup phase increment */
  increment = S_opl2_tuning_table[row];

  /* return phase increment, shifted based on the octave */
  if (octave == 0)
    return increment >> 4;
  else if (octave == 1)
    return increment >> 3;
  else if (octave == 2)
    return increment >> 2;
  else if (octave == 3)
    return increment >> 1;
  else if (octave == 4)
    return increment;
  else if (octave == 5)
    return increment << 1;
  else if (octave == 6)
    return increment << 2;
  else if (octave == 7)
    return increment << 3;
  else if (octave == 8)
    return increment << 4;
  else
    return 0;
}

/*******************************************************************************
** tuning_compute_detune_coarse()
*******************************************************************************/
int tuning_compute_detune_coarse(char block, int fnum, unsigned char dt2)
{
  int column;
  int detune_amount;

  if (dt2 == 0)
    return 0;

  if ((block < 0) || (block > 7))
    return 0;

  if ((fnum < 0) || (fnum > 2047))
    return 0;

  /* select detune table column */
  if ((dt2 >= 1) && (dt2 <= 3))
    column = dt2 - 1;
  else
    return 0;

  /* compute detune amount */
  detune_amount = 0;

  if (fnum & 0x0400)
    detune_amount += S_detune_coarse_table[10][column];
  if (fnum & 0x0200)
    detune_amount += S_detune_coarse_table[9][column];
  if (fnum & 0x0100)
    detune_amount += S_detune_coarse_table[8][column];
  if (fnum & 0x0080)
    detune_amount += S_detune_coarse_table[7][column];
  if (fnum & 0x0040)
    detune_amount += S_detune_coarse_table[6][column];
  if (fnum & 0x0020)
    detune_amount += S_detune_coarse_table[5][column];
  if (fnum & 0x0010)
    detune_amount += S_detune_coarse_table[4][column];
  if (fnum & 0x0008)
    detune_amount += S_detune_coarse_table[3][column];
  if (fnum & 0x0004)
    detune_amount += S_detune_coarse_table[2][column];
  if (fnum & 0x0002)
    detune_amount += S_detune_coarse_table[1][column];
  if (fnum & 0x0001)
    detune_amount += S_detune_coarse_table[0][column];

  /* apply block adjustment */
  if (block == 0)
    detune_amount = detune_amount >> 1;
  else if (block > 1)
    detune_amount = detune_amount << (block - 1);

  return detune_amount;
}

