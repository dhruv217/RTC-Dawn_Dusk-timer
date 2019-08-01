/* for software wire use below
#include <SoftwareWire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
SoftwareWire myWire(SDA, SCL);
RtcDS1307<SoftwareWire> Rtc(myWire);
 for software wire use above */

/////// --------------------------------------------//////////
/////// ------- To Set Time From Serial Send -------//////////
/////// -------   Time,YYYY/MM/DDThh:mm:ssZ  -------//////////
/////// -------   Time,2019/07/21T22:10:00Z  -------//////////
/////// --------------------------------------------//////////

// Rtc by Makuna ===> Arduino Library for RTC, Ds1302, Ds1307, Ds3231, & Ds3234 with deep support.
// Dawn2Dusk by DM Kishi
#define countof(a) (sizeof(a) / sizeof(a[0]))
#define LightSwitch 13
/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <Dusk2Dawn.h>
#include <RtcDS1307.h>

RtcDS1307<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

Dusk2Dawn bhopal(23.2599, 77.4126, 4.00);

int DateToday, SunRize, SunSet, MinutesSinceMidnight;

// CONNECTIONS:
// DS1307 SDA --> SDA
// DS1307 SCL --> SCL
// DS1307 VCC --> 5v
// DS1307 GND --> GND

/* for software wire use below
#include <SoftwareWire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
SoftwareWire myWire(SDA, SCL);
RtcDS1307<SoftwareWire> Rtc(myWire);
 for software wire use above */


void setup () 
{
    Serial.begin(57600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    pinMode(LightSwitch, OUTPUT);

    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");
            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue

            Rtc.SetDateTime(compiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }


    DateToday = now.Day();
    SunRize = bhopal.sunrise(now.Year(), now.Month(), now.Day(), true);
    SunSet = bhopal.sunset(now.Year(), now.Month(), now.Day(), true);
    char time3[] = "00:00";
    Dusk2Dawn::min2str(time3, SunRize);
    Serial.println(time3); // 11:56
    Dusk2Dawn::min2str(time3, SunSet);
    Serial.println(time3); // 11:56

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low); 
}

void loop () 
{
    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }
    // Process Serial Time Set Data
    if (Serial.available()) {
      // Time,2019/07/21T10:30:00Z
      processSyncMessage();
    }

    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if(now.Day() > DateToday){
      DateToday = now.Day();
      SunRize = bhopal.sunrise(now.Year(), now.Month(), now.Day(), true);
      SunSet = bhopal.sunset(now.Year(), now.Month(), now.Day(), true);
    }
    MinutesSinceMidnight = now.Hour() * 60 + now.Minute();
    if(MinutesSinceMidnight == SunRize){
      digitalWrite(LightSwitch, LOW);
    }
    if(MinutesSinceMidnight == SunSet){
      digitalWrite(LightSwitch, HIGH);
    }

    delay(10000); // ten seconds
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

void processSyncMessage() {
  String incoming;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  incoming = Serial.readString();
  if(incoming.substring(0, 4) == "Time") {
    RtcDateTime inputTime = getTimeFromISOString(incoming.substring(5));
    Serial.print("Time Recieved and Set To:  ");
    printDateTime(inputTime);
    Serial.println();
    Rtc.SetDateTime(inputTime);
  }
}


// Example Input Format String 2019/07/21T10:30:00Z 
RtcDateTime getTimeFromISOString(String time) {
  char timeArray[21];
  time.toCharArray(timeArray, 21);
  int yr, mnth, d, h, m, s;

  sscanf(timeArray, "%4d/%2d/%2dT%2d:%2d:%2dZ", &yr, &mnth, &d, &h, &m, &s);
  return RtcDateTime(yr, mnth, d, h, m, s);
}
