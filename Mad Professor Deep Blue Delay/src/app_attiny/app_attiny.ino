atti#include <tinySPI.h>

#include <arduino.h>

//////
#define MP41_CS 7
#define LED_TAP 3
#define LED_DIV_HALF 11
#define LED_DIV_THIRD 8
#define LED_DIV_QUARTER 9
#define SWITCH_TAP 2
#define ANALOGPOT A0

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
long int divInterval;
long int minInterval;
long int maxInterval;
int mappedInterval;
int newInterval = 0;

int timesTapped = 0;
int stillTapping = 0;
long int firstTapTime;
long int lastTapTime;
int tapState;
int lastTapState;
int longTapPress;
int divValue = 1;
int isDivEnabled = 0;
long int lastDivTime;

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
void lightDivLED();
void setTapCount();
void blinkTapLED(long int blinkDelay);
int analogPotTurned();
void calibration();
void setDivision();
void divReset();

//////
void setup()
{
  pinMode(MP41_CS, OUTPUT);
  pinMode(LED_TAP, OUTPUT);
  pinMode(LED_DIV_HALF, OUTPUT);
  pinMode(LED_DIV_QUARTER, OUTPUT);
  pinMode(LED_DIV_THIRD, OUTPUT);
  pinMode(SWITCH_TAP, INPUT_PULLUP);
  pinMode(ANALOGPOT, INPUT);
  pinMode(CALIN, INPUT);
  pinMode(CALOUT, OUTPUT);

  minInterval = MINDELAY;
  maxInterval = MAXDELAY;

  SPI.begin();
  
  //calibration();
}

//////
void loop()
{
  now = micros();
  checkTapTimeout();

  if (tapButtonPressed())
  {
    setTapCount();  
    lastTapTime = now;
  }

  if (timesTapped >= MINTAPS)
  {
    interval = getInterval();
    newInterval = 1;
    isDivEnabled = 1;
  }
  
  if (analogPotTurned())
  {
    interval = map(analogPotCurrVal, 0, 1023, minInterval, maxInterval);
    newInterval = 1;
    isDivEnabled = 0;
    divValue = 1;
  }
  
  if (newInterval)
  {   
    if (isDivEnabled == 1)
    {
      divInterval = interval / divValue;
      mappedInterval = map(divInterval, minInterval, maxInterval, 0, 255);      
    }
    else if (isDivEnabled == 0)
    {
      mappedInterval = map(interval, minInterval, maxInterval, 0, 255);
    }
    digitalPotWrite(mappedInterval);
    newInterval = 0;
  }
  
  if (isDivEnabled == 1)
  {
    blinkTapLED(divInterval);
  }
  else if (isDivEnabled == 0)
  {
    blinkTapLED(interval);
  }

  lightDivLED();
}

void checkTapTimeout()
{
  if(timesTapped > 0 && (now - lastTapTime) > maxInterval*1.5)
  {
    tapReset();
  }
  
  if (tapState == HIGH)
  {
    divReset();
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
  tapState = digitalRead(SWITCH_TAP);

  if (tapState == LOW && now - lastTapTime > 30000 && tapState != lastTapState )
  {
    lastTapState = tapState;
    return 1;
  }

  if (tapState == LOW && now - lastDivTime > 1000000 && tapState == lastTapState && longTapPress == 0 && isDivEnabled == 1)
  {
    setDivision();
    longTapPress = 1;
    lastDivTime = now;
  }

  if (tapState == LOW && now - lastDivTime > 1000000 && tapState == lastTapState && longTapPress == 1 && isDivEnabled == 1)
  {
    setDivision();
    lastDivTime = now;
  }

  lastTapState = tapState;
  return 0;
}

void setTapCount()
{
  if(timesTapped == 0)
  {
    firstTapTime = now;
    stillTapping = 1;
  }
  timesTapped++;
}

void blinkTapLED(long int blinkDelay)
{
  if(now >= nextBlinkSync)
  {
    nextBlinkSync = now + blinkDelay*2;
    nextBlinkTime = now;
  }

  if (now >= nextBlinkTime)
  {
    digitalWrite(LED_TAP, HIGH);
    nextBlinkTime = blinkDelay + now;
  }

  if (now >= nextBlinkTime - blinkDelay/2)
  {
    digitalWrite(LED_TAP, LOW);
  }
}

int analogPotTurned()
{
  analogPotCurrVal = analogRead(ANALOGPOT);
  if(analogPotCurrVal > analogPotLastVal + 3 || analogPotCurrVal < analogPotLastVal - 3)
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

void lightDivLED()
{
  if(isDivEnabled == 1)
  {
    switch (divValue)
    {
      case 1:
        digitalWrite(LED_DIV_QUARTER, LOW);
        break;
      case 2:
        digitalWrite(LED_DIV_HALF, HIGH);
        break;
      case 3:
        digitalWrite(LED_DIV_HALF, LOW);
        digitalWrite(LED_DIV_THIRD, HIGH);
        break;
      case 4:
        digitalWrite(LED_DIV_THIRD, LOW);
        digitalWrite(LED_DIV_QUARTER, HIGH);
        break;
    }
  }
  else
  {
    digitalWrite(LED_DIV_HALF, LOW);
    digitalWrite(LED_DIV_QUARTER, LOW);
    digitalWrite(LED_DIV_THIRD, LOW);
  }  
}

void setDivision()
{
  divValue++;
  newInterval = 1;
  if(divValue > 4)
  {
    divValue = 1;
  }
}

void divReset()
{
  longTapPress = 0;
  lastDivTime = now;
}

