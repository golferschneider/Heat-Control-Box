
/*
 * created november 27 2015
 * by Kyle Schneider
 */


int timeSec = 0;
int timeHour = 0;
//for SD card
#include <SPI.h>
#include <SD.h>

//for pressure sensor
#include <SFE_BMP180.h>
#include <Wire.h>
SFE_BMP180 pressure;
double baseline;
#define ALTITUDE 25.0 //the willy field, McMurdo Station(bollon launch) is at about sea level

 const int chipSelect = 10;

 //for heater
 const int heaterPin = 3;
 double lowTemp = 22.0;
 
void setup() {
    // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // this includes erasing the data on card
 // Serial.print("Initializing SD car...");
  pinMode(heaterPin, OUTPUT);
  //see if the card is present and can initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
//deletes DATALOGGER file if the file is already there
//Serial.println("Removing example.txt...");
//  SD.remove("datalog.txt");
  
//see if the pressure sensor is working
 if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail (disconnected?)\n\n");
    while(1); // Pause forever.
  }
}

/////////////////////////////////////////////////end setup//////////////////////////////////////////

void loop() {
  float temp = getVoltage(A0);
  float fahrenheit = (((temp - .5) * 100.0) * 1.8) +32.0;
  double tempSensor;
  char status;
  double T,P,p0,a;
  float celcus;
  
  /////////////////loggin data////////////////////////
  if(timeSec >= 10){
    timeSec = 0;
    timeHour++;
    /////////just for temp/////////////
    

 ////////////////////////////////////////////////////////for the pressure/altitude sensor////////////////////////////////////////////////////////////////////
 char status;
  double T,P,p0,a;
 
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    pressure.getTemperature(tempSensor);
    if (status != 0)
    {     
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P,p0);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
////////////////////////////////////////////////////////end pressure/altitude sensor////////////////////////////////////////////////////////////////////

     String dataString = "Hour " + String(timeHour) + ": temp=" + String(T,3) + " pressure=" + String(P,5) + "mb";
  Serial.println(dataString);
    
    ///////////adding data to card////////////////
    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
   dataFile.println(dataString);
   Serial.println("adding data to card");
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }


  Serial.println(T,3);
 if(tempSensor < lowTemp){
    digitalWrite(heaterPin, HIGH);
    lowTemp = 22.0;
    //lowTemp = 25.0;
    Serial.print(tempSensor);
    Serial.print(" ");
    Serial.print(lowTemp);
    dataFile.println("hot plate on");
    Serial.println("hot plate on"); 
  }else{
    digitalWrite(heaterPin, LOW);
    lowTemp = 15.5;
    //lowTemp = 23.8;
     dataFile.println("hot plate off");
    Serial.println("hot plate not on");
  }
  dataFile.close();
 }
 
 ////////////////////end logging data//////////////////////
  //deals with temperature and turning on and off the heater

   //this delays reading every 10 seconds
  timeSec = timeSec + 10;
 delay(10000);
}

float getVoltage(int pin){
    return (analogRead(pin) * .004882614);
  }
