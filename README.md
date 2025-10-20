# 🎹 Organ Pi 🎜 MIDI File Player
## C++ (gnu++20 standard), customized fork of [cxxmidi](https://github.com/eugeneolsen/cxxmidi), standard library.
## 👉🏽 See a demo video in the <a href="https://www.raspberrypi.com/news/raspberry-pi-is-this-churchs-new-organist/" target="_blank">Raspberry Pi News</a>
## Description
The Organ Pi MIDI file player is a software MIDI sequencer that plays a MIDI file through MIDI OUT port 1 through a USB-to-MIDI converter.  This program was developed specifically to run on a Raspberry Pi 4B (or better) running Raspberry Pi OS 64-bit (or any Debian Linux derivative, such as Ubuntu) through a USB to MIDI adapter to an Allen Protégé-16 organ.  Other organs may be supported in the future.

Currently, Casio and Yamaha consumer keyboards are also supported for testing.

This project was created for people with technical aptitude.  A more user-friendly touch-screen interface is in the works.

## Features
✅ Simple Linux command line syntax.<br>
✅ Define an introduction using MIDI markers.<br>
✅ Play the number of verses defined in the MIDI file or overridden on the command line. For implementation details, see the [Meta Events document](meta_events.md)  
✅ Command line flags for prelude and postlude without playing the introduction.<br>
✅ Localized to five languages: English, Spanish, Brazilian Portuguese, and French for both France and Canada.<br>

## System Requirements
- Raspberry Pi 4B or better.  The Orange Pi 5 family of single-board computers has also been tested with success.
- 64-bit Debian 12 (Bookworm) or later.  Raspberry Pi OS based on Debian 12 works fine.
- 4GB RAM (may require less RAM; hasn't been tested with less).
- NVME SSD recommended but performs respectably with a fast SD card or flash memory.
- Allen Protégé-16 organ.  Other Allen organ models may work but have not been tested.

## Installation
For installation instructions, see the [installer documentation](midiplay-installer/README.md).

## Usage 
On the Linux or SSH command line, type:

`play` *<filename\>* *options*

*filename* need not include the extension ".mid".  The extension ".mid" is appended if not present in the *filename.*

It is recommended that aliases be added to `~/.bash_aliases` as follows:

`alias p='play'`

For Spanish and Portuguese:

`alias toque='play'`<br>
`alias toq='play'`<br>
`alias t='play'`



### Options
`-v --version`  Display the version number of this command.

`-p`*n* `--prelude=`*n* play two verses *without* the introduction as prelude or postlude where *n* indicates the speed at which to play, 10 being the tempo in the MIDI file.  The default if no number is provided for the option is 8, which is 80% of the tempo in the MIDI file.

`-n`*n* where *n* is the number of verses to play *after* the introduction.  Overrides the default number of verses specified in the MIDI file with player-specific meta event type 0x01 (see below).

`-t`*bpm*` --tempo=`*bpm*  overrides the tempo specified in the MIDI file,
where *bpm* is beats per minute.

`-x`*n* where *n* is the number of verses to play *without* an introduction.  Overrides the default number of verses specified in the MIDI file with player-specific meta event type 0x01 (see below).


## Planned Enhancements
1. Proper handling of **melodic variations** such as the extra pickup note at the beginning of the first verse of "O Come All Ye Faithful."
2. Handling of **irregular meter** such as at the beginning of verse 5 of "The Morning Breaks."
3. First and second endings.


## License

MIT - see [LICENSE](LICENSE.md) document.

#### Licensed Dependencies:
cxxmidi: MIT

## Release History
Version 1.5.8 - 2025.10.08 Various code cleanup changes.  No user-facing changes.  
Version 1.5.7 - 2025.10.02 Move magic strings to internationalization tables, localize.<br>
Version 1.5.6 - 2025.10.02 Remove namespace pollution from .cpp files<br>
Versino 1.5.5 - 2025.10.01 Replace POSIX synchronization with modern C++ synchronization<br>
Version 1.5.4 - 2025.09.30 Extract playback code into a discrete playback engine<br>
Version 1.5.3 - 2025.09.25 Add const qualifiers to getters in Options class.<br>
Version 1.5.2 - 2025.09.25 Extract MIDI file load logic and callback to a separate module<br>
Version 1.5.1 - 2025.09.22 Fix segmentation fault when environment variables not set<br>
Version 1.5.0 - 2025.09.17 Extract MIDI device management to its own class.<br>
Version 1.4.9 - 2025.09.15 Separate signal handling into its own class.<br>
Version 1.4.7 - 2025.09.02 Convert magic numbers to constants<br>
Version 1.4.6 - 2025.09.01 Throw away SysEx, Lyrics, and most Control Change events.<br>
Version 1.4.5 - 2025.08.30 Add namespace prefixes to object and function names, remove "using namespace" directives from header files.<br>
Version 1.4.4 - 2025.08.29 Play introduction only when introduction markers are present in Track 0.<br>
Version 1.4.3 - 2025.08.27 Eliminate magic numbers for custom Meta events.<br>
Version 1.4.1 - 2025.08.26 Integrate stabilized cxxmidi fork.