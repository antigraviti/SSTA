#include "ble_service.h"

// ============================================================================
// BLE GATT Service Implementation
// ONYX Bethlehem Labs + SSTA IP Holdings
// ============================================================================

BLEService* g_bleServicePtr = nullptr;

// --- Server Callbacks ---

void ServerCallbacks::onConnect(BLEServer* server) {
    if (g_bleServicePtr) g_bleServicePtr->onConnect();
}

void ServerCallbacks::onDisconnect(BLEServer* server) {
    if (g_bleServicePtr) g_bleServicePtr->onDisconnect();
    // Restart advertising so phone can reconnect
    BLEDevice::startAdvertising();
}

// --- Control Write Callback ---

void ControlWriteCallback::onWrite(BLECharacteristic* pChar) {
    if (!g_bleServicePtr) return;

    std::string val = pChar->getValue();
    if (val.length() >= sizeof(BLEControlCmd)) {
        BLEControlCmd cmd;
        memcpy(&cmd, val.data(), sizeof(BLEControlCmd));

        // Store for main loop to process
        // (Direct hardware control from BLE callback is unsafe)
        g_bleServicePtr->_newCommand = true;
        g_bleServicePtr->_lastCmd = cmd;
    }
}

// --- PID Write Callback ---

void PIDWriteCallback::onWrite(BLECharacteristic* pChar) {
    if (!g_bleServicePtr) return;

    std::string val = pChar->getValue();
    if (val.length() >= sizeof(BLEPIDPacket)) {
        BLEPIDPacket pid;
        memcpy(&pid, val.data(), sizeof(BLEPIDPacket));

        g_bleServicePtr->_newPID = true;
        g_bleServicePtr->_lastPID = pid;
    }
}

// --- BLEService Implementation ---

BLEService::BLEService()
    : _server(nullptr)
    , _charTemps(nullptr)
    , _charControl(nullptr)
    , _charStatus(nullptr)
    , _charPID(nullptr)
    , _connected(false)
    , _newCommand(false)
    , _newPID(false)
{
    _lastCmd = {};
    _lastPID = {};
    g_bleServicePtr = this;
}

void BLEService::init() {
    BLEDevice::init(DEVICE_NAME);

    _server = BLEDevice::createServer();
    _server->setCallbacks(new ServerCallbacks());

    // Create service
    ::BLEService* svc = _server->createService(BLEUUID(BLE_SERVICE_UUID), 20);

    // Temperature characteristic — notify only
    _charTemps = svc->createCharacteristic(
        BLEUUID(BLE_CHAR_TEMPS_UUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _charTemps->addDescriptor(new BLE2902());

    // Control characteristic — read/write
    _charControl = svc->createCharacteristic(
        BLEUUID(BLE_CHAR_CONTROL_UUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    _charControl->setCallbacks(new ControlWriteCallback());

    // Status characteristic — notify only
    _charStatus = svc->createCharacteristic(
        BLEUUID(BLE_CHAR_STATUS_UUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _charStatus->addDescriptor(new BLE2902());

    // PID tuning characteristic — read/write
    _charPID = svc->createCharacteristic(
        BLEUUID(BLE_CHAR_PID_UUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    _charPID->setCallbacks(new PIDWriteCallback());

    svc->start();

    // Start advertising
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);  // Connection interval hint
    BLEDevice::startAdvertising();
}

bool BLEService::isConnected() const {
    return _connected;
}

void BLEService::notifyTemperatures(const SensorReadings& readings) {
    if (!_connected || !_charTemps) return;

    BLETempPacket pkt;
    pkt.t1 = readings.temps[0];
    pkt.t2 = readings.temps[1];
    pkt.t3 = readings.temps[2];
    pkt.t4 = readings.temps[3];
    pkt.t5 = readings.temps[4];
    pkt.t6 = readings.temps[5];
    pkt.timestamp = (uint32_t)readings.timestamp;

    _charTemps->setValue((uint8_t*)&pkt, sizeof(pkt));
    _charTemps->notify();
}

void BLEService::notifyStatus(const BLEStatusPacket& status) {
    if (!_connected || !_charStatus) return;

    _charStatus->setValue((uint8_t*)&status, sizeof(status));
    _charStatus->notify();
}

bool BLEService::hasNewCommand() const {
    return _newCommand;
}

BLEControlCmd BLEService::getCommand() {
    _newCommand = false;
    return _lastCmd;
}

bool BLEService::hasNewPIDGains() const {
    return _newPID;
}

BLEPIDPacket BLEService::getPIDUpdate() {
    _newPID = false;
    return _lastPID;
}

void BLEService::onConnect() {
    _connected = true;
    Serial.println("[BLE] Client connected");
}

void BLEService::onDisconnect() {
    _connected = false;
    _newCommand = false;
    Serial.println("[BLE] Client disconnected");
}
