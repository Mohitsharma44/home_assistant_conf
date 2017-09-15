#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <map>
// WiFi and MQTT info

const char* ssid = "wifiname";
const char* password = "wifipassword";
const char* mqtt_user = "username";
const char* mqtt_pass = "password";
const char* mqtt_server = "mqttserver";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int sw = 0;

std::map<String, int> gpio_config {
  {"home/livingroom/linknode1/sw1", 0},
  {"home/livingroom/linknode1/sw2", 0},
  {"home/livingroom/linknode1/sw3", 0},
  {"home/livingroom/linknode1/sw4", 0}
};


void setup() {
  pinMode(12, OUTPUT); // Setup Relay1
  pinMode(13, OUTPUT); // Setup Relay2
  pinMode(14, OUTPUT); // Setup Relay3
  pinMode(16, OUTPUT); // Setup Relay4
  gpio_config["home/livingroom/linknode1/sw1"] = 12;
  gpio_config["home/livingroom/linknode1/sw2"] = 13;
  gpio_config["home/livingroom/linknode1/sw3"] = 14;
  gpio_config["home/livingroom/linknode1/sw4"] = 16;
  Serial.begin(115200);
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

void callback(char* topic, byte* payload, unsigned int length) {
  String data = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println();
  sw = gpio_config[topic];
  // Switch Relay on or off
  if (data == "on") {
    digitalWrite(sw, HIGH);
  } else if (data == "off"){
    digitalWrite(sw, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      for(auto const &gpio_topic : gpio_config){
        //client.publish(gpio_topic.first.c_str, "off");
        // Subscribe to all the topics
        client.subscribe(gpio_topic.first.c_str());
      }
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
  }
  client.loop();
}
