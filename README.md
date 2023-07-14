# **play** command
Plays a MIDI file through MIDI OUT port 1 through a USB port.  This program was developed specifically to run on a Raspberry Pi 4B running Raspberry Pi OS 64-bit through a USB to MIDI adapter to an Allen Protege-16 organ.

## Usage 
play *<filename\>* *options*


## Options
-v --version  Display the version number of this command.

-p*n* --prelude=*n* play two verses without the introduction as prelude or postlude where *n* indicates the speed at which to play, 10 being the tempo in the MIDI file.  The default if no number is provided for the option is 8, which is 80% of the tempo in the MIDI file.

-n*n* where *n* is the number of verses to play *after* the introduction.  Overrides the default number of verses specified in the MIDI file with the 0x10 Meta event.

-t*bpm* --tempo=*bpm*  overrides the tempo specified in the MIDI file.

-x*n* where *n* is the number of verses to play *without* an introduction.  Overrides the default number of verses specified in the MIDI file with the 0x10 Meta event.

## Custom Meta Events
The play command uses custom Meta events to inform playback options for single-verse MIDI files.  These events are optional but helpful.  They are not sent to the connected playback device.

<br>
FF 0x10 01 <i>n</i> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of Verses</b>

>>*n* specifies the number of verses to play.

<br>
FF 0x11 02 <i>tttt</i>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp <b>Pause Between Verses</b>

>>*tttt* specifies the number of ticks to pause between verses, including between the introduction and the first verse.