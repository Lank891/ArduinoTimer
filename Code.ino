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

//Printing state (used after changing state with new name as an argument)
void printState(String state){
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(state);
}

//If timer reaches 0 counting
void handleStop(){
  //Set alarm state and send sound
  state = 2;
  tone(pinPiezo, 440);
  printState("Stop");
}

//Formatting time and printing on LCD
void printTime(){
  int minutes = timeInSeconds / 60;
  int seconds = timeInSeconds % 60;

  lcd.setCursor(11, 1);
  //If minutes has 2 digits print them, if 1: print 0 and then minutes
  if(minutes >= 10){
    lcd.print(minutes);
  }
  else {
    lcd.print('0');
    lcd.print(minutes);
  }

  lcd.print(":");

  //If seconds has 2 digits print them, if 1: print 0 and then seconds
  if(seconds >= 10){
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

  //If button is not pressed and was pressed before (after releasing button)

  //If state = setting and minutes was pressed add 60 seconds to timer
  if(!minutesPress && minutesPressOld && state == 0){
    timeInSeconds += 60;
  }
  //If state = setting and seconds was pressed add 10 seconds to timer
  if(!secondsPress && secondsPressOld && state == 0){
    timeInSeconds += 10;
  }

  //If timer reaches 1 hour change it to 0 again
  if(timeInSeconds >= 3600){
    timeInSeconds = 0;
  }

  //If start/reset button was pressed
  if(!startPress && startPressOld){
    //If state was "setting" and we have some time change state to counting and save time
    if(state == 0 && timeInSeconds > 0){
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
  if(state == 1){
    //Check if 999ms passed, if yes substract 1 second and save new "last time"
    if(millis() - lastMillis >= 999){
        timeInSeconds -= 1;
        lastMillis = millis();
      }
    //If 0 seconds left - handleStop
     if(timeInSeconds <= 0){
      handleStop();
     }
  }

  //Save states of buttons in this cycle as states of previous one
  startPressOld = startPress;
  minutesPressOld = minutesPress;
  secondsPressOld = secondsPress;

  //Print formatted time
  printTime();
}
