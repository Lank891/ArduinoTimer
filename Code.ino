#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //Setting LCD

const int pinStart = A2; //Pin for Start/Reset button
const int pinMinutes = A1; //Pin for adding 1 minute
const int pinSeconds = A0; //Pin for adding 10 seconds
const int pinPiezo = 8; //Pin for piezo sound


int timeInSeconds; //Time left in seconds
int state; //State of timer: 0 - setting time, 1 - counting, 2 - alarm

//For button pressing - actual state of a button and its last state
bool startPress;
bool minutesPress;
bool secondsPress;
bool startPressOld;
bool minutesPressOld;
bool secondsPressOld;

//For measuring time
unsigned long lastMillis;

//Playing alarm knowing how many ms passed form the start
void playAlarm(unsigned long ms) {

  //That's megalovania 8s fragment
  ms = 1 + ( ms % 8000 );
  if (ms >= 1 && ms <= 250) tone(pinPiezo, 294);
  else if (ms >= 2001 && ms <= 2250) tone(pinPiezo, 262);
  else if (ms >= 4001 && ms <= 4250) tone(pinPiezo, 247);
  else if (ms >= 6001 && ms <= 6250) tone(pinPiezo, 233);
  else {
    //That's reapeating fragment so we need only from  251-2000, 2251-4000 etc
    //Mod 2001 so we will have always from 0 to 2000, but we will have x251 b/c
    //of others conditions so we will have always value in range [251, 2000];
    ms = ms % 2001;

    if (ms >= 251 && ms <= 500) tone(pinPiezo, 587);
    else if (ms >= 501 && ms <= 875) tone(pinPiezo, 440);
    else if (ms >= 876 && ms <= 1000) tone(pinPiezo, 415);
    else if (ms >= 1001 && ms <= 1125) noTone(pinPiezo);
    else if (ms >= 1126 && ms <= 1375) tone(pinPiezo, 392);
    else if (ms >= 1376 && ms <= 1625) tone(pinPiezo, 349);
    else if (ms >= 1626 && ms <= 1750) tone(pinPiezo, 294);
    else if (ms >= 1751 && ms <= 1875) tone(pinPiezo, 349);
    else if (ms >= 1876 && ms <= 2000) tone(pinPiezo, 392);
  }
}

//Printing state (used after change with new name)
void printState(String state) {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(state);
}

//If timer reaches 0 counting
void handleStop() {
  //Set alarm state and
  state = 2;
  printState("Stop");

  //Set timer for meauring time to 0 for playing alarm
  lastMillis = millis();
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

void setup() {
  //Setting values
  lcd.begin(16, 2);
  state = 0;
  timeInSeconds = 0;
  startPressOld = false;
  minutesPressOld = false;
  secondsPressOld = false;
  lastMillis = 0;

  //Print initial state:
  printState("Set Time:");
}

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
    //If 0 seconds left - handleStop
    if (timeInSeconds <= 0) {
      handleStop();
    }
  }

  //If we are playing alarm
  if (state == 2) {
    //Play alarm, argument is milliseconds from the moment
    //we started playing alarm
    playAlarm(millis() - lastMillis);
  }

  //Save states of buttons in this cycle as states of previous one
  startPressOld = startPress;
  minutesPressOld = minutesPress;
  secondsPressOld = secondsPress;

  //Print formatted time
  printTime();
}
