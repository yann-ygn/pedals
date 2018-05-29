#include <arduino.h>

#define FOOTSWITCH 0
#define RELAY 1
#define OPTOCOUPLER 2

int now;
int switchState;
int lastSwitchState = 0;
long int lastPressTime;

int footSwitchPress();

void setup()
{
    pinMode(FOOTSWITCH, INPUT_PULLUP);
    pinMode(RELAY, OUTPUT);
    pinMode(OPTOCOUPLER, OUTPUT);
}

void loop()
{
    now = millis();

    if (footSwitchPress())
    {
        lastPressTime = now;
    }
}

int footSwitchPress()
{
    switchState = digitalRead(FOOTSWITCH);

    if (switchState == LOW && now - lastPressTime > 30000 && switchState != lastSwitchState )
    {
        lastSwitchState = switchState;
        return 1;
    }

    lastSwitchState = switchState;
    return 0;
}