/*******************************************************************************
** filter.h (filter)
*******************************************************************************/

#ifndef FILTER_H
#define FILTER_H

/* standard filter cutoffs: */
/* genesis: 2840            */
/* genesis m1 v2: 3390      */
/* soundblaster: 3200       */

typedef struct filter
{
  int       cutoff;
  float     q;

  float     b_0;
  float     b_1;
  float     b_2;
  float     a_1;
  float     a_2;

  short int x[3];
  short int y[3];
} filter;

/* function declarations */
short int   filter_init(filter* fltr);
filter*     filter_create();
short int   filter_deinit(filter* fltr);
short int   filter_destroy(filter* fltr);

short int   filter_setup(filter* fltr);
short int   filter_update(filter* fltr, short int input);

#endif
