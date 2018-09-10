#include <arduino.h>

#define FOOTSWITCH 4
#define RELAY 3
#define OPTOCOUPLER 0

long int now;
int switchState;
int lastSwitchState = 0;
int relayState = 0;
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
        relayState = !relayState;

        digitalWrite(OPTOCOUPLER, HIGH);
        delay(10);
        digitalWrite(RELAY, relayState);
        delay(30);
        digitalWrite(OPTOCOUPLER, LOW);
    }
}

int footSwitchPress()
{
    switchState = digitalRead(FOOTSWITCH);

    if (switchState == LOW && now - lastPressTime > 300)
    {
        lastSwitchState = switchState;
        return 1;
    }

    lastSwitchState = switchState;
    return 0;
}
