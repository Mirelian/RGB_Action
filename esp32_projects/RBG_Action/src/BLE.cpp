#include "header.h"

class MyBLECallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        if (pCharacteristic->getUUID().toString() == SSID_CHAR_UUID)
        {
            strncpy(ssid, value.c_str(), sizeof(ssid));
            Serial.print("Received SSID: ");
            Serial.println(ssid);
        }
        else if (pCharacteristic->getUUID().toString() == PASS_CHAR_UUID)
        {
            strncpy(password, value.c_str(), sizeof(password));
            Serial.print("Received Password: ");
            Serial.println(password);
        }
    }
};

void setup_BLE()
{
    BLEDevice::init("ESP32");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pCharacteristicSSID = pService->createCharacteristic(
        SSID_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    BLECharacteristic *pCharacteristicPassword = pService->createCharacteristic(
        PASS_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    pCharacteristicSSID->setCallbacks(new MyBLECallbacks());
    pCharacteristicPassword->setCallbacks(new MyBLECallbacks());

    pService->start();
    pServer->getAdvertising()->start();

    uint8_t btMac[6];
    esp_read_mac(btMac, ESP_MAC_BT);

    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  btMac[0], btMac[1], btMac[2],
                  btMac[3], btMac[4], btMac[5]);

    Serial.println("Waiting for a client connection to write SSID and password...");
}