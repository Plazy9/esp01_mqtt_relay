#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MY_BLUE_LED_PIN 1
#define RELAY_PIN 2 //Relay Pin


// Update these with values suitable for your network.

const char* ssid = wifiSSID;
const char* password = wifiPassword;
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = mqttServer;
const char* mqttServerUser = mqttUser;
const char* mqttServerPWD = mqttPassword;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  pinMode(RELAY_PIN, OUTPUT);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    client.publish("pl_esp01_relay/pl_stateTopic", "on");
    digitalWrite(MY_BLUE_LED_PIN , LOW);   // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(RELAY_PIN , LOW);
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    client.publish("pl_esp01_relay/pl_stateTopic", "off");
    digitalWrite(MY_BLUE_LED_PIN , HIGH);  // Turn the LED off by making the voltage HIGH
    digitalWrite(RELAY_PIN , HIGH);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttServerUser, mqttServerPWD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("pl_esp01_relay/pl_outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("pl_esp01_relay/pl_commandTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(MY_BLUE_LED_PIN , OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("pl_esp01_relay/pl_outTopic", msg);
  }
}
