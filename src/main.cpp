#include <Arduino.h>
#include<ArduinoJson.h>
#include<PubSubClient.h>
#include<WiFi.h>
#include<Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"



const char* ssid ="";
const char* pwd ="";
const char* broker = "broker.hivemq.com";
const char* client_id ="esp32";
const char* mqtt_user = "admin";
const char* mqtt_pwd ="Sergioala10.";
const char* hostname = "Home Controller";


float temperature ,humidity,pression,gas;
bool door,lavatrice,light;
#define lavatrice_pin 34
#define door_pin 36
#define light_pin 27
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme


WiFiClient espclient;
PubSubClient client(espclient);


void pin_state(bool state ,int pin_number);
void Read_Data();
void reconnect();
void connect_wifi();
void callback(char* topic,byte* payload,unsigned int length);
void setup() {
  Serial.begin(115200);
  connect_wifi();
  client.setCallback(callback);
  client.setServer(broker,1883);
  pinMode(lavatrice_pin,OUTPUT);
  pinMode(light_pin,OUTPUT);
  pinMode(door_pin,OUTPUT);
   while (!Serial);
  Serial.println(F("BME680 test-------------------------"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
  
  
  

 
}

void connect_wifi(){
  WiFi.setHostname(hostname);
  WiFi.begin(ssid,pwd);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("tried to connect ....................");
    delay(200);
  }
  Serial.println("connected done -----------------");
  Serial.print("ip add :");
  Serial.println(WiFi.localIP());
  Serial.print("mac add :");
  Serial.print(WiFi.macAddress());
}
void reconnect(){
  while(!client.connected()){
    Serial.println('connecting to mqtt------------');
    if(client.connect(client_id,mqtt_user,mqtt_pwd)){
      client.subscribe("home_rx");
    }
    else{
      Serial.println(client.state());
      Serial.print("waiting for five seconds");
      delay(5000);
    }
  }
}
void callback(char* topic,byte* payload,unsigned int length){
  Serial.print("subscrine to topic :");
  Serial.print(topic);
  Serial.print("payload :");
  String msg ;
  for(int i=0 ;i<length;i++){
    Serial.print((char)payload[i]);
    msg +=(char)payload[i];
  }
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc,msg);
  if(error){
    Serial.print("error to open json file");
    Serial.print(error.c_str());
    return ;
  }
  light = doc["outdoor"]["light"];
  door = doc["outdoor"]["door"];
  lavatrice = doc["outdoor"]["lavatrice"];
  pin_state(light,light_pin);
  pin_state(door,door_pin);
  pin_state(lavatrice,lavatrice_pin);
  Serial.print("outdoor light : ");
  Serial.println(light);
  Serial.print("outdoor door : ");
  Serial.println(door);
  Serial.print("outdoor lavatrice: ");
  Serial.println(lavatrice);
  
  }
  
  void pin_state(bool state ,int pin_number){
    if (state == true){
      digitalWrite(pin_number,HIGH);
    }
    else{ 
      digitalWrite(pin_number,LOW);
    }
  }


void Read_Data(){
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at  :"));
  Serial.print(millis());
  Serial.print(F(" and will finish at  :"));
  Serial.println(endTime);

  Serial.println(F("wait a moment ----------------"));
  delay(50);
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));
  temperature = bme.temperature ; 

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));
  pression = bme.pressure ;

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));
  humidity = bme/humidity ; 
  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));
  gas = bme.gas_resistance ;
  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println();
  delay(2000);
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  delay(2000);
  Read_Data();
  StaticJsonDocument<256> doc ;
  JsonObject root = doc.to<JsonObject>();
  JsonObject kitchen =root.createNestedObject("kitchen");
  kitchen["temperature"].set(temperature);
  kitchen["humidity"].set(humidity);
  kitchen["pressure"].set(pression);
  kitchen["gaz"].set(gas);
  String json_output;
  serializeJson(doc,json_output);
  Serial.println(json_output);
  client.publish("home_tx",json_output.c_str());
  
  
}

