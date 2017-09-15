#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <map>
#include <vector>

// WiFi and MQTT info

const char* ssid = "";
const char* password = "";
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);


std::vector<int> sw1_state(2);
std::vector<int> sw2_state(2);
std::vector<int> sw3_state(2);
std::vector<int> sw4_state(2);

std::map<String, std::vector<int>> gpio_config {
  {"home/livingroom/linknode1/sw1", sw1_state},
  {"home/livingroom/linknode1/sw2", sw2_state},
  {"home/livingroom/linknode1/sw3", sw3_state},
  {"home/livingroom/linknode1/sw4", sw4_state}
};


void setup() {
  pinMode(12, OUTPUT); // Setup Relay1
  pinMode(13, OUTPUT); // Setup Relay2
  pinMode(14, OUTPUT); // Setup Relay3
  pinMode(16, OUTPUT); // Setup Relay4
  gpio_config["home/livingroom/linknode1/sw1"] = {12, 0};
  gpio_config["home/livingroom/linknode1/sw2"] = {13, 0};
  gpio_config["home/livingroom/linknode1/sw3"] = {14, 0};
  gpio_config["home/livingroom/linknode1/sw4"] = {16, 0};
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
  // Reference of gpio_config
  std::vector<int>& sw = gpio_config[topic];
  // Switch Relay on or off
  if (data == "on") {
    digitalWrite(sw[0], 1);
    sw[1] = 1;
  } else if (data == "off"){
    digitalWrite(sw[0], 0);
    sw[1] = 0;
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
        client.publish(gpio_topic.first.c_str(), gpio_topic.second[1]==1 ? "on" : "off");
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
