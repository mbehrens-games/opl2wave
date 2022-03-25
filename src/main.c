/*******************************************************************************
** OPL2WAVE - Michael Behrens 2020
*******************************************************************************/

/*******************************************************************************
** main.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "channel.h"
#include "clock.h"
#include "datatree.h"
#include "downsamp.h"
#include "export.h"
#include "global.h"
#include "parse.h"
#include "tuning.h"

/*******************************************************************************
** main()
*******************************************************************************/
int main(int argc, char *argv[])
{
  int   i;

  char* name;
  char  input_filename[256];
  char  output_filename[256];

  data_tree_node* root;

  int sample_index;

  int release_time;

  short int* sample_buffer;
  short int* export_buffer;

  int sample_buffer_size;
  int export_buffer_size;

  /* initialization */
  i = 0;

  name = NULL;
  root = NULL;

  sample_index = 0;

  release_time = 0;

  sample_buffer = NULL;
  export_buffer = NULL;

  /* read command line arguments */
  i = 1;

  while (i < argc)
  {
    /* name */
    if (!strcmp(argv[i], "-n"))
    {
      i++;
      if (i >= argc)
      {
        printf("Insufficient number of arguments. ");
        printf("Expected name. Exiting...\n");
        return 0;
      }

      name = strdup(argv[i]);
      i++;
    }
    else
    {
      printf("Unknown command line argument %s. Exiting...\n", argv[i]);
      return 0;
    }
  }

  /* make sure name is defined */
  if (name == NULL)
  {
    printf("Name not defined. Exiting...\n");
    return 0;
  }

  /* determine input and output filenames */
  strncpy(input_filename, name, 252);
  strncat(input_filename, ".txt", 4);

  strncpy(output_filename, name, 252);
  strncat(output_filename, ".wav", 4);

  /* setup */
  globals_init();
  export_init();

  /* read input file */
  root = parse_file_to_data_tree(input_filename);

  if (root == NULL)
  {
    printf("Data tree not created from input file. Exiting...\n");
    goto cleanup;
  }

  parse_data_tree_to_globals(root);
  data_tree_node_destroy_tree(root);
  root = NULL;

  /* initialize tables */
  tuning_setup();
  lfo_generate_tables();
  globals_compute_sinc_filter();

  /* determine buffer sizes */
  sample_buffer_size = (int) (G_export_length * ADLIB_CLOCK);
  export_buffer_size = (int) (G_export_length * G_export_sampling);

  /* allocate buffers */
  sample_buffer = malloc(sizeof(short int) * sample_buffer_size);

  if (sample_buffer == NULL)
    goto cleanup;

  export_buffer = malloc(sizeof(short int) * export_buffer_size);

  if (export_buffer == NULL)
    goto cleanup;

  /* determine release (key-off) time */
  release_time = (int) (G_note_length * ADLIB_CLOCK);

  /* setup channel */
  channel_setup(&G_channel);
  channel_key_on(&G_channel);

  /* sound generation start */
  sample_index = 0;

  while (sample_index < sample_buffer_size)
  {
    /* check for key off command */
    if (sample_index >= release_time)
      channel_key_off(&G_channel);

    /* update channel */
    if (!channel_is_inactive(&G_channel))
      channel_update(&G_channel);

    /* add sample to buffer */
    sample_buffer[sample_index] = G_channel.level;
    sample_index += 1;
  }

  /* apply 2nd order lowpass */
  if (G_filter.cutoff != 0)
  {
    filter_setup(&G_filter);

    for (i = 0; i < sample_buffer_size; i++)
    {
      filter_update(&G_filter, sample_buffer[i]);
      sample_buffer[i] = G_filter.y[0];
    }
  }

  /* downsample */
  if (G_export_sampling != 49716)
  {
    downsamp_apply_filter(sample_buffer, sample_buffer_size);

    downsamp_perform_downsample(sample_buffer, sample_buffer_size, 
                                export_buffer, export_buffer_size);
  }
  else
  {
    memcpy(export_buffer, sample_buffer, export_buffer_size);
  }

  /* open output file */
  if (export_open_file(output_filename))
  {
    fprintf(stdout, "Output file not opened. Exiting...\n");
    goto cleanup;
  }

  /* write to file */
  export_write_header(export_buffer_size);
  export_write_block(export_buffer, export_buffer_size);

  /* close output file */
  export_close_file();

  /* cleanup */
cleanup:
  if (sample_buffer != NULL)
  {
    free(sample_buffer);
    sample_buffer = NULL;
  }

  if (export_buffer != NULL)
  {
    free(export_buffer);
    export_buffer = NULL;
  }

  export_deinit();
  globals_deinit();

  return 0;
}
