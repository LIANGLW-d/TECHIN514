#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
static BLEUUID serviceUUID("2f22b6ad-51df-4f4c-9a3c-9f902d12c464");
static BLEUUID charUUID("f7089b06-e82a-44bc-935a-2f31628f11c9");
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
float maxDistance = 0;
float minDistance = 1000;
// **更新最大/最小值**
void updateMinMax(float value) {
if (value > maxDistance) maxDistance = value;
if (value < minDistance) minDistance = value;
}
static void notifyCallback(
BLERemoteCharacteristic* pBLERemoteCharacteristic,
uint8
_
t* pData,
size
_
t length,
bool isNotify) {
Serial.print("Received Data: ");
Serial.write(pData, length);
Serial.println();
float receivedDistance = atof((char*)pData);
updateMinMax(receivedDistance);
Serial.print("Current: ");
Serial.print(receivedDistance);
Serial.print(" cm | Max: ");
Serial.print(maxDistance);
Serial.print(" cm | Min: ");
Serial.print(minDistance);
Serial.println(" cm");
}
class MyClientCallback : public BLEClientCallbacks {
void onConnect(BLEClient* pclient) {}
void onDisconnect(BLEClient* pclient) {
connected = false;
Serial.println("Disconnected!");
}
};
bool connectToServer() {
Serial.print("Connecting to ");
Serial.println(myDevice->getAddress().toString().c
_
str());
BLEClient* pClient = BLEDevice::createClient();
pClient->setClientCallbacks(new MyClientCallback());
pClient->connect(myDevice);
Serial.println("Connected to server!");
BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
if (pRemoteService == nullptr) {
Serial.println("Failed to find service");
pClient->disconnect();
return false;
}
pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
if (pRemoteCharacteristic == nullptr) {
Serial.println("Failed to find characteristic");
pClient->disconnect();
return false;
}
if (pRemoteCharacteristic->canNotify())
pRemoteCharacteristic->registerForNotify(notifyCallback);
connected = true;
return true;
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
void onResult(BLEAdvertisedDevice advertisedDevice) {
if (advertisedDevice.haveServiceUUID() &&
advertisedDevice.isAdvertisingService(serviceUUID)) {
BLEDevice::getScan()->stop();
myDevice = new BLEAdvertisedDevice(advertisedDevice);
doConnect = true;
doScan = true;
}
}
};
void setup() {
Serial.begin(115200);
Serial.println("Starting BLE Client...
");
BLEDevice::init("");
BLEScan* pBLEScan = BLEDevice::getScan();
pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
pBLEScan->setActiveScan(true);
pBLEScan->start(5, false);
}
void loop() {
if (doConnect) {
if (connectToServer()) Serial.println("Connected to Server!");
doConnect = false;
}
if (connected) BLEDevice::getScan()->start(0);
delay(1000);
}
