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
const int RedLED = 26; // Red LED pin
const int GreenLED = 27; // Green LED pin
const int BlueLED = 14; // Blue LED pin
const int YellowLED = 12; // Yellow LED pin


const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker settings
const char* mqtt_broker = "<test.mosquitto.org>";  // Free public MQTT broker
const int mqtt_port = 1883;
const char* mqtt_username = "<username>";  // username given in the telegram group
const char* mqtt_password = "<password>";  // password given in the telegram group

const char* mqtt_topic_green = "ttpu/iot/asadullo/ledgreen";   // Topic to publish
const char* mqtt_topic_red = "ttpu/iot/asadullo/ledgred";    // Topic to subscribe
const char* mqtt_topic_blue = "ttpu/iot/asadullo/ledblue";   // Topic to publish
const char* mqtt_topic_yellow = "ttpu/iot/asadullo/ledyellow";   // Topic to publish
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
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  digitalWrite(BlueLED, LOW);
  digitalWrite(YellowLED, LOW);

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