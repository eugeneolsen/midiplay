## Player-Specific Meta Events
The play command uses custom MIDI Meta events to inform playback options for single-verse MIDI files.  They are used at time zero in track zero. These events are optional but helpful.  They are not sent to the connected playback device.

The Sequencer-Specific Meta Event, 0xFF7F is used as follows:

`0xFF7F` Sequencer-Specific Meta Event<br>
`length` per MIDI variable length standard<br>
`0x7D` Private, prototyping, experimental, or test event. Private to this custom player.<br>
`byte` event type<br>
`data`

Currently supported **event types** are:

`0x01` number of verses. Data = ASCII character '1' – '9'<br>
`0x02` pause between verses.  Data = unsigned 16-bit integer indicating the number of MIDI clock ticks to pause.

Examples:

`0x00FF7F037D0134` Play 4 verses.<br>
`0x00FF7F047d020528` Pause for 1,320 ticks between verses.

#### *The following custom meta events currently work but are deprecated as they may conflict with current and future MIDI specifications:*

FF 0x10 01 <i>n</i> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of Verses</b>

>>*n* specifies the number of verses to play.

<br>
FF 0x11 02 <i>tttt</i>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp <b>Pause Between Verses</b>

>>*tttt* specifies the number of ticks to pause between verses, including between the introduction and the first verse.
