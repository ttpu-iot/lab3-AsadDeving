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


// GLOBAL DECLARATIONS
const int But = 25; // Button pin
const int LightSensor = 33; // Light sensor pin

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker settings
const char* mqtt_broker = "<test.mosquitto.org>";  // Free public MQTT broker
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";  // username given in the telegram group
const char* mqtt_password = "mqttpass";  // password given in the telegram group

const char* mqtt_topic_sensor = "ttpu/iot/asadullo/light"; 
const char* mqtt_topic_button = "ttpu/iot/asadullo/button";   

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectWiFi();
void connectMQTT();

/*************************
 * SETUP
 */
void setup(){
  Serial.begin(115200);
  delay(1000);
  pinMode(But, INPUT);
  connectWiFi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  connectMQTT();
  
}


/*************************
 * LOOP
 */
void loop(){ 
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    connectWiFi();
  }
  if (!mqtt_client.connected()){
    Serial.println("Reconnecting to MQTT broker...");
    connectMQTT();
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
  if (currentTime != lastbtnstate && (currentTime - lastdebouncetime) > 100){
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
    } else {
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
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }  
  }
}
