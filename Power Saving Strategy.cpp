#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "UW MPSK";
const char* password = "9ChpeQ&[Q<";
#define DATABASE_SECRET "AIzaSyCJeokAjBIxjt9Fl5rraHth_PEXiJ22b9w"
#define DATABASE_URL "https://esp32-demo-8c81f-default-rtdb.firebaseio.com/"

const int trigPin = D4;
const int echoPin = D5;

WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);

float lastDistance = 0.0;
const float threshold = 5.0;
const int sleepDuration = 30 * 1000000; 

void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
}

void setupFirebase() {
    ssl.setInsecure();
    initializeApp(client, app, getAuth(dbSecret));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    client.setAsyncResult(result);
}

float measureDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2;
}

void uploadToFirebase(float distance) {
    String path = "/ultrasonic/distance";
    if (Database.set<float>(client, path, distance)) {
        Serial.println("Data uploaded successfully!");
    } else {
        Serial.println("Data upload failed.");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    connectToWiFi();
    setupFirebase();
}

void loop() {
    float distance = measureDistance();
    Serial.print("Measured Distance: ");
    Serial.println(distance);

    if (abs(distance - lastDistance) > threshold) {
        uploadToFirebase(distance);
        lastDistance = distance;
        esp_deep_sleep(sleepDuration);
    } else {
        esp_deep_sleep(sleepDuration);
    }
}
