// - RED LED - `D26`
// - Green LED - `D27`
// - Blue LED - `D14`
// - Yellow LED - `D12`

// - Button (Active high) - `D25`
// - Light sensor (analog) - `D33`

// - LCD I2C - SDA: `D21`
// - LCD I2C - SCL: `D22`

/**************************************
 * LAB 3 - EXERCISE 2
 **************************************/


#include "Arduino.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

const int RedLED = 26; // Red LED pin
const int GreenLED = 27; // Green LED pin
const int BlueLED = 14; // Blue LED pin
const int YellowLED = 12; // Yellow LED pin
const int But = 25; // Button pin
const int LightSensor = 33; // Light sensor pin

hd44780_I2Cexp lcd;  // Auto-detect I2C address
const int LCD_COLS = 16;
const int LCD_ROWS = 2;


const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker settings
const char* mqtt_broker = "mqtt.iotserver.uz";  // Free public MQTT broker
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";  // username given in the telegram group
const char* mqtt_password = "mqttpass";  // password given in the telegram group

const char* mqtt_topic_green = "ttpu/iot/asadullo/ledgreen";   // Topic to publish
const char* mqtt_topic_red = "ttpu/iot/asadullo/ledgred";    // Topic to subscribe
const char* mqtt_topic_blue = "ttpu/iot/asadullo/ledblue";   // Topic to publish
const char* mqtt_topic_yellow = "ttpu/iot/asadullo/ledyellow";   // Topic to publish
const char* mqtt_topic_sensor = "ttpu/iot/asadullo/light"; 
const char* mqtt_topic_button = "ttpu/iot/asadullo/button";   
const char* mqtt_topic_lcd = "ttpu/iot/asadullo/display";
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectWiFi();
void connectMQTT();
void mqqtCallback(char* topic, byte* payload, unsigned int length);
void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(But, INPUT);
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  digitalWrite(BlueLED, LOW);
  digitalWrite(YellowLED, LOW);

  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status) {
    Serial.println("LCD initialization failed!");
    Serial.print("Status code: ");
    Serial.println(status);
    hd44780::fatalError(status);
  }
  
  Serial.println("LCD initialized successfully!");
  
  // Clear LCD and display initial message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);

  connectWiFi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqqtCallback);
  connectMQTT();


}

void loop() 
{
  if (!mqtt_client.connected()) {
    Serial.println("Reconnecting to MQTT broker...");
    connectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    connectWiFi();
  }
  mqtt_client.loop();
  static unsigned long lastPublishTime = 0;
  const long publishInterval = 5000;  
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime >= publishInterval) {
    lastPublishTime = currentTime;
    int sensorValue = analogRead(LightSensor);
    Serial.print("Light sensor value: ");
    Serial.println(sensorValue);
    JsonDocument doc;
    doc["light"] = sensorValue;
    doc["timestamp"] = millis();
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    Serial.print("Publishing message: ");
    Serial.println(jsonBuffer);
    if (mqtt_client.publish(mqtt_topic_sensor, jsonBuffer)) {
      Serial.println("Message published successfully!");
    } else {
      Serial.println("Failed to publish message!");
    }
  }
  
  //Decetct button press
  static int lastbtnstate = LOW;
  int currentbtnstate = digitalRead(But);
  static unsigned long lastdebouncetime = 0;
  currentTime = millis();

  if (currentbtnstate != lastbtnstate && (currentTime - lastdebouncetime) > 100) {

    lastbtnstate = currentbtnstate;
    String btnmsg="";

    if (currentbtnstate == HIGH){
      btnmsg="Button Pressed";
      Serial.println(btnmsg);
    }
    else{
      btnmsg="Button Released";
      Serial.println(btnmsg);
    }

    JsonDocument btndoc;
    btndoc["event"] = btnmsg;
    btndoc["timestamp"] = millis();
    char btnEventMsg[256];
    serializeJson(btndoc, btnEventMsg); 

    if (mqtt_client.publish(mqtt_topic_sensor, btnEventMsg)) {
      Serial.println("Button event published successfully!");
    } 
    else {
      Serial.println("Failed to publish button event!");
    }
  }
}
void connectWiFi(void){
  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
} 

void connectMQTT(void){
  while (!mqtt_client.connected()){
    Serial.println("Connecting to MQTT broker...");
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker!"); 
      //Connected, now subscribe to topic
      mqtt_client.subscribe(mqtt_topic_red);
      mqtt_client.subscribe(mqtt_topic_green);
      mqtt_client.subscribe(mqtt_topic_blue);
      mqtt_client.subscribe(mqtt_topic_yellow);
      mqtt_client.subscribe(mqtt_topic_lcd);

    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }  
  }
}

void mqqtCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  int chosenLED =-1;
  String topicStr = String(topic);
  if (topicStr == mqtt_topic_red)
  {
    chosenLED = RedLED;
  }
  else if (topicStr == mqtt_topic_green)
  {
    chosenLED = GreenLED;
  }
  else if (topicStr == mqtt_topic_blue)
  {
    chosenLED = BlueLED;
  }
  else if (topicStr == mqtt_topic_yellow)
  {
    chosenLED = YellowLED;
  }
  
  
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message.c_str());
  if(error){
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  if(topicStr == mqtt_topic_lcd && doc.containsKey("text")){
    String lcdMsg = doc["text"].as<String>();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(lcdMsg);
  }

  String state_value = "";
  if (doc.containsKey("state")){
    state_value = doc["state"].as<String>();
  }
  int ledState = -1;
  if (state_value == "ON"){
      ledState = HIGH;
  }
  else if (state_value == "OFF"){
    ledState = LOW;
  }
  if (chosenLED !=-1 && ledState != -1){
    digitalWrite(chosenLED, ledState);
    Serial.print("Set LED on pin ");
    Serial.print(chosenLED);
    Serial.print(" to state ");
    Serial.println(state_value);
  }
} 