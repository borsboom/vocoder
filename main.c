/******************************************************************************
 * $Id: main.c,v 1.6 1999/03/16 06:55:59 emanuel Exp $
 * Copyright (C) 1996-1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include "config.h"
#include "error.h"
#include "wave.h"
#include "fft.h"

typedef REAL (*COMPLEX_ARRAY)[2];

#define DEFAULT_WINDOW_TIME 15 /* 1/15th of a second */

static void vocode_window(REAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output);
static void vocoder(void);
static void loop(WAVE_FILE *source, SAMPLE *dest, size_t length);
static size_t read_zero(WAVE_FILE *source, SAMPLE *dest, size_t length);
static void sample_to_complex_array(SAMPLE *sample_array,
                                    COMPLEX_ARRAY complex_array,
                                    size_t length, SAMPLE max_magnitude);
static void sample_to_real_array(SAMPLE *sample_array, REAL *real_array,
                                 size_t length, SAMPLE max_magnitude);
static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array, size_t length,
                                    SAMPLE max_magnitude, REAL volume);
static void parse_args(int argc, char *argv[]);
static void open_files(void);
static void close_files(void);
static void allocate_memory(void);
static void free_memory(void);

char *prog_name;

static char *copyright = 
"Zerius Vocoder 1.2 Copyright (C) 1996-1999 Emanuel Borsboom\n"
"email: emanuel@zerius.com / web: http://zerius.com/vocoder/\n";

static char *modulator_filename, *carrier_filename, *output_filename;
static BOOL got_window_length, got_window_overlap;
static size_t window_length, window_overlap;
static int band_count;
static REAL volume;
static BOOL quiet;
static BOOL normalize;

static WAVE_FILE *modulator_file, *carrier_file, *output_file;
static SAMPLE modulator_max_magnitude, carrier_max_magnitude,
              output_max_magnitude;
static INT modulator_length, modulator_rate;

static SAMPLE *modulator_sample_buffer, *carrier_sample_buffer,
              *output_sample_buffer1, *output_sample_buffer2;
static REAL *modulator;
static COMPLEX_ARRAY looped_carrier, output;

static double *fft_c, *fft_s;
static int *fft_rev;

static void vocode_window(REAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output)
{
  int band_no, band_length, extra_band_length;

  band_length = window_length / (band_count * 2);
  extra_band_length = window_length / 2 - band_length * (band_count - 1);

  realfftmag(modulator, window_length);
  fft(carrier, window_length, fft_c, fft_s, fft_rev);
  normalize_fft(carrier, window_length);
  
  for (band_no = 0; band_no < band_count; band_no++) {
    int i, j, k, l;
    REAL m, c;
    
    l = (band_no == band_count - 1) ? extra_band_length : band_length;

    m = 0; c = 0;
    for (i = 0, j = band_no * band_length, k = window_length - j - 1;
         i < l; i++, j++, k--)
      {
        if (normalize) {
          REAL c1 = carrier[j][0]*carrier[j][0] + carrier[j][1]*carrier[j][1],
               c2 = carrier[k][0]*carrier[k][0] + carrier[k][1]*carrier[k][1];
          c += sqrt(c1) + sqrt(c2);
        }
        m += modulator[j];
      }

    if (!normalize) c = 1.0;
    if (c == 0) c = 0.0001;

    for (i = 0, j = band_no * band_length, k = window_length - j - 1;
         i < l; i++, j++, k--) {
      output[j][0] = carrier[j][0] * m / c;
      output[j][1] = carrier[j][1] * m / c;
      output[k][0] = carrier[k][0] * m / c;
      output[k][1] = carrier[k][1] * m / c;
    }
  }

  invfft (output, window_length, fft_c, fft_s, fft_rev);
}

static void start_status(void) 
{
}

static int update_status(INT frame_no, INT num_frames) {
  int i, count = (frame_no + 1) * 56 / num_frames;
  if (quiet) return 0;
  printf("\r%3d%% |", (int)((frame_no + 1) * 100 / num_frames));
  for (i = 0; i < count; ++i)
    putchar('*');
  for (; i < 56; ++i)
    putchar(' ');
  printf("| %ld/%ld", frame_no + 1, num_frames);
  fflush(stdout);
  return 0;	/* Return true to quit */
}

static void finish_status(void) {
  if (!quiet)
    printf("\n");
}

static void vocoder(void)
{
  int i;
  SAMPLE *output_old = output_sample_buffer1,
         *output_new = output_sample_buffer2, *output_temp;
  INT num_frames, frame_no;

  num_frames = (modulator_length - window_overlap) /
               (window_length - window_overlap);
  frame_no = 0;

  read_zero(modulator_file, modulator_sample_buffer, window_length);
  loop(carrier_file, carrier_sample_buffer, window_length);
  
  sample_to_real_array(modulator_sample_buffer, modulator, window_length,
                       modulator_max_magnitude);
  sample_to_complex_array(carrier_sample_buffer, looped_carrier,
                          window_length, carrier_max_magnitude);
  
  vocode_window(modulator, looped_carrier, output);
  
  complex_to_sample_array(output, output_old, window_length,
                          output_max_magnitude, volume);
  wave_write(output_file, output_old, window_length - window_overlap);

  for (i = 0; i < window_overlap; ++i)
    {
      modulator_sample_buffer[i] =
        modulator_sample_buffer[window_length - window_overlap + i];
      carrier_sample_buffer[i] =
        carrier_sample_buffer[window_length - window_overlap + i];
    }

  start_status();
  
  while (read_zero(modulator_file, modulator_sample_buffer +
                   window_overlap, window_length - window_overlap))
    {
      if (update_status(frame_no, num_frames)) break;
      
      loop(carrier_file, carrier_sample_buffer + window_overlap,
           window_length - window_overlap);
      
      sample_to_real_array(modulator_sample_buffer, modulator, window_length,
                           modulator_max_magnitude);
      sample_to_complex_array(carrier_sample_buffer, looped_carrier,
                              window_length, carrier_max_magnitude);

      vocode_window(modulator, looped_carrier, output);

      complex_to_sample_array(output, output_new, window_length,
                              output_max_magnitude, volume);

      for (i = 0; i < window_overlap; ++i)
        { 
          output_new[i] = (output_new[i] * (i / (double)window_overlap)) +
                          (output_old[window_length - window_overlap + i] *
                           ((window_overlap - i) / (double)window_overlap));
        }

      wave_write(output_file, output_new, window_length - window_overlap);

      for (i = 0; i < window_overlap; ++i)
        {
          modulator_sample_buffer[i] =
            modulator_sample_buffer[window_length - window_overlap + i];
          carrier_sample_buffer[i] =
            carrier_sample_buffer[window_length - window_overlap + i];
        }

      output_temp = output_new;
      output_new = output_old;
      output_old = output_temp;

      ++frame_no;
    }

  wave_write(output_file, output_old + window_length - window_overlap,
             window_overlap);

  finish_status();
}

static void loop(WAVE_FILE *source, SAMPLE *dest, size_t length)
{
  while (length > 0)
    {
      size_t n;

      n = wave_read(source, dest, length);
      if (n < length) 
        wave_seek(source, 0);

      dest += n;
      length -= n;
    }
}

static size_t read_zero(WAVE_FILE *source, SAMPLE *dest, size_t length)
{
  size_t i, n = wave_read(source, dest, length);
  for (i = n; i < length; ++i)
    dest[i] = 0;
  return n;
}

static void sample_to_complex_array(SAMPLE *sample_array,
                                    COMPLEX_ARRAY complex_array,
                                    size_t length, SAMPLE max_magnitude)
{
  int i;
  for (i = 0; i < length; ++i)
    {
      complex_array[i][0] = sample_array[i] / (REAL)max_magnitude;
      complex_array[i][1] = 0;
    }
}

static void sample_to_real_array(SAMPLE *sample_array, REAL *real_array,
                                 size_t length, SAMPLE max_magnitude)
{
  int i;
  for (i = 0; i < length; ++i)
    {
      *real_array++ = *sample_array++ / (REAL)max_magnitude;
    }
}

static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array,
                                    size_t length, SAMPLE max_magnitude,
                                    REAL volume)
{
  int i;
  for (i = 0; i < length; ++i)
    {
      REAL sample = complex_array[i][0] * volume;
      if (sample < -1.0) sample = -1.0;
      else if (sample > 1.0) sample = 1.0;
      sample_array[i] = sample * max_magnitude;
    }
}

static void usage()
{
  printf("%s\n", copyright);
  printf("usage: %s [-q] [-N] [-b <band-count>] [-w <window-length] "
         "[-o <window-overlap>] [-v <volume>] "
         "<modulator-file> <carrier-file> <output-file>\n", prog_name);
  exit(1);
}

static void prompt_user(const char *prompt, char *buffer, size_t length)
{
  printf("%s? ", prompt);
  fgets(buffer, length, stdin);
}

static void message_user(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
}

static void alert_user(const char *fmt, ...)
{
  va_list ap;
  printf("! ");
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
}

static void ask_user(const char *prompt, char *buffer, size_t length,
		     BOOL require_non_empty, BOOL require_number)
{
  char *p;
  BOOL okay;
  do
    {
      prompt_user(prompt, buffer, length);
      okay = TRUE;
      if (require_non_empty)
        {
	  okay = FALSE;
	  p = buffer;
	  while (*p && *p != '\n') 
	    if (!isspace(*p++)) 
	      {
	        okay = TRUE;
	        break;
	      }
	  if (!okay) alert_user("You must enter a value.");
	}
      if (require_number) 
        {
          p = buffer;
	  while (*p && *p != '\n') 
	    {
	      if (!isdigit(*p) && *p != '.') 
	        {
	          alert_user("You may only enter digits.");
		  okay = FALSE;
		  break;
	        }
	      p++;
	    }
        }
    }
  while (!okay);
  p = strchr(buffer, '\n');
  if (p) *p = '\0';
}

static void ask_user_filename(const char *prompt, char *buffer, size_t length,
                              BOOL require_existance)
{
  ask_user(prompt, buffer, length, TRUE, FALSE);
  if (require_existance)
    {
      FILE *fp;
      fp = fopen(buffer, "rb");
      while (fp == NULL)
        {
	  alert_user("Cannot open %s: %s", buffer, strerror(errno));
          ask_user(prompt, buffer, length, TRUE, FALSE);
          fp = fopen(buffer, "rb");
	}
      fclose(fp);
    }
}

static void parse_args(int argc, char *argv[])
{
  int i;

  prog_name = argv[0];

  got_window_length = FALSE;
  got_window_overlap = FALSE;
  quiet = FALSE;
  normalize = TRUE;
  volume = 1.0;
  band_count = 16;

  if (argc <= 1) 
    {
      char buf[16];
      modulator_filename = error_malloc(PATH_MAX);
      carrier_filename = error_malloc(PATH_MAX);
      output_filename = error_malloc(PATH_MAX);
      
      ask_user_filename("Modulator filename (required)", modulator_filename, 
                        PATH_MAX, TRUE);

      ask_user_filename("Carrier filename (required)", carrier_filename, 
                        PATH_MAX, TRUE);

      ask_user("Window length (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0])  
        {
          window_length = atoi(buf);
	  got_window_length = TRUE;
        }

      ask_user("Window overlap (empty for default)", buf, sizeof(buf), 
	       FALSE, TRUE);
      if (buf[0])  
        {
          window_overlap = atoi(buf);
	  got_window_overlap = TRUE;
        }

      ask_user("Band count (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0]) band_count = atoi(buf);

      ask_user("Output volume (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0]) volume = atof(buf);
      ask_user_filename("Output filename (required)", output_filename, 
                        PATH_MAX, FALSE);
    } 
  else if (argc == 7 &&
      argv[1][0] != '-' && argv[2][0] != '-' &&
      atoi(argv[3]) != 0 && atoi(argv[4]) != 0 && atoi(argv[5]) != 0 &&
      argv[6][0] != '-')
    {
      modulator_filename = argv[1];
      carrier_filename = argv[2];
      window_length = atoi(argv[3]);
      got_window_length = TRUE;
      window_overlap = atoi(argv[4]);
      got_window_overlap = TRUE;
      band_count = atoi(argv[5]);
      output_filename = argv[6];
    }
  else
    {
      for (i = 1; i < argc && argv[i][0] == '-'; ++i)
        {
          if (argv[i][1] == 'v' && i < argc - 1)
            volume = atof(argv[++i]);
          else if (argv[i][1] == 'b' && i < argc - 1)
            band_count = atoi(argv[++i]);
          else if (argv[i][1] == 'q')
            quiet = TRUE;
          else if (argv[i][1] == 'N')
            normalize = FALSE;
          else if (argv[i][1] == 'w' && i < argc - 1) {
            window_length = atoi(argv[++i]);
            got_window_length = TRUE;
          }
          else if (argv[i][1] == 'o' && i < argc - 1) {
            window_overlap = atoi(argv[++i]);
            got_window_overlap = TRUE;
          }
          else
            usage();
        }
      
      if (argc != i + 3)
        usage();
      
      modulator_filename = argv[i++];
      carrier_filename = argv[i++];
      output_filename = argv[i++];
    }
}

static void open_files(void)
{
  WAVE_INFO wave_info;
  carrier_file = wave_open(carrier_filename, &wave_info);
  if (wave_info.channels != 1)
    error("carrier must be mono (1 channel)");
  carrier_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
  
  modulator_file = wave_open(modulator_filename, &wave_info);
  if (wave_info.channels != 1)
    error("modulator must be mono (1 channel)");
  modulator_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
  modulator_length = wave_info.length;
  modulator_rate = wave_info.rate;
  
  output_file = wave_create(output_filename, &wave_info);
  output_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
}

static void check_args(void)
{
  if (!got_window_length)
    window_length = ipow(2, ilog2(modulator_rate / DEFAULT_WINDOW_TIME));
  if (!got_window_overlap)
    window_overlap = window_length / 2;
  
  if (window_length < 2 || ipow(2, ilog2(window_length)) != window_length)
    error("window-length must be > 1 and a power of two\n"
          "(the closest power of two to the number you entered is %d)",
          ipow(2, ilog2(window_length)));
  if (window_overlap < 0 || window_overlap > window_length / 2)
    error("window-overlap must be >= 0 and <= window-length/2");
  if (band_count < 1 || band_count > window_length / 2)
    error("band-count must be > 0 and <= window-length/2");

  if (!quiet)
    message_user("%s\nwindow-length: %d   window-overlap: %d   band-count: %d   "
                 "volume: %.2f", copyright,
                 window_length, window_overlap, band_count, volume);
}

static void close_files(void)
{
  wave_close(output_file);
  wave_close(modulator_file);
  wave_close(carrier_file);
}

static void allocate_memory(void)
{
  modulator_sample_buffer = error_malloc(sizeof(SAMPLE) * window_length);
  carrier_sample_buffer = error_malloc(sizeof(SAMPLE) * window_length);
  output_sample_buffer1 = error_malloc(sizeof(SAMPLE) * window_length);
  output_sample_buffer2 = error_malloc(sizeof(SAMPLE) * window_length);
  modulator = error_malloc(sizeof(REAL) * window_length);
  looped_carrier = error_malloc(sizeof(REAL) * 2 * window_length);
  output = error_malloc(sizeof(REAL) * 2 * window_length);
}

static void free_memory(void)
{
  free(output);
  free(looped_carrier);
  free(modulator);
  free(output_sample_buffer1);
  free(output_sample_buffer2);
  free(carrier_sample_buffer);
  free(modulator_sample_buffer);
}

int main(int argc, char *argv[])
{
  parse_args(argc, argv);
  open_files();
  check_args();

  allocate_memory();
  fft_create_arrays (&fft_c, &fft_s, &fft_rev, window_length);
  vocoder();
  
  free_memory();
  close_files();
  return 0;
}
