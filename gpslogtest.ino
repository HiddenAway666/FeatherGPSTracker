#include <SPI.h>
#include <SD.h>
#include <Adafruit_GPS.h>

#define GPSSerial Serial1

Adafruit_GPS GPS(&GPSSerial);

#define GPSECHO false

File myFile;

uint32_t timer = millis();
float mph;
float battery;
const int chipSelect = 4;
char fileName[] = "xxxxxxxx.CSV";
char extension[] = "CSV";
double temporaryLat;
double temporaryLon;
double currentDegreesLat;
double currentDegreesLon;
void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) {
    //; // wait for serial port to connect. Needed for native USB port only
  //}


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  GPS.begin(9600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  GPSSerial.println(PMTK_Q_RELEASE);
}

void loop() {
  char c = GPS.read();
  
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) timer = millis();
     
  // approximately every 1 second, print out the current stats
  if (millis() - timer > 900) {
    timer = millis(); // reset the timer
    if (GPS.fix) {
      calculateCurrentLocation();
      Serial.print("\nDate: ");
      Serial.print(GPS.month, DEC); Serial.print('/');
      Serial.print(GPS.day, DEC); Serial.print("/20");
      Serial.println(GPS.year, DEC);
      Serial.print("Time: ");
      Serial.print(GPS.hour, DEC); Serial.print(':');
      Serial.print(GPS.minute, DEC); Serial.print(':');
      Serial.print(GPS.seconds, DEC); Serial.print('.');
      Serial.println(GPS.milliseconds);
      Serial.print("Location: ");
      Serial.print(currentDegreesLat, 10); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(currentDegreesLon, 10); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      mph = (GPS.speed)*1.15078;
      Serial.print("Speed (mph): "); Serial.println(mph);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      Serial.print("Fix: "); Serial.print((int)GPS.fix);
      Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
      battery = (analogRead(A9)*(3.3/1023)*2);
      Serial.print("Battery: "); Serial.println(battery);
      // if the file is available, write to it:

      if (fileName) {
        sprintf(fileName, "%02d%02d%02d.csv", GPS.year, GPS.month, GPS.day);
        myFile = SD.open(fileName, FILE_WRITE);
        digitalWrite(13, HIGH);
        myFile.print(GPS.month, DEC); myFile.print('/');
        myFile.print(GPS.day, DEC); myFile.print("/20");
        myFile.println(GPS.year, DEC);
        myFile.print(GPS.hour, DEC); myFile.print(':');
        myFile.print(GPS.minute, DEC); myFile.print(':');
        myFile.println(GPS.seconds, DEC);
        myFile.print(currentDegreesLat, 10); myFile.print(GPS.lat);
        myFile.print(", ");
        myFile.print(currentDegreesLon, 10); myFile.println(GPS.lon);
        myFile.println(mph);
        myFile.println(battery);
        Serial.println("Written");
        myFile.close();
        digitalWrite(13, LOW);
      }
      // if the file isn't open, pop up an error:
      else {
        Serial.println("error opening gpslog.txt");
      }
    }
  }
}
void calculateCurrentLocation(){ 
  double minutesLat; 
  double minutesLong;
  double degreesLat; 
  double degreesLong; 
  double secondsLat; 
  double secondsLong; 
  double millisecondsLat; 
  double millisecondsLong; 

  temporaryLat = (GPS.latitude);
  temporaryLon = (GPS.longitude);
  degreesLat = trunc(temporaryLat/100);
  degreesLong = trunc(temporaryLon/100);
  minutesLat = temporaryLat - (degreesLat*100);
  minutesLong = temporaryLon - (degreesLong*100);
  secondsLat = (minutesLat - trunc(minutesLat)) * 60;
  secondsLong = (minutesLong - trunc(minutesLong)) * 60;
  millisecondsLat = (secondsLat - trunc(secondsLat)) * 1000;
  millisecondsLong = (secondsLong - trunc(secondsLong)) * 1000;
  
  minutesLat = trunc(minutesLat);
  minutesLong = trunc(minutesLong);
  secondsLat = trunc(secondsLat);
  secondsLong = trunc(secondsLong);

  currentDegreesLat = degreesLat + minutesLat/60 + secondsLat/3600 + millisecondsLat/3600000; 
  currentDegreesLon = degreesLong + minutesLong/60 + secondsLong/3600 + millisecondsLong/3600000; 
  //lat = (currentDegreesLat, 10);
  Serial.print("Minute lat: "); 
  Serial.println(currentDegreesLat, 10);
}
