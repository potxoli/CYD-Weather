#include <WiFi.h>  // 
#include <TFT_eSPI.h>
#include <ArduinoJson.h> // 7.1.0
#include <HTTPClient.h> // https://github.com/arduino-libraries/ArduinoHttpClient   version 0.6.1
#include <ESP32Time.h>  // https://github.com/fbiego/ESP32Time  verison 2.0.6
#include "NotoSansBold15.h"
#include "tinyFont.h"
#include "smallFont.h"
#include "midleFont.h"
#include "bigFont.h"
#include "font18.h"
#include "config.h" // config file with API and other settings
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite AnimationSprite = TFT_eSprite(&tft);
ESP32Time rtc(0);


//#################### CONFIG STUFF  ###################
//time zone  
//int zone = TIME_ZONE;
String timezoneString = TIMEZONE_STRING;

String town = TOWN;
String latitude = LATITUDE;
String longitude = LONGITUDE;
String myAPI = MY_API_KEY;
String units = UNITS;  //  metric, imperial
const char* ssid = SSID;
const char* password = PASSWORD;
//#################### end of config stuff ###################


const char* ntpServer = "pool.ntp.org";
//String server = "https://api.openweathermap.org/data/2.5/weather?q=" + town + "&appid=" + myAPI + "&units=" + units;
String server = "https://api.openweathermap.org/data/3.0/onecall?lat=" + latitude + "&lon=" + longitude +"&exclude=minutely,hourly,alerts" + "&appid=" + myAPI + "&units=" + units;


//additional variables
int ani = 100;
float maxT;
float minT;
float dailyMin;
float dailyMax;
unsigned long timePased = 0;
int counter=0;

//................colors
#define bck TFT_BLACK
unsigned short grays[13];

// static strings of data showed on right side 
char* PPlbl[] = { "HUM", "RAIN", "WIND" };
String PPlblU[] = { "%", "%", "m/s" };

//data that changes
float temperature = 22.2;
float wData[3];
float PPpower[24] = {};    //graph
float PPpowerT[24] = {};   //graph
int PPgraph[24] = { 0 };   //graph
char sunsetString[6];
char sunriseString[6];

//scroling message on bottom right side
String Wmsg = "";

void setTime() {
  // Use POSIX timezone string for automatic DST handling
  configTzTime(timezoneString.c_str(), ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  }
}

void getData() {
  time_t sunrise;
  time_t sunset;
  struct tm sunriseTime;
  struct tm sunsetTime;

  HTTPClient http;

  http.begin(server);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.println("server response");
    Serial.println(payload);

    // Parsiranje JSON odgovora
    //StaticJsonDocument<1024> doc;
    StaticJsonDocument<8192> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      temperature = doc["current"]["temp"];
      wData[0] = doc["current"]["humidity"];
      //wData[1] = doc["current"]["pressure"];
      wData[1] = doc["daily"][0]["pop"]; // Probability of Rain
      wData[1] = wData[1] * 100;
      wData[2] = doc["current"]["wind_speed"];

      dailyMax = doc["daily"][0]["temp"]["max"];
      dailyMin = doc["daily"][0]["temp"]["min"];


      int visibility = doc["current"]["visibility"];
      visibility = visibility / 1000;
      //const char* description = doc["current"]["weather"][0]["description"];     
      const char* summary = doc["daily"][0]["summary"];
      long dt = doc["current"]["dt"];

      //Wmsg = "#Description: " + String(description) + "  #Visbility: " + String(visibility) + " #Updated: " + rtc.getTime();
      Wmsg = String(summary) + "  #Visbility: " + String(visibility) + "km" + " #Updated: " + rtc.getTime();

      // Show Weather and temperature on serial monitor
      Serial.print(Wmsg);
      Serial.print("\nTemperature: ");
      Serial.print(temperature);

      // Get Sunrise and Sunset
      sunrise = doc["current"]["sunrise"];
      sunset = doc["current"]["sunset"];
      localtime_r(&sunrise, &sunriseTime);
      localtime_r(&sunset, &sunsetTime);
      strftime(sunriseString,sizeof(sunriseString),"%H:%M",&sunriseTime);
      strftime(sunsetString,sizeof(sunsetString),"%H:%M",&sunsetTime);

      Serial.print("Sunrise: ");
      Serial.print(sunriseString);

      Serial.print("Sunset: ");
      Serial.print(sunsetString);


    } else {
      Serial.print("ERROR JSON-a: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP ERROR ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


void setup() {

  // using this board can work on battery
  //pinMode(15,OUTPUT);
  //digitalWrite(15,1);
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Connecting to WIFI!!",30,50,4);
  sprite.createSprite(320, 170);
  AnimationSprite.createSprite(220,20);


  //set brightness
  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  ledcWrite(0, 130);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());

  setTime();
  getData();

  // generate 13 levels of gray
  int co = 210;
  for (int i = 0; i < 13; i++) {
    grays[i] = tft.color565(co, co, co);
    co = co - 20;
  }
}


void draw() {

  AnimationSprite.fillSprite(grays[10]);
  AnimationSprite.loadFont(font18);
  AnimationSprite.setTextColor(grays[1], grays[10]);
  AnimationSprite.drawString(Wmsg, ani, 4);
  AnimationSprite.unloadFont();

  sprite.fillSprite(TFT_BLACK);
  sprite.drawLine(138, 10, 138, 164, grays[6]);
  sprite.drawLine(100, 108, 134, 108, grays[6]);
  sprite.setTextDatum(0);

  //LEFTSIDE
  sprite.loadFont(midleFont);
  sprite.setTextColor(grays[1], TFT_BLACK);
  sprite.drawString("WEATHER", 6, 10);
  sprite.unloadFont();

  sprite.loadFont(font18);
  sprite.setTextColor(grays[7], TFT_BLACK);
  sprite.drawString("TOWN:", 6, 110);
  sprite.setTextColor(grays[2], TFT_BLACK);
  if (units == "metric")
    sprite.drawString("C", 14, 50);
  if (units == "imperial")
    sprite.drawString("F", 14, 50);

 
  sprite.setTextColor(grays[3], TFT_BLACK);
  sprite.drawString(town, 46, 110);
  sprite.fillCircle(8, 52, 2, grays[2]);
  sprite.unloadFont();

  // draw wime without seconds
  sprite.loadFont(tinyFont);
  sprite.setTextColor(grays[4], TFT_BLACK);
  sprite.drawString(rtc.getTime().substring(0, 5), 6, 132);
  sprite.unloadFont();

  // draw some static text
  sprite.setTextColor(grays[5], TFT_BLACK);
  sprite.drawString("INTERNET", 86, 10);
  sprite.drawString("STATION", 86, 20);
  sprite.setTextColor(grays[7], TFT_BLACK);
  sprite.drawString("SECONDS", 92, 157);

  // draw temperature
  sprite.setTextDatum(4);
  sprite.loadFont(bigFont);
  //sprite.setTextColor(grays[0], TFT_BLACK);
  sprite.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  sprite.drawFloat(temperature, 1, 69, 80);
  sprite.unloadFont();


  //draw sec rectangle
  sprite.fillRoundRect(90, 132, 42, 22, 2, grays[2]);
  //draw seconds
  sprite.loadFont(font18);
  sprite.setTextColor(TFT_BLACK, grays[2]);
  sprite.drawString(rtc.getTime().substring(6, 8), 111, 144);
  sprite.unloadFont();


  sprite.setTextDatum(0);
  //RIGHT SIDE
  sprite.loadFont(font18);
  sprite.setTextColor(grays[1], TFT_BLACK);
  //sprite.drawString("LAST 12 HOUR", 144, 10);
  sprite.drawString("MIN: " + String(dailyMin) + "   MAX: " + String(dailyMax),144,10);
  sprite.unloadFont();

  sprite.fillRect(144, 28, 84, 2, grays[10]);

  sprite.setTextColor(grays[3], TFT_BLACK);
  //sprite.drawString("MIN:" + String(minT), 254, 10);
  //sprite.drawString("MAX:" + String(maxT), 254, 20);
  sprite.fillSmoothRoundRect(144, 34, 174, 60, 3, grays[10], bck);
  sprite.drawLine(170, 39, 170, 88, TFT_WHITE);
  sprite.drawLine(170, 88, 314, 88, TFT_WHITE);

  sprite.setTextDatum(4);

  for (int j = 0; j < 24; j++)
    for (int i = 0; i < PPgraph[j]; i++)
      sprite.fillRect(173 + (j * 6), 83 - (i * 4), 4, 3, grays[2]);

  sprite.setTextColor(grays[2], grays[10]);
  sprite.drawString("MAX", 158, 42);
  sprite.drawString("MIN", 158, 86);

  sprite.loadFont(font18);
  sprite.setTextColor(grays[7], grays[10]);
  sprite.drawString("T", 158, 58);
  sprite.unloadFont();


  for (int i = 0; i < 3; i++) {
    sprite.fillSmoothRoundRect(144 + (i * 60), 100, 54, 32, 3, grays[9], bck);
    sprite.setTextColor(grays[3], grays[9]);
    sprite.drawString(PPlbl[i], 144 + (i * 60) + 27, 107);
    sprite.setTextColor(grays[2], grays[9]);
    sprite.loadFont(font18);
    sprite.drawString(String((int)wData[i])+PPlblU[i], 144 + (i * 60) + 27, 124);
    sprite.unloadFont();

    sprite.fillSmoothRoundRect(144, 148, 174, 16, 2, grays[10], bck);
    //errSprite.pushToSprite(&sprite, 148, 150);
  }
  //sprite.setTextColor(grays[4], bck);
  //sprite.drawString("CURRENT WEATHER", 190, 141);
  //sprite.setTextColor(grays[9], bck);
  //sprite.drawString(String(counter), 310, 141);

  // Sunrise and Sunset
  sprite.loadFont(font18);
  sprite.setTextColor(grays[4], bck);
  sprite.drawString("SUNRISE " + String(sunriseString) + " SUNSET " + String(sunsetString), 230, 151);
  //sprite.drawString("Sunset: " + String(sunsetString), 190, 161);
  sprite.unloadFont();

  //FinalRender
  sprite.pushSprite(0, 15);
  AnimationSprite.pushSprite(50,200);
}

void updateData() {

  //update alsways
  //part needed for scroling weather msg
  ani--;
  if (ani < -420)
    ani = 100;


  //

  if (millis() > timePased + 300000) {
    timePased = millis();
    counter++;
    getData();

    if (counter==10) {
       setTime();
       counter=0;
       maxT = -50;
       minT = 1000;
      PPpower[23] = temperature;
      for (int i = 23; i > 0; i--)
        PPpower[i - 1] = PPpowerT[i];

      for (int i = 0; i < 24; i++) {
        PPpowerT[i] = PPpower[i];
        if (PPpower[i] < minT) minT = PPpower[i];
        if (PPpower[i] > maxT) maxT = PPpower[i];
      }
      if (minT == maxT) maxT = maxT + 0.1;
      
      for (int i = 0; i < 24; i++) {
        PPgraph[i] = map(PPpower[i], minT, maxT, 0, 12);
      }
    }
  }
}

void loop() {

  updateData();
  draw();
}
