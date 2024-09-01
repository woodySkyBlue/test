/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// The remote service we wish to connect to.
static BLEUUID SERVICE_UUID("5699d362-0c53-11e7-93ae-92361f002671");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static int SCAN_TIME = 1;
BLEScan* pBLEScan;

static boolean doConnect = false;
static boolean isConnected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* portableGasMonitor;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("Connect Event");
    }
    void onDisconnect(BLEClient* pclient) {
        isConnected = false;
        Serial.println("Disconnected.");
    }
};

bool connectToServer() {
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    // ポータブルガスモニタに接続
    pClient->connect(portableGasMonitor);
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr) {
        Serial.println("[Error] pClient->getService(SERVICE_UUID)");
        pClient->disconnect();
        return false;
    }
    // pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    // if (pRemoteCharacteristic == nullptr) {
    //     Serial.println("[Error] pRemoteService->getCharacteristic(charUUID)");
    //     pClient->disconnect();
    //     return false;
    // }
    // if(pRemoteCharacteristic->canRead()) {
    //     std::string value = pRemoteCharacteristic->readValue();
    // }
    // if(pRemoteCharacteristic->canNotify())
    //     pRemoteCharacteristic->registerForNotify(notifyCallback);

    isConnected = true;
    return true;
}

//**********************************************************************************
// アドバタイズ信号受信イベント
//**********************************************************************************
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        //Serial.println("[Event] Advertised");
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(SERVICE_UUID)) {
            // 受信したアドバタイズ信号がポータブルガスモニタのとき
            // Serial.printf("Name=%s, ", advertisedDevice.getName().c_str());
            // Serial.printf("Address=%s, ", advertisedDevice.getAddress().toString().c_str());
            // Serial.printf("RSSI=%d \n", advertisedDevice.getRSSI());
            Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

            BLEDevice::getScan()->stop();
            portableGasMonitor = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }
};


void setup() {
    Serial.begin(115200);
    // BLE初期化
    BLEDevice::init("");
    // 新しいデバイスが検出されたときに通知されるコールバックを設定
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    // pBLEScan->start(SCAN_TIME, false);
}

void loop() {
    if (isConnected) {
        Serial.println("Connected.");
    } else {
        Serial.println("ReScanning Start.");
        //BLEDevice::getScan()->start(0);
        pBLEScan->start(SCAN_TIME, false);
    }
    // フラグ 「doConnect 」がtrueなら、接続したいBLEサーバーをスキャンして見つけたことになる。
    // 次に、そのサーバーに接続します。
    // 接続が完了したら、connectedフラグをtrueに設定します。
    if (doConnect == true) {
        doConnect = false;
        if (connectToServer()) {
            Serial.println("Connect PortableGasMonitor");
        } else {
            Serial.println("[Error] Connect PortableGasMonitor");
        }
    }
    delay(1000);
}
