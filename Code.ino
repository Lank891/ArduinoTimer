#include <LiquidCrystal.h>

//------------------CONSTANTS AND VARIABLES---------------------------

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //Setting LCD

const short pinStart = A2; //Pin for Start/Reset button
const short pinMinutes = A1; //Pin for adding 1 minute
const short pinSeconds = A0; //Pin for adding 10 seconds
const short pinPiezo = 8; //Pin for piezo sound

unsigned short timeInSeconds; //Time left in seconds
unsigned short state; //State of timer: 0 - setting time, 1 - counting, 2 - alarm

//For button pressing - actual state of a button and its last state
bool startPress;
bool minutesPress;
bool secondsPress;
bool startPressOld;
bool minutesPressOld;
bool secondsPressOld;

//For measuring time
unsigned long lastMillis;

unsigned short songNum; //number of actually playing song

//----------------MUSIC INTERFACE-----------------------------------

//Class for one note - contsins frequency (0 for noTone)
// and timeStamp which tells us afrer how many millis it should start
class note {
public:
  void create(unsigned short _f, unsigned long _ts){
    freq = _f;
    timeStamp = _ts;
  }
  unsigned short freq;
  unsigned long timeStamp;
};

//Class for one song - contains vector of notes and duration of song in millis
//Assuming that everything is correct since it has to be coded anyway
//and lenOfMusic - number of notes
class music {
public:
  void create(unsigned short bpm, String sounds){
    float qLen = 60000./(float)bpm; //ms of quarter
    float hLen = qLen * 2.;  //ms of half note
    float wLen = qLen * 4.;  //ms of whole note
    float eLen = qLen / 2.;  //ms of 8th
    float sLen = qLen / 4.;  //ms of 16th
    float tLen = qLen / 8.;  //ms of 32th

    unsigned long actualLength = 0; //moment of song we are now, start at 10ms
    //Since we have additional note at the begining

    lenOfMusic = sounds.length() / 5; //no of notes
    notes = new note[lenOfMusic];

    //Reading notes and adding them to notes vector
    for(unsigned short n = 0; n < lenOfMusic; n++){
      //Informations from string
      char heigth = sounds.charAt(5*n + 0);
      char halfTone = sounds.charAt(5*n + 1);
      char octave = sounds.charAt(5*n + 2);
      char note = sounds.charAt(5*n + 3);
      char addLen = sounds.charAt(5*n + 4);

      //Actual informations about note
      float noteFreq;
      float noteLen;

      //Switching possible characters (assuming correctness)
      //Set initial freq as C4 octave
      switch (heigth) {
        case 'c': noteFreq = 261.63; break;
        case 'd': noteFreq = 293.66; break;
        case 'e': noteFreq = 329.63; break;
        case 'f': noteFreq = 349.23; break;
        case 'g': noteFreq = 392.00; break;
        case 'a': noteFreq = 440.00; break;
        case 'b': noteFreq = 493.88; break;
        default: noteFreq = 0; break; // 'p' = default = pause
      };
      //+- half a tone
      switch (halfTone) {
        case 'b': noteFreq /= 1.0594631; break;
        case 'c': noteFreq *= 1.0596431; break;
        default: break; //'-' = default = do nothing
      };
      //Real octave
      switch (octave) {
        case '2': noteFreq /= 4.; break;
        case '3': noteFreq /= 2.; break;
        case '5': noteFreq *= 2.; break;
        case '6': noteFreq *= 4.; break;
        default: break; //Case '4' = default = do nothing
      };
      //Mote length
      switch (note) {
        case '0': noteLen = wLen; break;
        case '1': noteLen = hLen; break;
        case '2': noteLen = qLen; break;
        case '3': noteLen = eLen; break;
        case '4': noteLen = sLen; break;
        default: noteLen = tLen; break; //default = '5' = 32th
      };
      //Additional dot
      switch (addLen){
        case '.': noteLen *= 1.5; break;
        default: break; //default = '-' = do nothing
      };
      
      notes[n].create(noteFreq, actualLength);
      actualLength += noteLen;
    }  

    duration = actualLength;
  }

  note *notes; //Array of notes
  unsigned long duration; //Duration of a song
  unsigned short lenOfMusic; //Length of a music
};

//--------------SONG LIST TO ADDING SONGS-------------------------------

/* 
 * Fragment of string for one note has 5 characters:
 * 1 - `cdefgabp`: note height (p for pause)
 * 2 - `bc-`: substract or add half of a tone (b or cross), - for nothing
 * 3 - `23456`: Octave: c2/c3/c4/c5/c6 (great-small-1 line-2 line-3 line)
 * 4 - `012345`: note length - 0 is full note, 1 is half, 2 is quarter ... 5 is 32th
 * 5 - `.-`: . adds half of a note length (like standard), - means do nothing 
 */

 /*
  * To add new song:
  * 1. Change value of songsCounter
  * 2. In initSongs add line:
  *   songsList[index].create(BPM, "StringWithNotes");
  * 3. That's all, program will randomly pick song after end of counting time
  */

const short songsCounter = 1; //Number of songs

//List of songs
music songsList[songsCounter];

//Initialize songs
void initSongs(){
  //Megalovania
  songsList[0].create(120, "d-44-d-44-d-53-a-43.ab44-p-44-g-43-f-44-f-44-d-44-f-44-g-44-c-44-c-44-d-53-a-43.ab44-p-44-g-43-f-44-f-44-d-44-f-44-g-44-bc34-b-34-d-53-a-43.ab44-p-44-g-43-f-44-f-44-d-44-f-44-g-44-bb34-b-34-d-53-a-43.ab44-p-44-g-43-f-44-f-44-d-44-f-44-g-44-");
}

//-------------------PLAYING ALARMS---------------------------------

//Caching last note
unsigned short lastNote;

//Playing alarm knowing how many ms passed form the start and no of song
void playAlarm(unsigned long ms, unsigned short noOfSong) {  
  //For looping the song
  ms = ms % (songsList[noOfSong].duration + 1);
  
  unsigned short freqToPlay = 0; //Actual frequency to play
  bool playNewNote = false; //Should we play next note

  //If this is the last note check if we should reset or pass
  if(lastNote == songsList[noOfSong].lenOfMusic - 1) {
    //If we should reset music set lastNote to 0 and save info to play first note
    if(ms < songsList[noOfSong].notes[lastNote].timeStamp){
      lastNote = 0;
      freqToPlay = songsList[noOfSong].notes[0].freq;
      playNewNote = true;
    }
    //Else just pass
    else {
      return;
    }
  }

  //If it is the time for the next note save freq, add next note and set flag to play new note
  if(ms >= songsList[noOfSong].notes[lastNote + 1].timeStamp){
    freqToPlay = songsList[noOfSong].notes[lastNote + 1].freq;
    lastNote++;
    playNewNote = true;
  }
  
  //Play tone (if freq = 0 it is noTone)
  if(playNewNote){
    if(freqToPlay == 0){
      noTone(pinPiezo);
    }
    else {
      tone(pinPiezo, freqToPlay);
    }
  }
}

//------------ADDITIONAL INFO FUNCTIONS AND END OF COUNTER HANDLING------------------

//Printing state (used after change with new name)
void printState(String state) {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(state);
}

//If timer reaches 0 counting
void handleEndOfCounter() {
  //Set alarm state and
  state = 2;
  printState("Stop");
  //Pick random song to play
  songNum = random(0, songsCounter);
  //Set timer for meauring time to 0 for playing alarm
  lastMillis = millis();
  //Setting last note to the last one so first note is good
  lastNote = songsList[songNum].lenOfMusic - 1;
}

//Formatting time and printing on LCD
void printTime() {
  int minutes = timeInSeconds / 60;
  int seconds = timeInSeconds % 60;

  lcd.setCursor(11, 1);
  //If minutes has 2 digits print them, if 1: print 0 and then minutes
  if (minutes >= 10) {
    lcd.print(minutes);
  }
  else {
    lcd.print('0');
    lcd.print(minutes);
  }

  lcd.print(":");

  //If seconds has 2 digits print them, if 1: print 0 and then minutes
  if (seconds >= 10) {
    lcd.print(seconds);
  }
  else {
    lcd.print('0');
    lcd.print(seconds);
  }
}

//---------------------SETUP FUNCTION------------------------------------

void setup() {
  //Setting values
  lcd.begin(16, 2);
  state = 0;
  timeInSeconds = 0;
  startPressOld = false;
  minutesPressOld = false;
  secondsPressOld = false;
  lastMillis = 0;

  //Seed from noise in analog 5 pin
  randomSeed(analogRead(5));

  //Print initial state:
  printState("Set Time:");

  //Initialize songs
  initSongs();
}

//--------------------MAIN LOOP FUNCTION-------------------------------------

void loop() {
  //Catching if buttons are pressed (press should always give 1023, but in case)
  startPress = analogRead(pinStart) > 511;
  minutesPress = analogRead(pinMinutes) > 511;
  secondsPress = analogRead(pinSeconds) > 511;

  //If button is not pressed and was (after releasing button)

  //If state = setting and minutes was pressed add 60 seconds to timer
  if (!minutesPress && minutesPressOld && state == 0) {
    timeInSeconds += 60;
  }
  //If state = setting and seconds was pressed add 10 seconds to timer
  if (!secondsPress && secondsPressOld && state == 0) {
    timeInSeconds += 10;
  }

  //If timer reaches 1 hour change it to 0 again
  if (timeInSeconds >= 3600) {
    timeInSeconds = 0;
  }

  //If start/reset button was pressed
  if (!startPress && startPressOld) {
    //If state was "setting" and we have some time change state to counting and save time
    if (state == 0 && timeInSeconds > 0) {
      state = 1;
      lastMillis = millis();
      printState("Time Left:");
    }
    //Else (state was counting or alarm) change state to setting and reset timer
    else {
      state = 0;
      timeInSeconds = 0;
      noTone(pinPiezo); //Mute piezo
      printState("Set Time:");
    }
  }

  //If we are counting
  if (state == 1) {
    //Check if 999ms passed, if yes substract 1 second and save new "last time"
    if (millis() - lastMillis >= 999) {
      timeInSeconds -= 1;
      lastMillis = millis();
    }
    //If 0 seconds left - handleEndOfCounter
    if (timeInSeconds <= 0) {
      handleEndOfCounter();
    }
  }

  //If we are playing alarm
  if (state == 2) {
    //Play alarm, argument is milliseconds from the moment
    //we started playing alarm
    playAlarm(millis() - lastMillis, songNum);
  }

  //Save states of buttons in this cycle as states of previous one
  startPressOld = startPress;
  minutesPressOld = minutesPress;
  secondsPressOld = secondsPress;

  //Print formatted time
  printTime();
}
