#include <WiFi.h>
#include <PubSubClient.h>

// Replace these with your Wi-Fi credentials
const char* ssid = "A54";
const char* password = "kumrute123";

const char* mqtt_server = "192.168.239.51"; 
const char* mqtt_user = "iot_user";
const char* mqtt_password = "pass";
const char* mqtt_topic_pub = "home/sound/level";
const char* mqtt_topic_sub = "home/led/color";

WiFiClient espClient;
PubSubClient client(espClient);

void setColor(String color) {
  #ifdef RGB_BUILTIN
  if (color == "red") {
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red
  } else if (color == "green") {
    rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Green
  } else if (color == "yellow") {
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, RGB_BRIGHTNESS, 0);  // Yellow
  } else {
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off
  }
  #endif
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = (char*)payload;
  Serial.print("Received: ");
  Serial.println(message);
  setColor(message);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  Serial.print("\nWiFi connected. IP:");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);  // add randomness


    Serial.print("My clientid: ");
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT...");
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  //pixels.begin();
  //pixels.setBrightness(50);  // Set brightness (0–255)
  //setColor("wait");

  setup_wifi();

  client.setServer("192.168.239.51", 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read sound level
  //int raw = analogRead(micPin);

  int dB = random(30, 90);  // Normal conversation to busy street

  // Occasionally simulate a loud event
  if (random(0, 100) < 5) {
    dB = random(90, 110);  // simulate a shout or vacuum cleaner
  }
  char msg[10];
  snprintf(msg, sizeof(msg), "%d", dB);

  // Publish to MQTT
  client.publish(mqtt_topic_pub, msg);
  Serial.print("Sent: ");
  Serial.println(msg);

  delay(5000);  // Every 5 seconds
}

