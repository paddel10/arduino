#include <SoftwareSerial.h>

#define LED_RED_PIN 13
#define BUTTON_PIN 2

#define RX_PIN 10 // connect to rx pin in GPS connection
#define TX_PIN 11 // connect to tx pin in GPS connection

#define GPRMC_STR "$GPRMC,"

SoftwareSerial GPSModule(RX_PIN, TX_PIN);
String lastPosition;

volatile int buttonStatus = 0; // variable reading the button's status, vektor 0
                               // button connection on pin 2 corresponds vektor 0 (this case)
                               // button connection on pin 3 corresponds vektor 1

class DebounceButton
{
    const byte pin;
    int state;
    unsigned long buttonDownMs;

protected:
    void shortClick()
    {
        Serial.print("shortClick() : " + lastPosition);
    }

    void longClick()
    {
        Serial.println("longClick()");
    }

public:
    DebounceButton(byte attachTo) : pin(attachTo)
    {
    }

    void init()
    {
        pinMode(pin, INPUT_PULLUP);
        state = HIGH;
    }

    void handle()
    {
        int prevState = state;
        state = digitalRead(pin);
        if (prevState == HIGH && state == LOW) {
            buttonDownMs = millis();
        } else if (prevState == LOW && state == HIGH)
        {
            if (millis() - buttonDownMs < 50)
            {
                // ignore this for debounce
            }
            else if (millis() - buttonDownMs < 250) {
                shortClick();
            } else {
                longClick();
            }
        }
    }
};

DebounceButton button(BUTTON_PIN);

void handleButtonInterrupt()
{
    button.handle();
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    Serial.begin(57600);
    Serial.println("setup() called");
    // attach interrupt handler
    button.init();
    attachInterrupt(0, handleButtonInterrupt, CHANGE);
    GPSModule.begin(9600);
}

void loop()
{
    Serial.flush();
    GPSModule.flush();
    while (GPSModule.available() > 0)
    {
        String gpsData = GPSModule.readStringUntil('\n');
        if (gpsData.startsWith(GPRMC_STR))
        {
            if (16 == gpsData.indexOf(",A,"))
            {
                // valid data
                lastPosition = gpsData;
                Serial.print(gpsData);
                digitalWrite(LED_BUILTIN, HIGH);
                digitalWrite(LED_RED_PIN, HIGH);
                continue;
            }
            if (16 == gpsData.indexOf(",V,"))
            {
                // invalid data
                Serial.println("void");
            }
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(LED_RED_PIN, LOW);
        }
    }
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}
