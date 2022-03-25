/*******************************************************************************
** export.h (file export functions)
*******************************************************************************/

#ifndef EXPORT_H
#define EXPORT_H

/* function declarations */
short int export_init();
short int export_deinit();

short int export_open_file(char* filename);
short int export_close_file();

short int export_write_header(int num_samples);
short int export_write_block(short int* buffer, int num_samples);

#endif
