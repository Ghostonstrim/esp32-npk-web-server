#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <SoftwareSerial.h>
//
#define RE 32
#define DE 33

const byte nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
//
byte values[11];
SoftwareSerial mod(26,27);

const char* ssid = "MALUPONTON";
const char* password = "Paulette20";

const int verde = 34;
const int blanco = 35;
const int rojo = 14;
const int amarillo = 12;
#define DHTPIN 15     // Digital pin connected to the DHT sensor

#define DHTTYPE    DHT11     // DHT 11

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
String header; // Variable para guardar el HTTP request
String estadoSalida = "off";
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
AsyncWebServer server(80);

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readDN() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  byte n = nitrogen();

  Serial.println(n);
  return String(n);

}
String readDP() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  byte p = phosphorous();

  Serial.println(p);
  return String(p);

}
String readDK() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  byte k = potassium();

  Serial.println(k);
  return String(k);

}
// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY") {
    return readDHTHumidity();
  }
  else if (var == "NITROGENO") {
    return readDN();
  }
  else if (var == "FOSFORO") {
    return readDP();
  }
  else if (var == "POTASIO") {
    return readDK();
  }
  return String();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mod.begin(9600);
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("");
  dht.begin();

  pinMode(verde, OUTPUT);
  pinMode(blanco, OUTPUT);
  pinMode(rojo, OUTPUT);
  pinMode(amarillo, OUTPUT);
  digitalWrite(verde, LOW);
  digitalWrite(blanco, LOW);
  digitalWrite(rojo, LOW);
  digitalWrite(amarillo, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  display.println("Conectando...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  display.clearDisplay();
  display.println(WiFi.localIP());
  display.display();
  // Route for root / web page
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/login.html");
  });
  // Route to load style.css file
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", readDHTTemperature().c_str());
  });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", readDHTHumidity().c_str());
  });
//    server.on("/nitrogeno", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send(200, "text/plain", readDN().c_str());
//  });
//    server.on("/fosforo", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send(200, "text/plain", readDP().c_str());
//  });
//    server.on("/potasio", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send(200, "text/plain", readDK().c_str());
//  });
  
  server.on("/onV", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(verde, HIGH);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });

    server.on("/offV", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(verde, LOW);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });

    server.on("/onB", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(blanco, HIGH);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
    server.on("/offB", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(blanco, LOW);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
  server.on("/onR", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(rojo, HIGH);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
    server.on("/offR", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(rojo, LOW);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
  server.on("/onA", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(amarillo, HIGH);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
    server.on("/offA", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(amarillo, LOW);
    request->send(SPIFFS, "/index.html",String(), false, processor);
  });
  


  



  //  server.on("/onV", HTTP_GET, onV);
  //  server.on("/offV", HTTP_GET, offV);
  //  server.on("/onB", HTTP_GET, onB);
  //  server.on("/offB", HTTP_GET, offB);
  //  server.on("/onR", HTTP_GET, onR);
  //  server.on("/offR", HTTP_GET, offR);
  //  server.on("/onA", HTTP_GET, onA);
  //  server.on("/offA", HTTP_GET, offA);




  // Start server
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
byte n,p,k;
  n = nitrogen();
  delay(250);
  p = phosphorous();
  delay(250);
  k = potassium();
  delay(250);

   Serial.print("Nitrogen: ");
  Serial.print(n);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(p);
  Serial.println(" mg/kg");
  Serial.print("Potassium: ");
  Serial.print(k);
  Serial.println(" mg/kg");
  delay(2000);
 
  display.clearDisplay();
  
 
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("N: ");
  display.print(n);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("P: ");
  display.print(p);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print("K: ");
  display.print(k);
  display.setTextSize(1);
  display.print(" mg/kg");
 
  display.display();

}

byte nitrogen(){
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(nitro,sizeof(nitro))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    Serial.print(values[i],HEX);
    }
    Serial.println();
  }
  return values[4];
}
 
byte phosphorous(){
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(phos,sizeof(phos))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    Serial.print(values[i],HEX);
    }
    Serial.println();
  }
  return values[4];
}
 
byte potassium(){
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(pota,sizeof(pota))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    Serial.print(values[i],HEX);
    }
    Serial.println();
  }
  return values[4];
}
