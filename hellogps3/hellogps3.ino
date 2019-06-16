#include <SoftwareSerial.h>


#define LED_RED_PIN 13

#define RX_PIN 10 // connect to rx pin in GPS connection
#define TX_PIN 11 // connect to tx pin in GPS connection

#define GPRMC_STR "$GPRMC,"

SoftwareSerial GPSModule(RX_PIN, TX_PIN);

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    Serial.begin(57600);
    Serial.println("setup() called");
    GPSModule.begin(9600);
}

void loop()
{
    Serial.flush();
    GPSModule.flush();
    while (GPSModule.available() > 0) {
        String gpsData = GPSModule.readStringUntil('\n');
        if (gpsData.startsWith(GPRMC_STR)) {
            if (16 == gpsData.indexOf(",A,")) {
                // valid data
                Serial.print(gpsData);
                digitalWrite(LED_BUILTIN, HIGH);
                digitalWrite(LED_RED_PIN, HIGH);
                continue;
            }
            if (16 == gpsData.indexOf(",V,")) {
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
