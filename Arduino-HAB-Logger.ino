#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345.h>
#include <SdFat.h>
#include <SdFatUtil.h> 

Adafruit_ADXL345 accel = Adafruit_ADXL345(12345);

SdFat sd;
ofstream logfile;

#define error(s) sd.errorHalt_P(PSTR(s))
// Serial print stream
ArduinoOutStream cout(Serial);

#define printserial true
#define logtofile true

unsigned long currentTime;
unsigned long loopTime;
int beepcounter = 0;
int alert_alt = 1000;
int alert_beep = 5;

TinyGPS gps;
SoftwareSerial ss(7, 8);

void setup()
{
  currentTime = millis();
  loopTime = currentTime;  
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  ss.begin(4800);
  Serial.println("Starting logger"); 
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

  if (logtofile){
    // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
    if (!sd.begin(10, SPI_HALF_SPEED)) sd.initErrorHalt();

    // create a new file in root, the current working directory
    char name[] = "LOGGER00.CSV";

    for (uint8_t i = 0; i < 100; i++) {
      name[6] = i/10 + '0';
      name[7] = i%10 + '0';
      if (sd.exists(name)) continue;
      logfile.open(name);
      break;
    }
    if (!logfile.is_open()) error("file.open");

    cout << pstr("Logging to: ") << name << endl;
  }
}

void loop()
{
  bool newData = false;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long fix_age;
    gps.f_get_position(&flat, &flon, &fix_age);
    float falt = gps.f_altitude();
    float fkmph = gps.f_speed_kmph();
    float fc = gps.f_course();
    int sats = gps.satellites();
    int year;
    byte month, day, hour, minute, second, hundredths;
    gps.crack_datetime(&year, &month, &day,
    &hour, &minute, &second, &hundredths, &fix_age);
    /* Get a new sensor event */
    sensors_event_t event; 
    accel.getEvent(&event);
    float xaccel = event.acceleration.x*8;
    float yaccel = event.acceleration.y*8;
    float zaccel = event.acceleration.z*8;
    currentTime = millis();
    if(currentTime >= (loopTime + 4000)){  //~5 delay


      if (printserial) {
        if (logtofile){
          Serial.print("Logging: ");
        }
        //Date
        Serial.print(day);
        Serial.print("/");
        Serial.print(month);
        Serial.print("/");
        Serial.print(year);
        Serial.print(",");
        //Time in UTC
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.print(second);
        Serial.print(",");
        //Latitude
        Serial.print(flat,6);
        Serial.print(",");
        //Longitude
        Serial.print(flon,6);
        Serial.print(",");
        //Altitude
        Serial.print(falt,1);
        Serial.print(",");
        //Speed km/hr
        Serial.print(fkmph,2);
        Serial.print(",");
        //Sats
        Serial.print(sats);
        Serial.print(",");
        Serial.print(xaccel,2); 
        Serial.print(",");
        Serial.print(yaccel,2); 
        Serial.print(",");
        Serial.print(zaccel,2); 
        Serial.print("\n");
      }
      if (logtofile){
        //Date,time
        logfile << (int)day << "/"; 
        logfile << (int)month << "/";
        logfile << (int)year << ",";

        if (hour < 10){
          logfile << "0" <<(int)hour << ":";
        }
        else {
          logfile << int(hour) << ":";
        }

        if (minute < 10) {
          logfile << "0" << int(minute) << ":";
        }
        else {
          logfile << int(minute) << ":";
        }

        if (second < 10) {
          logfile << "0" << int(second) << ","; 
        }
        else {
          logfile << int(second) << ",";
        }
        //Lat/lon
        logfile << setprecision(6) << flat << ",";
        logfile << setprecision(6) << flon << ",";
        //Alt
        logfile << setprecision(2) << falt << ",";
        //Speed 
        logfile << setprecision(2) << fkmph << ",";
        //Sats
        logfile << int(sats) << ",";
        //XYZ
        logfile << setprecision(2) << xaccel << ",";
        logfile << setprecision(2) << yaccel << ",";
        logfile << setprecision(2) << zaccel; 
        logfile << endl << flush;
      }
      loopTime = currentTime;
      //check for beeping
      Serial.println(beepcounter);
      if (falt < alert_alt && beepcounter > alert_beep) {
        Serial.println("BEEP ON"); 
        beep_on();
        beepcounter=0;
      }
      else {
        Serial.println("BEEP OFF"); 
        beep_off(); 
        beepcounter++;
      } 
    }

  }

}

void beep_on(){
  analogWrite(2,255);
}
void beep_off(){
  analogWrite(2,0);
}







