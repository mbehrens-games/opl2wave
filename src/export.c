/*******************************************************************************
** export.c (file export functions)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "export.h"
#include "global.h"

static FILE*          S_export_fp;

static unsigned int   S_chunk_size;
static unsigned int   S_subchunk1_size;
static unsigned int   S_subchunk2_size;

static unsigned int   S_byte_rate;
static unsigned short S_audio_format;
static unsigned short S_block_align;

static unsigned int   S_sampling_rate;
static unsigned short S_bits_per_sample;
static unsigned short S_num_channels;

/*******************************************************************************
** export_init()
*******************************************************************************/
short int export_init()
{
  S_export_fp = NULL;

  S_chunk_size = 0;
  S_subchunk1_size = 0;
  S_subchunk2_size = 0;

  S_byte_rate = 0;
  S_audio_format = 0;
  S_block_align = 0;

  S_sampling_rate = 0;
  S_bits_per_sample = 0;
  S_num_channels = 0;

  return 0;
}

/*******************************************************************************
** export_deinit()
*******************************************************************************/
short int export_deinit()
{
  /* close open file if necessary */
  if (S_export_fp != NULL)
    export_close_file();

  S_chunk_size = 0;
  S_subchunk1_size = 0;
  S_subchunk2_size = 0;

  S_byte_rate = 0;
  S_audio_format = 0;
  S_block_align = 0;

  S_sampling_rate = 0;
  S_bits_per_sample = 0;
  S_num_channels = 0;

  return 0;
}

/*******************************************************************************
** export_open_file()
*******************************************************************************/
short int export_open_file(char* filename)
{
  /* close file if one is currently open */
  if (S_export_fp != NULL)
    export_close_file();

  /* open file */
  S_export_fp = fopen(filename, "wb");

  /* if file did not open, return error */
  if (S_export_fp == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** export_close_file()
*******************************************************************************/
short int export_close_file()
{
  if (S_export_fp != NULL)
  {
    fclose(S_export_fp);
    S_export_fp = NULL;
  }

  return 0;
}

/*******************************************************************************
** export_write_header()
*******************************************************************************/
short int export_write_header(int num_samples)
{
  char id_field[4];

  /* make sure that file pointer is present */
  if (S_export_fp == NULL)
    return 1;

  /* make sure number of samples is positive */
  if (num_samples <= 0)
    return 1;

  /* set sampling rate and bits per sample */
  S_sampling_rate = G_export_sampling;
  S_bits_per_sample = G_export_bitres;

  /* set number of channels (mono) */
  S_num_channels = 1;

  /* compute subchunk sizes and other derived field values */
  S_audio_format = 1; /* 1 denotes PCM */
  S_block_align = S_num_channels * (S_bits_per_sample / 8);
  S_byte_rate = S_sampling_rate * S_block_align;

  S_subchunk1_size = 16; /* always 16 for PCM data */
  S_subchunk2_size = num_samples * S_block_align;
  S_chunk_size = 4 + (8 + S_subchunk1_size) + (8 + S_subchunk2_size);

  /* write 'RIFF' chunk */
  id_field[0] = 'R';
  id_field[1] = 'I';
  id_field[2] = 'F';
  id_field[3] = 'F';
  fwrite(id_field, 1, 4, S_export_fp);

  fwrite(&S_chunk_size, 4, 1, S_export_fp);

  id_field[0] = 'W';
  id_field[1] = 'A';
  id_field[2] = 'V';
  id_field[3] = 'E';
  fwrite(id_field, 1, 4, S_export_fp);

  /* write 'fmt ' chunk */
  id_field[0] = 'f';
  id_field[1] = 'm';
  id_field[2] = 't';
  id_field[3] = ' ';
  fwrite(id_field, 1, 4, S_export_fp);

  fwrite(&S_subchunk1_size, 4, 1, S_export_fp);
  fwrite(&S_audio_format, 2, 1, S_export_fp);
  fwrite(&S_num_channels, 2, 1, S_export_fp);
  fwrite(&S_sampling_rate, 4, 1, S_export_fp);
  fwrite(&S_byte_rate, 4, 1, S_export_fp);
  fwrite(&S_block_align, 2, 1, S_export_fp);
  fwrite(&S_bits_per_sample, 2, 1, S_export_fp);

  /* write 'data' chunk */
  id_field[0] = 'd';
  id_field[1] = 'a';
  id_field[2] = 't';
  id_field[3] = 'a';
  fwrite(id_field, 1, 4, S_export_fp);

  fwrite(&S_subchunk2_size, 4, 1, S_export_fp);

  return 0;
}

/*******************************************************************************
** export_write_block()
*******************************************************************************/
short int export_write_block(short int* buffer, int num_samples)
{
  int           i;
  unsigned char temp_char;

  /* make sure that file pointer is present */
  if (S_export_fp == NULL)
    return 1;

  /* make sure number of samples is positive */
  if (num_samples <= 0)
    return 1;

  /* make sure position is the end of the file */
  fseek(S_export_fp, 0, SEEK_END);

  /* write 8-bit mono data */
  if (G_export_bitres == 8)
  {
    for (i = 0; i < num_samples; i++)
    {
      temp_char = 127 - (buffer[i] / 256);
      fwrite(&temp_char, 1, 1, S_export_fp);
    }
  }
  /* write 16-bit mono data */
  else if (G_export_bitres == 16)
  {
    fwrite(buffer, 2, num_samples, S_export_fp);
  }
  /* otherwise, return error */
  else
    return 1;

  return 0;
}

