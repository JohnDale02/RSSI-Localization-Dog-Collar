#include <Arduino.h>
#include <unordered_map>
#include <string>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ctime>
#include <chrono>
#define CERT mqtt_broker_cert
#define MSG_BUFFER_SIZE (50)

//--------------------------------------
// config (edit here before compiling)
//--------------------------------------/

int BOARD_NUMBER = 2;
unsigned long updateInterval = 1000;  // time in ms for timer to update

std::unordered_map<int, std::array<const char*,3>> Board_Init {
{1, {"ESP2", "ESP3", "ESP1"}},
{2, {"ESP1", "ESP3", "ESP2"}},
{3, {"ESP1", "ESP2", "ESP3"}}
};

//#define MQTT_TLS // uncomment this define to enable TLS transport
//#define MQTT_TLS_VERIFY // uncomment this define to enable broker certificate verification
const char* ssid = "DalesWifi1";
const char* password = "teddy123";
const char* mqtt_server = "10.0.0.77"; // eg. your-demo.cedalo.cloud or 192.168.1.11
const uint16_t mqtt_server_port = 1883; // or 8883 most common for tls transport
const char* mqttUser = "";
const char* mqttPassword = "";

const u_int8_t STATUS_PIN = D0;
const u_int8_t TOPIC_PIN = D5;

const char* mqtt1 = Board_Init[BOARD_NUMBER][0];
const char* mqtt2 = Board_Init[BOARD_NUMBER][1];
const char* mqttTopicOut = Board_Init[BOARD_NUMBER][2];
auto last_time = std::chrono::steady_clock::now();
int value = 0;   // led initialization for data coming in
int repeat = 0;  // toggle led's 1/4 of time

//--------------------------------------
// globals
//--------------------------------------
#ifdef MQTT_TLS
  WiFiClientSecure wifiClient;
#else
  WiFiClient wifiClient;
#endif
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, updateInterval);
PubSubClient mqttClient(wifiClient);

void connection_status(int delayTime, int iterations){
  for (int i = 0; i < iterations; i++){
    digitalWrite(STATUS_PIN, HIGH);
    delay(delayTime);
    digitalWrite(STATUS_PIN, LOW);
    delay(delayTime);
  }
}

//--------------------------------------
// function setup_wifi called once
//--------------------------------------
void setup_wifi() {
  
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    connection_status(500, 5);
  }

  timeClient.begin();

#ifdef MQTT_TLS
  #ifdef MQTT_TLS_VERIFY
    X509List *cert = new X509List(CERT);
    wifiClient.setTrustAnchors(cert);
  #else
    wifiClient.setInsecure();
  #endif
#endif

  Serial.println("WiFi connected");
}

//--------------------------------------
// function callback called everytime 
// if a mqtt message arrives from the broker
//--------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
//--------------------------------------
// function connect called to (re)connect
// to the broker
//--------------------------------------
void connect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    connection_status(100, 50);
    String mqttClientId = mqttTopicOut;
    if (mqttClient.connect(mqttClientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");

      mqttClient.subscribe(mqtt1, 0);
      mqttClient.subscribe(mqtt2, 0);
      digitalWrite(STATUS_PIN, HIGH);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" will try again in 5 seconds");
    }
  }
}

//--------------------------------------
// main arduino setup fuction called once
//--------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(TOPIC_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  setup_wifi();
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(callback);
  connection_status(1000, 4);
  
}

//--------------------------------------
// main arduino loop fuction called periodically
//--------------------------------------

void loop() {
  if (!mqttClient.connected()) {
    digitalWrite(STATUS_PIN, LOW);
    digitalWrite(TOPIC_PIN, LOW);
    connect();
  }

  else{
    auto now = std::chrono::steady_clock::now();
    timeClient.update();

    if ((now - last_time).count() >= 300000000) {

      mqttClient.loop();
      last_time = now;
      String myCurrentTime = timeClient.getFormattedTime();
      String Board_Name = String(Board_Init[BOARD_NUMBER][2]);
      mqttClient.publish(mqttTopicOut,(": Time: " + myCurrentTime).c_str(), false);

      if ((value == 1) & (repeat >= 4)){
        value = 0;
        repeat = 0;
        digitalWrite(TOPIC_PIN, HIGH);
      }

      if ((value == 0) & (repeat >= 4)){
        value = 1;
        repeat = 0;
        digitalWrite(TOPIC_PIN, LOW);
      }
    }
    repeat++;
  }
  
}