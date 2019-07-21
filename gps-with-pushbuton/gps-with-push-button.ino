#include <SoftwareSerial.h>
#include <SPI.h>
#include "SdFat.h"

#include <MicroNMEA.h>
#include "DebounceButton.h"

#define SERIAL_BAUD 115200
#define GPS_BAUD 9600
#define LED_RED_PIN 3
#define BUTTON_PIN 2

#define RX_PIN 5 // connect to rx pin in GPS connection
#define TX_PIN 6 // connect to tx pin in GPS connection

#define GPRMC_STR "$GPRMC,"
#define POI_STR "poi";

// LED state
bool ledState = LOW;

// sdcard
const int chipSelect = 10;
int sdcardInitialized = 0;
SdFile logFile;
SdFat sd;
#define error(msg) sd.errorHalt(F(msg))

// gps module
SoftwareSerial GPSModule(RX_PIN, TX_PIN);
String lastPosition;

// nmea parser
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

volatile int buttonStatus = 0; // variable reading the button's status, vektor 0
                               // button connection on pin 2 corresponds vektor 0 (this case)
                               // button connection on pin 3 corresponds vektor 1

DebounceButton button(BUTTON_PIN);

void handleButtonInterrupt()
{
    button.handle(lastPosition);
}

void initSD()
{
    if (!sdcardInitialized) {
        // sd card
        if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
            sd.initErrorHalt();
            Serial.println("Card failed, or not present");
        } else {
            Serial.println("Card initialized");
            sdcardInitialized = 1;
        }
    }
}

void gpsHardwareReset()
{
	// Empty input buffer
	while (GPSModule.available())
		GPSModule.read();

	// Reset is complete when the first valid message is received
	while (1) {
		while (GPSModule.available()) {
			char c = GPSModule.read();
			if (nmea.process(c)) {
                nmea.clear();
				return;
            }
		}
	}
}

void toggleLED()
{
    digitalWrite(LED_BUILTIN, ledState);
    digitalWrite(LED_RED_PIN, ledState);
    ledState = !ledState;
}

void writeToSD(String sentence, String filename)
{
    String logFilename = filename;
    String poiFilename = String("p") + filename;

    if (sdcardInitialized) {
        // save position
        if (logFile.open(logFilename.c_str(), O_WRONLY | O_CREAT | O_APPEND)) {
            logFile.println(sentence);
            logFile.flush();
            logFile.close();
            Serial.println(sentence);
            toggleLED();
        } else {
            error("file.open");
            Serial.println("error opening " + logFilename);
        }
    
        // save poi
        String value = button.getValue();
        if (value.length() > 0) {
            if (logFile.open(poiFilename.c_str(), O_WRONLY | O_CREAT | O_APPEND)) {
                logFile.println(value);
                logFile.flush();
                logFile.close();
                Serial.println("POI saved: " + value);
                button.resetValue();
            } else {
                Serial.println("error opening " + poiFilename);
            }
        }
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    toggleLED();
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("setup() called");

    // sd card
    initSD();

    // attach interrupt handler
    button.init();
    attachInterrupt(0, handleButtonInterrupt, CHANGE);

    // prepare gps
    GPSModule.begin(GPS_BAUD);
    Serial.println("Resetting GPS module ...");
	gpsHardwareReset();
	Serial.println("... done");

    // Clear the list of messages which are sent.
	MicroNMEA::sendSentence(GPSModule, "$PORZB");

	// Send only RMC and GGA messages.
    MicroNMEA::sendSentence(GPSModule, "$PORZB,RMC,1,GGA,1");

	// Disable compatability mode (NV08C-CSM proprietary message) and
	// adjust precision of time and position fields
	MicroNMEA::sendSentence(GPSModule, "$PNVGNME,2,9,1");

    Serial.println("setup() completed");
}

void loop()
{
    Serial.flush();
    GPSModule.flush();

    while (GPSModule.available() > 0)
    {
        char c = GPSModule.read();
        
        if (nmea.process(c) && nmea.isValid()) {
            lastPosition = String(nmea.getSentence());
            writeToSD(
                lastPosition, 
                String(nmea.getYear()) + String(nmea.getMonth()) + String(nmea.getDay()) + String(".txt")
            );
            nmea.clear();
        }
    }
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}
