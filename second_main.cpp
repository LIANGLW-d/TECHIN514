#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <SwitecX25.h>

// X27.168 配置
#define STEPS 315  // X27.168 通常 315°，但可能因型号不同
#define PIN_MOTOR_1 2
#define PIN_MOTOR_2 3
#define PIN_MOTOR_3 4
#define PIN_MOTOR_4 5
SwitecX25 motor(STEPS, PIN_MOTOR_1, PIN_MOTOR_2, PIN_MOTOR_3, PIN_MOTOR_4);

// LED 指示灯
#define LED_PIN 9

// BLE 连接参数
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// 存储当前温度
float currentTemperature = 0.0;

// 处理 BLE 通知数据
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                           uint8_t* pData, size_t length, bool isNotify) {
    String dataStr;
    for (size_t i = 0; i < length; i++) {
        dataStr += (char)pData[i];
    }

    Serial.print("[BLE] Received: ");
    Serial.println(dataStr);

    // 解析温度数据
    int tIndex = dataStr.indexOf("T=");
    int cIndex = dataStr.indexOf("°C");

    if (tIndex != -1 && cIndex != -1) {
        String tempStr = dataStr.substring(tIndex + 2, cIndex);
        currentTemperature = tempStr.toFloat();
        Serial.print("Parsed Temperature: ");
        Serial.println(currentTemperature);

        // 控制 X27.168 指针
        int mappedAngle = map(currentTemperature, 15, 30, 0, STEPS);
        motor.setPosition(mappedAngle);

        // LED 逻辑：温度 > 30°C 时点亮 LED，否则熄灭
        if (currentTemperature > 30) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    }
}

// BLE 连接回调
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("[INFO] Connected to BLE Server.");
    }

    void onDisconnect(BLEClient* pclient) {
        connected = false;
        Serial.println("[ERROR] Disconnected from BLE Server.");
    }
};

// 连接 BLE Server
bool connectToServer() {
    Serial.println("[INFO] Connecting to BLE Server...");

    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("[ERROR] Service not found!");
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("[ERROR] Characteristic not found!");
        pClient->disconnect();
        return false;
    }

    Serial.println("[SUCCESS] Connected to Service & Characteristic.");

    if (pRemoteCharacteristic->canNotify()) {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    return true;
}

// 监听 BLE 广播
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
            Serial.println("[INFO] Found BLE Server!");
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // 初始状态 LED 关闭

    // 初始化 X27.168
    motor.zero();  
    delay(500);
    motor.setPosition(0);

    // 初始化 BLE
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
        if (connectToServer()) {
            Serial.println("[INFO] BLE Connection Established.");
        } else {
            Serial.println("[ERROR] Failed to Connect.");
        }
        doConnect = false;
    }

    motor.update();
    delay(10);
}
