//The code is licensed under GNU GPLv3.

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "globals.h"

String SgetDrawGraph(int reading);

String SgetCurrentText() {
  String a = String("");
  String hhr  = (hr>=10) ?String(hr) :(String(0)+hr);
  String mmin = (min>=10)?String(min):(String(0)+min);
  String ssec = (sec>=10)?String(sec):(String(0)+sec);
  return a + "<p>Startup time: <strong>"+ startUpTime +"</strong> Uptime: <strong>" + hhr + ":" + mmin + ":" + ssec + "</strong>,<small> Device Model: ESP8266MOD, Device Vendor: AI-THINKER, Sensor1 Model: SparkFun HTU21D, Sensor2 Model: SparkFun BMP280</small></p>"
  "<p>Current Average Temperature: <strong class=\"reading\">" + (currTemp+currTemp2)/2 + units[0] + "</strong>, "
  "Current Humidity: <strong class=\"reading\">" + currHumi + units[1] + "</strong>, "
  "Current Pressure: <strong class=\"reading\">" + (currPres/100.0f) + units[3] +"</strong></p>";
}

String SgetLayout() {
  return "<!DOCTYPE html>"
  "<html>"
  "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>"
    "<meta charset=\"utf-8\"/>"
    "<script type=\"text/javascript\" src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js\"></script>"
    "<script type=\"text/javascript\" src=\"/res/script.js\"></script>"
    "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" integrity=\"sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7\" crossorigin=\"anonymous\">"
    "<link href=\"http://maxcdn.bootstrapcdn.com/font-awesome/4.5.0/css/font-awesome.min.css\" rel=\"stylesheet\">"
    "<link rel=\"stylesheet\" href=\"/res/style.css\"/>"
  "</head>"
  "<body>"
   "<div class=\"container\">"
    "<div class=\"row\" id=\"header1\">"
     "<h2>Welcome to the WebUI of the weather station in my room.</h2>"
     "<div id=\"stat\">"+SgetCurrentText()+"</div>"
    "</div>"
    "<div id=\"log\" class=\"row bg-warning\">"
     "<p>No errors encountered.<p>"
    "</div>"
    "<div class=\"row\" id=\"graph-grid\">"
     "<div class=\"col-xs-12 col-sm-6 col-md-6 col-lg-4 nopadding\" id=\"temp\"></div>"
     "<div class=\"col-xs-12 col-sm-6 col-md-6 col-lg-4 nopadding\" id=\"humi\"></div>"
     "<div class=\"col-xs-12 col-sm-6 col-md-6 col-lg-4 nopadding\" id=\"pres\"></div>"
    "</div>"
   "</div>"
  "</body>"
  "</html>";
}

String SgetDrawGraph(int reading) { //TODO: refactor
  String out = "";
  char sline[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 600 300\" >\n";
  out += "<rect width=\"600\" height=\"300\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";

// poszczegolne wyniki
    out += "<g stroke=\"" + (String)colors[reading] + "\">\n";
    std::list<std::vector<float> >::iterator tempIterator;
    int i = 0;
    float yprevious = 300.0;
    char y0S[7];
    char y2S[7];
    for (tempIterator = History.begin(); tempIterator != History.end(); ++tempIterator) {
      int x1 = 10 * i;
      int x2 = x1 + 10;
      //float y2 = 300 - 3 * (*tempIterator).at(reading);
      float y2 = (-300.0f/(Statistics.at(0).at(reading)+2*deltas.at(reading)-Statistics.at(1).at(reading))*(*tempIterator).at(reading) +
          ((Statistics.at(0).at(reading)+deltas.at(reading))*300.0f/(Statistics.at(0).at(reading)+2*deltas.at(reading)-Statistics.at(1).at(reading))));

      dtostrf(yprevious, 4, 2, y0S);
      dtostrf(y2, 4, 2, y2S);
      sprintf(sline, "<line x1=\"%d\" y1=\"%s\" x2=\"%d\" y2=\"%s\" stroke-width=\"1\" />\n", x1, y2S, x2, y2S);
      out += sline;
      i += 1;
      yprevious = y2;
    }
    out += "</g>\n";
//
  char tline[100];
  sprintf(tline, "<text x=\"%d\" y=\"%s\" fill=\"%s\">%s</text>", (int)((30-600)/(-1*60.0)*i), y2S, colors[reading], cCS[reading]);
  out += tline;
  out += "</svg>\n";

  return out;
}

String SgetTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");

  while(!!!client.available()) {
     yield();
  }

  while(client.available()){
    if (client.read() == '\n') {
      if (client.read() == 'D') {
        if (client.read() == 'a') {
          if (client.read() == 't') {
            if (client.read() == 'e') {
              if (client.read() == ':') {
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}// - See more at: http://www.esp8266.com/viewtopic.php?f=29&t=6007&start=5#sthash.yK4yygBY.dpuf

String precUptimePrint(long a) {
  return String(a/1000)+"."+(a%1000);
}

String debug() {
  return String("[")+precUptimePrint(millis())+"] : ";
}

void setupAP() {
  WiFi.mode(WIFI_AP_STA);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 3], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  String AP_NameString = "WeatherStation " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, "123456789");
  Serial.print(debug()+"Set a softAP under the IP Address ");
  Serial.println(WiFi.softAPIP());
  Serial.println(debug()+" > >  softAP MAC address:"+WiFi.softAPmacAddress());
}

void setupLocal() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  int attempts = 0;
  while (!(WiFi.status() != WL_CONNECTED) || (attempts<20)) {//while (true)
    delay ( 50 );
    Serial.print ( "." );
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println ( "" );
    Serial.print (debug()+"Connected to " );
    Serial.println ( ssid );
    Serial.print (debug()+"IP address: " );
    Serial.println ( WiFi.localIP() );
  } else {
    Serial.println("\n"+debug()+"Local connection not established");
  }
}

#endif

