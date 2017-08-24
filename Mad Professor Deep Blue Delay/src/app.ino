#include <arduino.h>
#include <spi.h>

//////
#define MP41_CS 53
#define LED 41
#define SWITCH 40
#define ANALOGPOT 8

#define CALIN 1
#define CALOUT 30

#define MAXDELAY 573029
#define MINDELAY 32935

#define MINTAPS 3

//////
byte address = 0x11;
int i=0;
int lastDigPotValue = 0;
int digPotValue = 0;

long int now;
long int interval;
long int minInterval;
long int maxInterval;
int mappedInterval;

int timesTapped;
int stillTapping;
long int firstTapTime;
long int lastTapTime;
int tapState;
int lastTapState;

long int nextBlinkTime;
long int nextBlinkSync;

int analogPotCurrVal;
int analogPotLastVal;

//////
void checkTapTimeout();
void tapReset();
void digitalPotWrite(int value);
long int getInterval();
int tapButtonPressed();
void firstPress();
void blinkLed(long int blinkDelay);
int analogPotTurned();
void calibration();

//////
void setup()
{
  pinMode(MP41_CS, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  pinMode(ANALOGPOT, INPUT);
  pinMode(CALIN, INPUT);
  pinMode(CALOUT, OUTPUT);

  minInterval = MINDELAY;
  maxInterval = MAXDELAY;

  SPI.begin();
  Serial.begin(9600);
  tapReset();
  digitalWrite(CALOUT, HIGH);
  //calibration();
}

void loop()
{
  now = micros();
  checkTapTimeout();
  
  if(timesTapped >= MINTAPS)
  {
    interval = getInterval();
  }
  
  if(analogPotTurned())
  {
    interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval);
  }
  
  if (tapButtonPressed())
  {
    firstPress();
  
    lastTapTime = now;
  }
  
  mappedInterval = map(interval, minInterval, maxInterval, 0, 255);
  
  digitalPotWrite(mappedInterval);
  
  blinkLed(interval);
  
  //Serial.println(analogPotCurrVal);
  //Serial.println(mappedInterval);
  //Serial.println(timesTapped);
  //Serial.println(interval); 
}

void checkTapTimeout()
{
  if(timesTapped > 0 && (now - lastTapTime) > maxInterval*1.5)
  {
    tapReset();
  }
}

void tapReset()
{
  timesTapped = 0;
  stillTapping = 0;
}

void digitalPotWrite(int value)
{
  if (value > 255)
  {
    value = 255;
  }
  else if (value < 0)
  {
    value = 0;
  }

  if (value != lastDigPotValue)
  {

    digitalWrite(MP41_CS, LOW);
    SPI.transfer(address);
    SPI.transfer(value);
    digitalWrite(MP41_CS, HIGH);
    lastDigPotValue = value;
  }
}

long int getInterval()
{
  long int avgTapInterval = (lastTapTime - firstTapTime) / (timesTapped - 1);

  if (avgTapInterval > maxInterval + 10000)
  {
    avgTapInterval = maxInterval;
  }
  if (avgTapInterval < minInterval - 10000)
  {
    avgTapInterval = minInterval;
  }
  return avgTapInterval;
}

int tapButtonPressed()
{
  tapState = digitalRead(SWITCH);
  if(tapState == LOW && now - lastTapTime > 30000 && tapState != lastTapState )
  {
    lastTapState = tapState;
    return 1;
  }
  lastTapState = tapState;
  return 0;
}

void firstPress()
{
  if(timesTapped == 0)
  {
    firstTapTime = micros();
    stillTapping = 1;
  }
  timesTapped++;
}

void blinkLed(long int blinkDelay)
{
  if(now >= nextBlinkSync)
  {
    nextBlinkSync = now + blinkDelay*2;
    nextBlinkTime = now;
  }

  if(now >= nextBlinkTime)
  {
    digitalWrite(LED, HIGH);
    nextBlinkTime = blinkDelay + now;
  }

  if(now >= nextBlinkTime - blinkDelay/2)
  {
    digitalWrite(LED, LOW);
  }
}

int analogPotTurned()
{
  analogPotCurrVal = analogRead(ANALOGPOT);
  if(analogPotCurrVal > analogPotLastVal + 3 || analogPotCurrVal < analogPotLastVal - 3 )
  {
    analogPotLastVal = analogPotCurrVal;

    return 1;
  } 
  else return 0;
}

void calibration()
{
  digitalPotWrite(0);
  digitalWrite(CALOUT, HIGH);
  long int startTime = micros();
  delay(5);
  digitalWrite(CALOUT, LOW);
  do
  {
    Serial.println(analogRead(CALIN));
    Serial.println(micros() - startTime);
  }
  while (analogRead(CALIN) < 600);
  long int minDiff = micros() - startTime;

  delay(1000);
  digitalPotWrite(255);

  digitalWrite(CALOUT, HIGH);
  startTime = micros();
  delay(5);
  digitalWrite(CALOUT, LOW);
  do
  {
    Serial.println(analogRead(CALIN));
    Serial.println(micros() - startTime);
  }
  while (analogRead(CALIN) < 600);
  long int maxDiff = micros() - startTime;


  // eepromWriteLong(42, 0);
  // eepromWriteLong(minDiff, 4);
  // eepromWriteLong(maxDiff, 8);

  /*Serial.print("42?: ");
  Serial.println(eepromReadLong(0));
  Serial.println(minDiff);
  Serial.println(eepromReadLong(4));
  Serial.println(maxDiff);
  Serial.println(eepromReadLong(8));*/
}
