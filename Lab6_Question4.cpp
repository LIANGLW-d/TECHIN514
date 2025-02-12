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
// **存储最大 & 最小值**
float maxDistance = 0;
float minDistance = 9999;
int dataCount = 0; // **记录接收到的数据次数**
void updateMinMax(float currentDistance) {
if (currentDistance > maxDistance) maxDistance = currentDistance;
if (currentDistance < minDistance) minDistance = currentDistance;
}
// **回调函数：接收 BLE 数据**
static void notifyCallback(
BLERemoteCharacteristic* pBLERemoteCharacteristic,
uint8
_
t* pData,
size
_
t length,
bool isNotify) {
char dataStr[20];
size
_
t copyLength = (length < sizeof(dataStr) - 1) ? length : (sizeof(dataStr) - 1);
memcpy(dataStr, pData, copyLength);
dataStr[copyLength] = '\0';
float currentDistance = atof(dataStr);
updateMinMax(currentDistance);
dataCount++; // **记录数据条数**
// **串口打印当前数据、最大 & 最小值**
Serial.print("[DATA ");
Serial.print(dataCount);
Serial.print("] Received Distance: ");
Serial.print(currentDistance);
Serial.print(" cm | Max: ");
Serial.print(maxDistance);
Serial.print(" cm | Min: ");
Serial.println(minDistance);
if (dataCount >= 10) {
Serial.println("\n Received at least 10 readings! Take screenshot now.\n");
}
}
class MyClientCallback : public BLEClientCallbacks {
void onConnect(BLEClient* pclient) {}
void onDisconnect(BLEClient* pclient) {
connected = false;
Serial.println("Disconnected");
}
};
bool connectToServer() {
Serial.print("Connecting to: ");
Serial.println(myDevice->getAddress().toString().c
_
str());
BLEClient* pClient = BLEDevice::createClient();
pClient->setClientCallbacks(new MyClientCallback());
pClient->connect(myDevice);
pClient->setMTU(517);
BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
if (!pRemoteService) {
Serial.println("Failed to find service!");
pClient->disconnect();
return false;
}
pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
if (!pRemoteCharacteristic) {
Serial.println("Failed to find characteristic!");
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
BLEDevice::init("");
BLEScan* pBLEScan = BLEDevice::getScan();
pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
pBLEScan->setInterval(1349);
pBLEScan->setWindow(449);
pBLEScan->setActiveScan(true);
pBLEScan->start(5, false);
}
void loop() {
if (doConnect) {
if (connectToServer()) Serial.println("Connected!");
else Serial.println("Failed to connect!");
doConnect = false;
}
if (connected) delay(1000);
else if (doScan) BLEDevice::getScan()->start(0);
}
