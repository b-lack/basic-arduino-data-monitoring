/*
basic temperature and humidity monitoring
*/

//------library

// include the RealtimeClock library:
#include <Wire.h>
#include <DS3231.h>

// include the SD library:
#include <SPI.h>
#include <SD.h>

// include the DHT library:
#include <SimpleDHT.h>

//------variables

// for DHT11, 
const int pinDHT11 = 2;
SimpleDHT11 dht11;

// for SD utility library functions
File dataFile;
const int chipSelect = 4;

// for clock
DS3231 clock;
RTCDateTime dt;

// 0=nothing; 1=read sd-card
int state = 0;

// write data every x seconds
const int interval = 60;

// start timer at x seconds
int timer = 60;

//------setup code

void setup() {
  Serial.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("{\"error\":\"1\", \"msg\":\"SD-Card initialization failed!\"}");
  }

  clock.begin();
  // Set sketch compiling time
  clock.setDateTime(__DATE__, __TIME__);

  Serial.println("{\"msg\":\"initialization done.\"}");
}

//------main code

void loop() {

  dt = clock.getDateTime();

  if(Serial.available()>0){
    state = Serial.read();
  }
  if(state == '1'){
    getDump();
    
    //reset state
    state = 0;
  }

  dataFile = SD.open("data.txt", FILE_WRITE);

  if (dataFile) {
    if(timer>=interval){

      //--- read with raw data.
      byte temperature = 0;
      byte humidity = 0;
      byte data[40] = {0};
      if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
        Serial.println("{\"error\":\"1\", \"msg\":\"Read DHT11 failed\"}");
        return;
      }

      //--- write data to file in .json like style
      dataFile.print("{");
      
      //--clock data
      //dataFile.print("\"ts\":\"" + String(dt.year) + "_" +  String(dt.month) + "_" + String(dt.day) + "_" + String(dt.hour) + "_" + String(dt.minute) + "_" + String(dt.second) + "\",");
      dataFile.print("\"ty\":\"" + String(dt.year) + "\",");
      dataFile.print("\"tmo\":\"" +  String(dt.month) + "\",");
      dataFile.print("\"td\":\"" + String(dt.day) + "\",");
      dataFile.print("\"th\":\"" + String(dt.hour) + "\",");
      dataFile.print("\"tmi\":\"" + String(dt.minute) + "\",");
      dataFile.print("\"ts\":\"" + String(dt.second) + "\",");

      //--clock temperature & humidity
      dataFile.print("\"d\":\"1\",");
      dataFile.print("\"t\":\"" + String((int)temperature) + "\",");
      dataFile.print("\"h\":\"" + String((int)humidity) + "\"");
      
      dataFile.print("}");dataFile.println("");

      Serial.println("{\"msg\":\"successfully written to SD-Card\", \"update\":1}");

      //reset Timer
      timer = 0;
    }
    
    dataFile.close();
    
    timer++;
    
  }else{
    Serial.println("{\"error\":\"1\", \"msg\":\"cannot read file\"}");  
  }

  
  // DHT11 sampling rate is 1HZ. 1000 gleich 1 Sekunde
  delay(1000);
}

void getDump() {
  Serial.println("{\"msg\":\"Reading SD-card\"}");
    
  File dumpFile = SD.open("data.txt");

  if (dumpFile){
      
      while (dumpFile.available()){
          Serial.write(dumpFile.read());
      }
      
      dumpFile.close();
      
      Serial.println("{\"msg\":\"Read: success\", \"upload\":1}");
      
  }else{
    Serial.println("{\"error\":\"1\", \"msg\":\"cannot read file\"}");  
  }
  
}
