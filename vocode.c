/******************************************************************************
 * $Id: vocode.c,v 1.2 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1996-1999,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "config.h"
#include "error.h"
#include "wave.h"
#include "fft.h"
#include "vocode.h"

typedef VREAL (*COMPLEX_ARRAY)[2];

static void vocode_window(VREAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output);
static void vocoder(void);
static void loop(WAVE_FILE *source, SAMPLE *dest, size_t length);
static size_t read_zero(WAVE_FILE *source, SAMPLE *dest, size_t length);
static void sample_to_complex_array(SAMPLE *sample_array,
                                    COMPLEX_ARRAY complex_array,
                                    size_t length, SAMPLE max_magnitude);
static void sample_to_real_array(SAMPLE *sample_array, VREAL *real_array,
                                 size_t length, SAMPLE max_magnitude);
static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array, size_t length,
                                    SAMPLE max_magnitude, VREAL vocode_volume);
static void allocate_memory(void);
static void free_memory(void);

char *vocode_modulator_filename, *vocode_carrier_filename, *vocode_output_filename;
size_t vocode_window_length, vocode_window_overlap;
int vocode_band_count;
VREAL vocode_volume;
VBOOL vocode_normalize;

static WAVE_FILE *modulator_file = NULL, *carrier_file = NULL, *output_file = NULL;
static SAMPLE modulator_max_magnitude, carrier_max_magnitude,
              output_max_magnitude;
static VINT modulator_length;
VINT vocode_modulator_rate;

static SAMPLE *modulator_sample_buffer = NULL, *carrier_sample_buffer = NULL,
              *output_sample_buffer1 = NULL, *output_sample_buffer2 = NULL;
static VREAL *modulator = NULL;
static COMPLEX_ARRAY looped_carrier = NULL, output = NULL;

static double *fft_c, *fft_s;
static int *fft_rev;

void (*vocode_start_status_cb)(VINT num_frames);
VBOOL (*vocode_update_status_cb)(VINT frame_no);
void (*vocode_finish_status_cb)(void);

static void vocode_window(VREAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output)
{
  int band_no, band_length, extra_band_length;

  band_length = vocode_window_length / (vocode_band_count * 2);
  extra_band_length = vocode_window_length / 2 - band_length * (vocode_band_count - 1);

  realfftmag(modulator, vocode_window_length);
  fft(carrier, vocode_window_length, fft_c, fft_s, fft_rev);
  normalize_fft(carrier, vocode_window_length);
  
  for (band_no = 0; band_no < vocode_band_count; band_no++) {
    int i, j, k, l;
    VREAL m, c;
    
    l = (band_no == vocode_band_count - 1) ? extra_band_length : band_length;

    m = 0; c = 0;
    for (i = 0, j = band_no * band_length, k = vocode_window_length - j - 1;
         i < l; i++, j++, k--)
      {
        if (vocode_normalize) {
          VREAL c1 = carrier[j][0]*carrier[j][0] + carrier[j][1]*carrier[j][1],
               c2 = carrier[k][0]*carrier[k][0] + carrier[k][1]*carrier[k][1];
          c += sqrt(c1) + sqrt(c2);
        }
        m += modulator[j];
      }

    if (!vocode_normalize) c = 1.0;
    if (c == 0) c = 0.0001;

    for (i = 0, j = band_no * band_length, k = vocode_window_length - j - 1;
         i < l; i++, j++, k--) {
      output[j][0] = carrier[j][0] * m / c;
      output[j][1] = carrier[j][1] * m / c;
      output[k][0] = carrier[k][0] * m / c;
      output[k][1] = carrier[k][1] * m / c;
    }
  }

  invfft (output, vocode_window_length, fft_c, fft_s, fft_rev);
}

static void vocoder(void)
{
  size_t i;
  SAMPLE *output_old = output_sample_buffer1,
         *output_new = output_sample_buffer2, *output_temp;
  VINT num_frames, frame_no;

  num_frames = (modulator_length - vocode_window_overlap) /
               (vocode_window_length - vocode_window_overlap);
  frame_no = 0;

  read_zero(modulator_file, modulator_sample_buffer, vocode_window_length);
  loop(carrier_file, carrier_sample_buffer, vocode_window_length);
  
  sample_to_real_array(modulator_sample_buffer, modulator, vocode_window_length,
                       modulator_max_magnitude);
  sample_to_complex_array(carrier_sample_buffer, looped_carrier,
                          vocode_window_length, carrier_max_magnitude);
  
  vocode_window(modulator, looped_carrier, output);
  
  complex_to_sample_array(output, output_old, vocode_window_length,
                          output_max_magnitude, vocode_volume);
  wave_write(output_file, output_old, vocode_window_length - vocode_window_overlap);

  for (i = 0; i < vocode_window_overlap; ++i)
    {
      modulator_sample_buffer[i] =
        modulator_sample_buffer[vocode_window_length - vocode_window_overlap + i];
      carrier_sample_buffer[i] =
        carrier_sample_buffer[vocode_window_length - vocode_window_overlap + i];
    }

  vocode_start_status_cb(num_frames);
  
  while (read_zero(modulator_file, modulator_sample_buffer +
                   vocode_window_overlap, vocode_window_length - vocode_window_overlap))
    {
      if (vocode_update_status_cb(frame_no)) break;
      
      loop(carrier_file, carrier_sample_buffer + vocode_window_overlap,
           vocode_window_length - vocode_window_overlap);
      
      sample_to_real_array(modulator_sample_buffer, modulator, vocode_window_length,
                           modulator_max_magnitude);
      sample_to_complex_array(carrier_sample_buffer, looped_carrier,
                              vocode_window_length, carrier_max_magnitude);

      vocode_window(modulator, looped_carrier, output);

      complex_to_sample_array(output, output_new, vocode_window_length,
                              output_max_magnitude, vocode_volume);

      for (i = 0; i < vocode_window_overlap; ++i)
        { 
          output_new[i] = (SAMPLE)((output_new[i] * (i / (double)vocode_window_overlap)) +
                          (output_old[vocode_window_length - vocode_window_overlap + i] *
                           ((vocode_window_overlap - i) / (double)vocode_window_overlap)));
        }

      wave_write(output_file, output_new, vocode_window_length - vocode_window_overlap);

      for (i = 0; i < vocode_window_overlap; ++i)
        {
          modulator_sample_buffer[i] =
            modulator_sample_buffer[vocode_window_length - vocode_window_overlap + i];
          carrier_sample_buffer[i] =
            carrier_sample_buffer[vocode_window_length - vocode_window_overlap + i];
        }

      output_temp = output_new;
      output_new = output_old;
      output_old = output_temp;

      ++frame_no;
    }

  wave_write(output_file, output_old + vocode_window_length - vocode_window_overlap,
             vocode_window_overlap);

  vocode_update_status_cb(frame_no - 1);
  vocode_finish_status_cb();
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
  size_t i;
  for (i = 0; i < length; ++i)
    {
      complex_array[i][0] = sample_array[i] / (VREAL)max_magnitude;
      complex_array[i][1] = 0;
    }
}

static void sample_to_real_array(SAMPLE *sample_array, VREAL *real_array,
                                 size_t length, SAMPLE max_magnitude)
{
  size_t i;
  for (i = 0; i < length; ++i)
    {
      *real_array++ = *sample_array++ / (VREAL)max_magnitude;
    }
}

static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array,
                                    size_t length, SAMPLE max_magnitude,
                                    VREAL vocode_volume)
{
  size_t i;
  for (i = 0; i < length; ++i)
    {
      VREAL sample = complex_array[i][0] * vocode_volume;
      if (sample < -1.0) sample = -1.0;
      else if (sample > 1.0) sample = 1.0;
      sample_array[i] = (SAMPLE)(sample * max_magnitude);
    }
}

void vocode_open_files(void)
{
  WAVE_INFO wave_info;
  carrier_file = wave_open(vocode_carrier_filename, &wave_info);
  if (wave_info.channels != 1)
    error_display("carrier must be mono (1 channel)");
  carrier_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
  
  modulator_file = wave_open(vocode_modulator_filename, &wave_info);
  if (wave_info.channels != 1)
    error_display("modulator must be mono (1 channel)");
  modulator_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
  modulator_length = wave_info.length;
  vocode_modulator_rate = wave_info.rate;
  
  output_file = wave_create(vocode_output_filename, &wave_info);
  output_max_magnitude = (1 << (wave_info.bits - 1)) - 1;
}

static void wave_close_if_open(WAVE_FILE **pp)
{
  if (*pp != NULL)
  {
	wave_close(*pp);
	*pp = NULL;
  }
}

static void close_files(void)
{
  wave_close_if_open(&output_file);
  wave_close_if_open(&modulator_file);
  wave_close_if_open(&carrier_file);
}

static void allocate_memory(void)
{
  modulator_sample_buffer = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  carrier_sample_buffer = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  output_sample_buffer1 = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  output_sample_buffer2 = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  modulator = error_malloc(sizeof(VREAL) * vocode_window_length);
  looped_carrier = error_malloc(sizeof(VREAL) * 2 * vocode_window_length);
  output = error_malloc(sizeof(VREAL) * 2 * vocode_window_length);
}

static void free_if_not_null(void **pp)
{
  if (*pp != NULL)
  {
	free(*pp);
	*pp = NULL;
  }
}

static void free_memory(void)
{
  free_if_not_null((void **)&output);
  free_if_not_null((void **)&looped_carrier);
  free_if_not_null((void **)&modulator);
  free_if_not_null((void **)&output_sample_buffer1);
  free_if_not_null((void **)&output_sample_buffer2);
  free_if_not_null((void **)&carrier_sample_buffer);
  free_if_not_null((void **)&modulator_sample_buffer);
}

void vocode_cleanup()
{
	free_memory();
	close_files();
}

void vocode(void)
{
  allocate_memory();
  fft_create_arrays (&fft_c, &fft_s, &fft_rev, vocode_window_length);
  vocoder();
  vocode_cleanup();
}
