/******************************************************************************
 * $Id: main.c,v 1.9 2002/09/20 02:24:14 emanuel Exp $
 * Copyright (C) 1996-1999,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include "error.h"
#include "vocode.h"
#include "fft.h"

#define COPYRIGHT \
"Zerius Vocoder 1.3 Copyright (C) 1996-1999, 2002 Emanuel Borsboom\n" \
"email: em@nuel.ca / web: http://www.nuel.ca/Vocoder\n"

#define DEFAULT_WINDOW_TIME 15 /* 1/15th of a second */

char *prog_name;

static VBOOL got_window_length, got_window_overlap;
static VBOOL quiet;

static void usage()
{
  printf("%s\n", COPYRIGHT);
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
		     VBOOL require_non_empty, VBOOL require_number)
{
  char *p;
  VBOOL okay;
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
                              VBOOL require_existance)
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
  vocode_normalize = TRUE;
  vocode_volume = 1.0;
  vocode_band_count = 16;

  if (argc <= 1) 
    {
      char buf[16];
      vocode_modulator_filename = error_malloc(PATH_MAX);
      vocode_carrier_filename = error_malloc(PATH_MAX);
      vocode_output_filename = error_malloc(PATH_MAX);
      
      ask_user_filename("Modulator filename (required)", vocode_modulator_filename, 
                        PATH_MAX, TRUE);

      ask_user_filename("Carrier filename (required)", vocode_carrier_filename, 
                        PATH_MAX, TRUE);

      ask_user("Window length (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0])  
        {
          vocode_window_length = atoi(buf);
          got_window_length = TRUE;
        }

      ask_user("Window overlap (empty for default)", buf, sizeof(buf), 
	       FALSE, TRUE);
      if (buf[0])  
        {
          vocode_window_overlap = atoi(buf);
          got_window_overlap = TRUE;
        }

      ask_user("Band count (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0]) vocode_band_count = atoi(buf);

      ask_user("Output volume (empty for default)", buf, sizeof(buf), 
               FALSE, TRUE);
      if (buf[0]) vocode_volume = atof(buf);
      ask_user_filename("Output filename (required)", vocode_output_filename, 
                        PATH_MAX, FALSE);
    } 
  else if (argc == 7 &&
      argv[1][0] != '-' && argv[2][0] != '-' &&
      atoi(argv[3]) != 0 && atoi(argv[4]) != 0 && atoi(argv[5]) != 0 &&
      argv[6][0] != '-')
    {
      vocode_modulator_filename = argv[1];
      vocode_carrier_filename = argv[2];
      vocode_window_length = atoi(argv[3]);
      got_window_length = TRUE;
      vocode_window_overlap = atoi(argv[4]);
      got_window_overlap = TRUE;
      vocode_band_count = atoi(argv[5]);
      vocode_output_filename = argv[6];
    }
  else
    {
      for (i = 1; i < argc && argv[i][0] == '-'; ++i)
        {
          if (argv[i][1] == 'v' && i < argc - 1)
            vocode_volume = atof(argv[++i]);
          else if (argv[i][1] == 'b' && i < argc - 1)
            vocode_band_count = atoi(argv[++i]);
          else if (argv[i][1] == 'q')
            quiet = TRUE;
          else if (argv[i][1] == 'N')
            vocode_normalize = FALSE;
          else if (argv[i][1] == 'w' && i < argc - 1) {
            vocode_window_length = atoi(argv[++i]);
            got_window_length = TRUE;
          }
          else if (argv[i][1] == 'o' && i < argc - 1) {
            vocode_window_overlap = atoi(argv[++i]);
            got_window_overlap = TRUE;
          }
          else
            usage();
        }
      
      if (argc != i + 3)
        usage();
      
      vocode_modulator_filename = argv[i++];
      vocode_carrier_filename = argv[i++];
      vocode_output_filename = argv[i++];
    }
}

static void check_args(void)
{
  if (!got_window_length)
    vocode_window_length = ipow(2, ilog2(vocode_modulator_rate / DEFAULT_WINDOW_TIME));
  if (!got_window_overlap)
    vocode_window_overlap = vocode_window_length / 2;
  
  if (vocode_window_length < 2 || (size_t)ipow(2, ilog2(vocode_window_length)) != vocode_window_length)
    error_display("window-length must be > 1 and a power of two\n"
          "(the closest power of two to the number you entered is %d)",
          ipow(2, ilog2(vocode_window_length)));
  if (vocode_window_overlap < 0 || (size_t)vocode_window_overlap > vocode_window_length / 2)
    error_display("window-overlap must be >= 0 and <= window-length/2");
  if (vocode_band_count < 1 || (size_t)vocode_band_count > vocode_window_length / 2)
    error_display("band-count must be > 0 and <= window-length/2");

  if (!quiet)
    message_user("%s\nwindow-length: %d   window-overlap: %d   band-count: %d   "
                 "volume: %.2f", COPYRIGHT,
                 vocode_window_length, vocode_window_overlap, vocode_band_count, vocode_volume);
}

static VINT _num_frames;

static void start_status(VINT num_frames) 
{
  _num_frames = num_frames;
}

static int update_status(VINT frame_no) {
  int i, count = (frame_no + 1) * 56 / _num_frames;
  if (quiet) return 0;
  printf("\r%3d%% |", (int)((frame_no + 1) * 100 / _num_frames));
  for (i = 0; i < count; ++i)
    putchar('*');
  for (; i < 56; ++i)
    putchar(' ');
  printf("| %ld/%ld", frame_no + 1, _num_frames);
  fflush(stdout);
  return 0;	/* Return true to quit */
}

static void finish_status(void) {
  if (!quiet)
    printf("\n");
}

static void display_error(char *msg)
{
  fprintf(stderr, "%s: %s\n", prog_name, msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  vocode_start_status_cb = start_status;
  vocode_update_status_cb = update_status;
  vocode_finish_status_cb = finish_status;
  error_display_cb = display_error;
  parse_args(argc, argv);
  vocode_open_files();
  check_args();
  vocode();
  vocode_cleanup();
  return 0;
}
