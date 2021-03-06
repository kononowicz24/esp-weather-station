//The code is licensed under GNU GPLv3.

//#include <Adafruit_HTU21DF.h>
#include <SparkFunHTU21D.h>
#include <Wire.h>
#include "i2c.h"

#include <SPI.h>
#include "RF24.h"

#include "i2c_BMP280.h"

#include <list>
#include <vector>
//#include <sstream>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern "C" {
  #include "user_interface.h"
}

#include "globals.h"
#include "resources.h"

#include "FS.h"

//os_timer_t readingTimer;
long long lastReadingMillis;

HTU21D mierniczek;
BMP280 bmp280;
RF24 radio(D4,D8);

ESP8266WebServer server(80);



void calculateStats() {
  std::vector<float> maxima (ILOSC_POMIAROW, -10000000.0f);
  std::vector<float> minima (ILOSC_POMIAROW, 250000000.0f);
  std::vector<float> avgima (ILOSC_POMIAROW,   0);

  for (int i=0; i<ILOSC_POMIAROW; ++i) {
  float cumulative = 0;
  int  callsnumber = 0;
    for (std::list<std::vector<float> >::iterator it1=History.begin(); it1!=History.end(); ++it1) {
      if((*it1).at(i)>maxima.at(i)) { maxima[i] = (*it1).at(i);}
      if((*it1).at(i)<minima.at(i)) { minima[i] = (*it1).at(i);}
      cumulative+=((*it1).at(i));
      ++callsnumber;
    }
    avgima[i] = cumulative/callsnumber;
  }
  Statistics[0] = maxima;
  Statistics[1] = minima;
  Statistics[2] = avgima;
}

void reading() {
  //os_intr_lock();
  lastReadingMillis = millis();
  sec = millis() / 1000;
  min = sec / 60;
  hr = min / 60;
  sec%=60;
  min%=60;
  currTemp = mierniczek.readTemperature();
  currHumi = mierniczek.readHumidity();
  //currPres = bme.readPressure();
  //currTemp2 = bme.readTemperature();
  bmp280.awaitMeasurement();
  bmp280.getPressure(currPres);
  bmp280.triggerMeasurement();
  bmp280.awaitMeasurement();
  bmp280.getTemperature(currTemp2);
  bmp280.triggerMeasurement();
  if (History.size()>=59) {
    History.pop_front();
  }
  cC[0] = currTemp;
  cC[1] = currHumi;
  cC[2] = lastReadingMillis/10/100.0f;
  cC[3] = currPres/100.0f;
  cC[4] = currTemp2;
  for (int i=0;i<ILOSC_POMIAROW;i++) {
    dtostrf(cC[i], 6, 2, cCS[i]);
  }

  History.push_back(std::vector<float>(cC, cC + sizeof(cC)/sizeof(float)));
  calculateStats();
}

unsigned int i=0;
void respondTime() {
  if( radio.available()){
      query request;                                                              // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &request, sizeof(query) );             // Get the payload
      }

      radio.stopListening();// First, stop listening so we can talk
      query okResponse; okResponse.co='o';
      Serial.print(debug()+"Sending response ");
      Serial.print(okResponse.co);
      radio.write( &okResponse, sizeof(query) );              // Send the final one back.
      Serial.println(debug()+" Sent response ");
      
      query okResult; okResult.co='c';
      String currTempDateTime = SgetTime();

      strncpy(okResult.date, getDateFromString(currTempDateTime).c_str(), sizeof(okResult.date));
      okResult.date[sizeof(okResult.date) - 1] = 0;
      strncpy(okResult.hour, getTimeFromString(currTempDateTime).c_str(), sizeof(okResult.hour));
      okResult.hour[sizeof(okResult.hour) - 1] = 0;
      
      Serial.print(debug()+"Sending response ");
      Serial.print(okResult.co);
      Serial.print(okResult.date);
      Serial.print(okResult.hour);
      
      radio.write( &okResult, sizeof(query) );
      radio.startListening();                                       // Now, resume listening so we catch the next packets.
      Serial.println(debug()+" Sent response ");i++;
   }
}

void setupMeters() {
  mierniczek.begin();
  Serial.println(debug()+"HTU21D found and working.");
  if (bmp280.initialize()) Serial.println(debug()+"BMP280 found and working.");
    else
    {
        Serial.println(debug()+"BMP280 is missing or the wiring is incorrect.");
        while (1) {}
    }

    // onetime-measure:
    bmp280.setEnabled(0);
    bmp280.triggerMeasurement();

  Statistics.push_back(std::vector<float>(ILOSC_POMIAROW,250000000.0f)); //max
  Statistics.push_back(std::vector<float>(ILOSC_POMIAROW,-15000000.0f)); //min
  Statistics.push_back(std::vector<float>(ILOSC_POMIAROW,0.0)); //avg
  Statistics.push_back(std::vector<float>(ILOSC_POMIAROW,0.0)); //avg(1h)
  Statistics.push_back(std::vector<float>(ILOSC_POMIAROW,0.0)); //n
  deltas.push_back(1.0f);
  deltas.push_back(10.0f);
  deltas.push_back(10.0f);
  deltas.push_back(1.5f);
  deltas.push_back(1.0f);
}

void setup ( void ) {
  Serial.begin ( 115200 );
  Serial.println(debug()+"System start");
  WiFi.mode(WIFI_OFF);
  //if (!
  setupLocal();
  setupAP();
  /*  Serial.print (debug()+"Connected to " );
    Serial.println ( ssid );
    Serial.print (debug()+"IP address: " );
    Serial.println ( WiFi.localIP() );
  }*/

  if ( MDNS.begin ( "weatherstation" ) ) {
    Serial.println (debug()+"MDNS responder started." );
  }
  setupMeters();
  if (SPIFFS.begin()) Serial.println(debug()+"FS mounted successfully.");

  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();
  Serial.println(debug()+"RF24 is ready to respond.");
  
  
  startUpTime = SgetTime();

  Serial.println(getDateFromString(startUpTime).c_str());
  Serial.println(getTimeFromString(startUpTime).c_str());

  server.on ( "/", []() {
    Serial.println(debug()+"Requested /");
    server.send(200, "text/html", SgetLayout());  
  });
  server.on ( "/temp.svg", []() {drawGraph(0);});
  server.on ( "/humi.svg", []() {drawGraph(1);} );
  server.on ( "/pres.svg", []() {drawGraph(3);} );
  server.on ( "/stat.txt", []() {
    Serial.println(debug()+"Requested stat.txt");
    server.send(200, "text/html", SgetCurrentText());
  });
  server.serveStatic("/res", SPIFFS, "/res","max-age=86400");

  
  server.begin();
  Serial.println (debug()+"HTTP server started." );
  Serial.println (debug()+"Done setup.\n\n" );
}

void drawGraph(int meter) {
  Serial.println(debug()+"Requested drawGraph("+meter+")");
  server.send ( 200, "text/html", SgetDrawGraph(meter));
}

void loop ( void ) {
  server.handleClient();
  if (millis() > lastReadingMillis + RESOLUTION_TEMP) reading();
  respondTime();
}




