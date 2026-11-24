/**
 ******************************************************************************
 * @file    HeartRateService.cpp
 * @brief   BLE Heart Rate Service (HRS) server.
 ******************************************************************************
 *
 ******************************************************************************
 */

#include "Services/HeartRateService.hpp"
#include "Services/HrsMeasurement.hpp"

#define LOG_MODULE_PRX      "BLE.SRV.HeartRate"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "Logger.h"

namespace Ble
{

HeartRateService::HeartRateService(Interface::IBle &ble) : mBle(ble)
{
    mSrv = mBle.addService(UUID(HrsMeasurement::kServiceUuid));

    // Heart Rate Measurement: notify only, as the specification requires
    mChrMeasurement = mBle.addCharacteristic(mSrv,
            UUID(HrsMeasurement::kMeasurementUuid),
            HrsMeasurement::kSize, CharProp::NOTIFY);
    mBle.setCharacteristicOpen(mChrMeasurement);
    mBle.attachCharacteristicCallback(mChrMeasurement, this);

    // Body Sensor Location: read only
    mChrBodySensorLocation = mBle.addCharacteristic(mSrv,
            UUID(HrsMeasurement::kBodySensorLocUuid), 1, CharProp::READ);
    mBle.setCharacteristicOpen(mChrBodySensorLocation);
    mBle.attachCharacteristicCallback(mChrBodySensorLocation, this);

    mBle.attachStatusCallback(this);
}

HeartRateService::~HeartRateService()
{
    mBle.detachStatusCallback(this);

    mBle.attachCharacteristicCallback(mChrBodySensorLocation, nullptr);
    mBle.delCharacteristic(mChrBodySensorLocation);

    mBle.attachCharacteristicCallback(mChrMeasurement, nullptr);
    mBle.delCharacteristic(mChrMeasurement);

    mBle.delService(mSrv);
}

void HeartRateService::publish(uint8_t bpm)
{
    if (!mNotify.load()) {
        return;
    }

    uint8_t value[HrsMeasurement::kSize];
    uint8_t len = HrsMeasurement::encode(bpm, value);
    if (!mBle.trySetCharacteristicValue(mChrMeasurement, value, len)) {
        LOG_DEBUG("Measurement dropped\n");
    }
}

void HeartRateService::onDisconnect(uint8_t reason)
{
    (void)reason;
    mNotify.store(false);
}

void HeartRateService::onNotifyIndicate(Ble::CharacteristicHandle chrHandle,
        bool notify, bool indicate)
{
    (void)indicate;

    if (chrHandle == mChrMeasurement) {
        mNotify.store(notify);
        LOG_INFO("Measurement notifications %s\n", notify ? "enabled" : "disabled");
    }
}

void HeartRateService::onRead(Ble::CharacteristicHandle chrHandle)
{
    if (chrHandle == mChrBodySensorLocation) {
        const uint8_t location = HrsMeasurement::kBodySensorLocWrist;
        mBle.setCharacteristicValue(mChrBodySensorLocation, &location, sizeof(location));
    }
}

}  /* namespace Ble */
