#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

// MQTT config buffers (filled via portal)
char mqtt_server[40] = "";
char mqtt_user[40] = "iot_user";
char mqtt_pass[40] = "pass";
char esp_location[40] = "";

char mqtt_topic_pub[40] = "home/sound/level";
char mqtt_topic_color[60];
char mqtt_topic_registration[40] = "home/sensor/register";
char mqtt_topic_response[60];


String hwid;

volatile bool registrationSuccess = false;
volatile bool registrationFailed = false;

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

// MQTT message callback
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = (char*)payload;
  String topicStr = String(topic);

  Serial.print("Received: ");
  Serial.print(topicStr);
  Serial.print(" => ");
  Serial.println(message);

  if (topicStr == String(mqtt_topic_response)) {
    if (message == "OK") {
      registrationSuccess = true;
    } else {
      registrationFailed = true;
    }
  } else if (topicStr == String(mqtt_topic_color)) {
    setColor(message);  // LED control
  }
}

// Setup WiFi + MQTT portal
void setup_wifi() {
  WiFiManager wm;

  const char* apName = "ESP32_Setup";
  const char* apPassword = "esp32config";

  wm.resetSettings();  // Always show portal on boot (for dev)

  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT Username", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", mqtt_pass, 40);
  WiFiManagerParameter custom_location("location", "Location", esp_location, 40);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_location);

  if (!wm.autoConnect(apName, apPassword)) {
    Serial.println("Failed to connect or portal timeout");
    ESP.restart();
  }

  strncpy(mqtt_server, custom_mqtt_server.getValue(), sizeof(mqtt_server));
  strncpy(mqtt_user, custom_mqtt_user.getValue(), sizeof(mqtt_user));
  strncpy(mqtt_pass, custom_mqtt_pass.getValue(), sizeof(mqtt_pass));
  strncpy(esp_location, custom_location.getValue(), sizeof(esp_location));

  Serial.println("Connected to WiFi!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

// Ensure MQTT connection
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();
    hwid = mac;

    String responseTopic = "/home/sensor/" + hwid + "/response";
    strncpy(mqtt_topic_response, responseTopic.c_str(), sizeof(mqtt_topic_response));
    
    String colorTopic = "home/sensor/" + hwid + "/color";
    strncpy(mqtt_topic_color, colorTopic.c_str(), sizeof(mqtt_topic_color));

    Serial.print("My clientid: ");
    Serial.println(clientId);

    Serial.print("MQTT server: ");
    Serial.println(mqtt_server);
    Serial.print("MQTT username: ");
    Serial.println(mqtt_user);
    Serial.print("MQTT pub topic: ");
    Serial.println(mqtt_topic_pub);
    Serial.print("MQTT sub topic: ");
    Serial.println(mqtt_topic_color);
    Serial.print("MQTT registration response topic: ");
    Serial.println(mqtt_topic_response);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT!");
      client.subscribe(mqtt_topic_color);       // LED control
      client.subscribe(mqtt_topic_response);  // Registration response

      // Format registration message
      String payload = "{\"hwid\":\"" + hwid + "\",\"location\":\"" + String(esp_location) + "\"}";
      Serial.print("Publishing registration payload: ");
      Serial.println(payload);

      // Publish registration message
      client.publish(mqtt_topic_registration, payload.c_str());

      // delay(5000);

      // if (registrationSuccess) {
      //   Serial.println("Registration successful.");
      // } else {
      //   Serial.println("No OK received, retrying...");
      //   delay(2000);  // wait before re-publishing
      // }

    } else {
      Serial.print("MQTT connect failed, rc=");
      Serial.print(client.state());
      Serial.println(" — retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// Main loop
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Simulated sound level
  int dB = random(30, 90);
  if (random(0, 100) < 5) {
    dB = random(90, 110);
  }

  String payload = "{\"hwid\":\"" + hwid + "\",\"level\":\"" + String(dB) + "\"}";
  Serial.print("Publishing microphone payload: ");
  Serial.println(payload);
  client.publish(mqtt_topic_pub, payload.c_str());

  delay(5000);
}
