/******************************************************************************
 * $Id: vocode.h,v 1.2 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1996-1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include "config.h"

extern char *vocode_modulator_filename, *vocode_carrier_filename, *vocode_output_filename;
extern size_t vocode_window_length, vocode_window_overlap;
extern int vocode_band_count;
extern VREAL vocode_volume;
extern VBOOL vocode_normalize;

extern VINT vocode_modulator_rate;

extern void (*vocode_start_status_cb)(VINT num_frames);
extern VBOOL (*vocode_update_status_cb)(VINT frame_no);
extern void (*vocode_finish_status_cb)(void);

void vocode_open_files(void);
void vocode_cleanup(void);
void vocode(void);
