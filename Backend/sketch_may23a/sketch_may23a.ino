#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

#define WIFI_SSID "hi"
#define WIFI_PASSWORD "123456789"
#define API_KEY "AIzaSyBfH2wh_Jg4NhCi41DO_oagPuXpFVBIZGs"
#define DATABASE_URL "https://iot-cloud-510fa-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define MQ2_PIN 34
#define MQ135_PIN 35
#define BEDROOM_LED 2
#define KITCHEN_LED 4
#define LIVING_LED 5

unsigned long lastUpdate = 0;
unsigned long lastLoop = 0;
const float LED_WATT = 10.0;
const float MQ2_WATT = 0.2;
const float MQ135_WATT = 0.2;
const float kWh_rate = 10.0;

void setup() {
  Serial.begin(115200);
  pinMode(BEDROOM_LED, OUTPUT);
  pinMode(KITCHEN_LED, OUTPUT);
  pinMode(LIVING_LED, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp succeeded");
  } else {
    Serial.printf("Firebase signUp failed: %s\n", config.signer.signupError.message.c_str());
  }

  while (!Firebase.ready()) delay(100);
}

void loop() {
  unsigned long now = millis();
  if (now - lastLoop < 1000) return; // 1s polling
  lastLoop = now;

  // LED toggling from Firebase
  bool state = getBool("/appliances/bedroom");
  digitalWrite(BEDROOM_LED, state);
  Firebase.RTDB.setBool(&fbdo, "/appliances/bedroom_state", state);

  state = getBool("/appliances/kitchen");
  digitalWrite(KITCHEN_LED, state);
  Firebase.RTDB.setBool(&fbdo, "/appliances/kitchen_state", state);

  state = getBool("/appliances/living");
  digitalWrite(LIVING_LED, state);
  Firebase.RTDB.setBool(&fbdo, "/appliances/living_state", state);

  // Sensor toggling from Firebase
  bool mq2_enabled = getBool("/appliances/mq2");
  bool mq135_enabled = getBool("/appliances/mq135");

  // Always update the state fields for the frontend display
  Firebase.RTDB.setBool(&fbdo, "/appliances/mq2_state", mq2_enabled);
  Firebase.RTDB.setBool(&fbdo, "/appliances/mq135_state", mq135_enabled);

  if (mq2_enabled) {
    int mq2Raw = analogRead(MQ2_PIN); // 0-4095
    int mq2Val = map(mq2Raw, 0, 4095, 0, 1000); // Map to 0-1000 for frontend
    Firebase.RTDB.setInt(&fbdo, "/sensors/mq2", mq2Val);
  }     

  if (mq135_enabled) {
    int mq135Raw = analogRead(MQ135_PIN); // 0-4095
    int mq135Val = map(mq135Raw, 0, 4095, 0, 500); // Map to 0-500 for frontend
    Firebase.RTDB.setInt(&fbdo, "/sensors/mq135", mq135Val);
  }

  // Usage tracking every minute
  if (now - lastUpdate >= 60000) {
    lastUpdate = now;

    updateUsage("bedroom", digitalRead(BEDROOM_LED), LED_WATT);
    updateUsage("kitchen", digitalRead(KITCHEN_LED), LED_WATT);
    updateUsage("living", digitalRead(LIVING_LED), LED_WATT);
    updateUsage("mq2", mq2_enabled, MQ2_WATT);
    updateUsage("mq135", mq135_enabled, MQ135_WATT);

    // Compute total cost
    float totalWh = getUsage("bedroom") + getUsage("kitchen") + getUsage("living") + getUsage("mq2") + getUsage("mq135");
    float kWh = totalWh / 1000.0;
    float cost = kWh * kWh_rate;
    Firebase.RTDB.setFloat(&fbdo, "/usage/total_cost", cost);
  }
}

bool getBool(String path) {
  if (Firebase.RTDB.getBool(&fbdo, path)) return fbdo.boolData();
  return false;
}

void updateUsage(String key, bool on, float watt) {
  if (!on) return;
  float wh = watt / 60.0;  // watt-minute to watt-hour
  float current = 0;
  if (Firebase.RTDB.getFloat(&fbdo, "/usage/" + key)) {
    current = fbdo.floatData();
    if (isnan(current)) current = 0;
  }
  Firebase.RTDB.setFloat(&fbdo, "/usage/" + key, current + wh);
}

float getUsage(String key) {
  if (Firebase.RTDB.getFloat(&fbdo, "/usage/" + key)) {
    float val = fbdo.floatData();
    if (isnan(val)) return 0;
    return val;
  }
  return 0;
}