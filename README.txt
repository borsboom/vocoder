$Id: README.txt,v 1.5 1999/03/16 06:56:14 emanuel Exp $

========================================================================
Zerius Vocoder 1.2 README
------------------------------------------------------------------------
Emanuel Borsboom <emanuel@zerius.com> March 1999
========================================================================

This program is used to make speech sound cool.  It imposes the vocal
effects of speech onto another sound.  This technique has been made
popular by artists such as Kraftwerk and Laurie Anderson.

------------
Installation
------------

If you are using the C source distribution, the first step is to
compile the vocoder.  Uncompress the archive and glance over the
Makefile, making sure the variables are all right.  The defaults
should work for most UNIX environments.  Also check config.h and make
sure that S32, U32, S16, and U16 are defined appropriately as signed
and unsigned 32-bit and 16-bit integers, respectively.  The defaults
should be fine for most 32-bit platforms.  Once finished, run 'make'
to compile the vocoder.

Once you have the executable (or if you have downloaded a binary), you
can copy it to the directory where you keep your binaries, or just run
it where it is.

-----
Usage
-----

There are two ways to run the vocoder.  If it is run without any
command-line arguments (by clicking on its icon in Windows, for example)
it will ask you for the values of the parameters.  The meanings of the
parameters follow in the next section.

To specify tho parameters on the command-line, use the following syntax:

        vocoder [-q] [-N] [-b <band-count>] [-w <window-length] 
                [-o <window-overlap>] [-v <volume>] 
                <modulator-file> <carrier-file> <output-file>

(Note: this version also supports the version 1.0 syntax in order
to be compatible with already existing front ends).

----------
Parameters
----------

A detailed explanation of what these parameters mean is in the next
section.

Modulator filename (<modulator-file>)
	the path to a sound file that contains the modulator waveform
	(required).

Carrier filename (<carrier-file>)
	the path to a sound file that contains the carrier waveform
	(required).

Window length (-w <window-length>)
	the number of samples that will be analyzed at a time, and must
	be a power of two (defaults to about 1/15th of a second worth of
	samples).

Window overlap (-o <window-overlap>)
	the number of samples that the windows will be overlapped
	(defaults to one half of the window-length).

Band count (-b <band-count>)
	the number of frequency bands that the carrier will be modulated
	with (defaults to 16).

Output volume (-v <volume>)
	the volume the output will be scaled by (defaults to 1.0).

Output filename (output-file)
	is the path to the output sound file (required).

These options are only available on the command-line:

-N	turns off normalizing the output with respect to the carrier. 

-q	turns off any displays.

The input sound files must be mono, 8- or 16-bit linear, uncompressed
AIFF or WAVE files.  The output sound file will have the same format
as the modulator (regardless of the file extension you give it).

-----------
Explanation
-----------

This channel vocoder works by analyzing the frequencies in the
modulator, splitting them into bands, finding the magnitude of each
band, and then amplifying the corresponding bands of the carrier by
that magnitude.

The modulator should simply be speech.  It works best of you speak
very clearly and more slowly than usual.

The carrier should be some kind of frequency rich waveform.  White
noise works well.  Periodic white noise (i.e. a very short sample of
white noise) gives a "robot-like" sound.  Another one that sounds good
is a synthesized string chord.  This waveform will automatically be
looped.  You can get interesting results by having the waveform change
over time.

Since what you pronounce changes over time, it would be pointless to
analyze the entire modulator waveform and excite those frequencies in
the carrier at once.  Instead, the program splits the modulator into
"windows", which it processes one-at-a-time.  The window-length
specifies how many samples are in each window.  You will want at least
a few windows for every syllable.  If this number is too large, the
output will be not be very understandable.  If it is too small, you
will have other problems.  Around 1/15th of a second (or the sampling
rate of the sound file divided by 15) tends to sound good, but
experiment to find the right value.  To give you an example, anywhere
from 512 to 2048 is okay for a modulator with a sampling rate of 44.1
khz.  If you half the sampling rate, you should half the
window-length, etc.  The window-length must be a power of two due to
the technique that us used to analyze the frequencies.

For those of you who are unfamiliar with the term "power of two," it
means a number that can be created by multiplying some number of two's
together.  For example, the following numbers are the powers of two up
to 4096:

        2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096

You get the next power of two by doubling the previous one.

Since the sound is processed in discrete windows, the output can
change very abruptly where it goes from one chunk to the next.  This
is audible as a click.  To remedy this, the program can have the
windows overlap and cross-fade between them.  The window-overlap
specifies how many samples of overlap there are between windows.
1/8th of the window-length tends to be a good starting point, but
in many cases, one half of the window-length gives the best results.  
This may not exceed half of the window-length.

In order to excite the frequencies in the carrier, the frequencies of
the modulator are split into bands.  The larger your band-count, the
more the output will sound like the modulator.  This number should
evenly divide the chunk-length for the best results.  Somewhere
between 8 and 64 usually sounds best.  The band-count may not exceed
half of the window-length.

If you find that the output is clipped (distorted) or is too quiet,
you can specify a value for the volume.  Anything less than one will
reduce the volume, and anything greater than one will increase it.

While the defaults for the parameters generally produce decent
results, the best results will be achieved by changing their values.
The best way figure out all the numbers and what the best waveforms
are is to experiment.  Have fun!

----------------
Closing Comments
----------------

Please see the web site at

        http://zerius.com/vocoder/

for the latest information.  The latest version will always be
available from there, or from the FTP site at

        ftp://ftp.zerius.com/pub/zerius/vocoder/

If you have any problems, don't hesitate to contact me.  I am always
pleased to help.  Also, drop me a line if like this program, or have
any suggestions.  I am especially eager to hear your creations.  If
you release some music utilizing the vocoder, please tell me so I can
try to find it (freebies are always accepted)!  My e-mail address is
emanuel@zerius.com.

Chanks to Cody Jones <cody@zerius.com> for porting to MacOS.

I appreciate any bug reports.

---------
Copyright
---------

The Zerius Vocoder is Copyright (C) 1996-1998 Emanuel Borsboom.

The FFT code (contained in fftn.c, fftaux.c, fft.h, and spt.h) is
Copyright (C) 1993 Steven Trainoff.

The code for converting to and from IEEE floating-point numbers is
Copyright (C) 1988-1991 Apple Computer Inc.

You are free to do whatever you like with the vocoder, as long as the
copyright notice stays intact and you note any changes.

There is no warranty.

