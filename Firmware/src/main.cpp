#include <Arduino.h>
#include "WiFi.h"
#include "../lib/Arduino_JSON/ArduinoJson.h"
#include "../lib/Pubsubclient/PubSubClient.h"

// put function declarations here:
const char* ssid = "Aryomtech";
const char* password = "2910@Vibhuti";
const char *topic_US_Sensor="esp32/waterlevel";
const char *topic_connection_sub="esp32/waterconnection";
const char *payload_connection="Connected";
const char* mqtt_server = "192.168.29.198";
int counter_alert=0;
int flag_alert=0;
char *payload_US_Sensor;
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
int UL_trigger = 32;
int UL_echo = 35;
int duration, distance;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(UL_trigger, OUTPUT);
  pinMode(UL_echo, INPUT);
  //esp_sleep_enable_timer_wakeup(6000000); //wakeup after 6 seconds
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String topic_str=String(topic);
  String messageresponse_switch; //this declare once when some data comes on topic esp32/output
  for (int j = 0; j < length; j++) {
    Serial.println(length);
    Serial.print((char)message[j]);
    messageresponse_switch += (char)message[j];
  }
}
 void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32", "vvc", "Hello")){
      Serial.println("connected");
      // Subscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
    printf("Reconnected\n");
  }
  printf("In Loop\n");
  client.loop();
  client.publish(topic_connection_sub,payload_connection);
   //this we are passing topic and payload with
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    digitalWrite(UL_trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(UL_trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(UL_trigger, LOW);
    duration = pulseIn(UL_echo, HIGH);
    distance = (duration / 2) / 29.1;
    Serial.println(String(distance).c_str());
    //payload_US_Sensor=String(distance).c_str();
   // sprintf(payload_US_Sensor, "%d", distance);
    Serial.println(payload_US_Sensor);
    client.publish(topic_US_Sensor, String(distance).c_str());
    Serial.print(distance);
    Serial.println(" cm");
    if(distance>=15 && distance<=23 && counter_alert<5 && flag_alert==0){
      client.publish("esp32/alert","Alert! Water Level is going to LOW, Please Refill");
      counter_alert++;
      if(counter_alert==5){
        flag_alert=1;
        counter_alert=0;
      }
    }
    else if(distance<=5 && counter_alert<5 && flag_alert==1){
      client.publish("esp32/alert","Alert! Water Level is going to HIGH, Please Stop Refill");
      counter_alert++;
      if(counter_alert==5){
        flag_alert=0;
        counter_alert=0;
      }
    }
}
// Sleep Code
/*client.publish("esp32/sleep","Going to Sleep for 6 seconds");
Serial.println("Going to Sleep for 5 seconds");
delay(1000);
esp_light_sleep_start();
Serial.println("woke up");*/
}
  // put your main code here, to run repeatedly:

// put function definitions here: