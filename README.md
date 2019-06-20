# ArduinoTimer
Timer created with Arduino Uno, Speaker and LCD
//
How to add new song to play (song will be choosen randomly after each end of counting):
//
Go to section  `SONG LIST TO ADDING SONGS`
//
* 1. Change value of songsCounter
* 2. In initSongs add line:
*   songsList[index].create(BPM, "StringWithNotes");
* 3. That's all, program will randomly pick song after end of counting time
//
String with notes contains characters, each note is 5 characters and there are no additional white characters
//
* Fragment of string for one note has 5 characters:
* 1 - `cdefgabp`: note height (p for pause)
* 2 - `bc-`: substract or add half of a tone (b or cross), - for nothing
* 3 - `23456`: Octave: c2/c3/c4/c5/c6 (great-small-1 line-2 line-3 line)
* 4 - `012345`: note length - 0 is full note, 1 is half, 2 is quarter ... 5 is 32th
* 5 - `.-`: . adds half of a note length (like standard), - means do nothing 
