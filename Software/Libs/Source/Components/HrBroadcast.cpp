/**
 ******************************************************************************
 * @file    HrBroadcast.cpp
 * @brief   Broadcasts the watch's heart rate over BLE, as a heart-rate sensor.
 ******************************************************************************
 *
 ******************************************************************************
 */

#include "HrBroadcast.hpp"

#include "SDK/SensorLayer/SensorDataBatch.hpp"
#include "SDK/SensorLayer/DataParsers/SensorDataParserHeartRate.hpp"

#define LOG_MODULE_PRX      "HrBroadcast"
#define LOG_MODULE_LEVEL    LOG_LEVEL_INFO
#include "Logger.h"

HrBroadcast::HrBroadcast(Interface::IBle &ble, Ble::HeartRateService &service)
    : mBle(ble)
    , mService(service)
    , mHr(SDK::Sensor::Type::HEART_RATE, this, skPeriodMs)
{
}

HrBroadcast::~HrBroadcast()
{
    stop();
}

bool HrBroadcast::start()
{
    if (mActive.load()) {
        return true;
    }

    if (!mBle.setSensorMode(true)) {
        LOG_ERROR("BLE sensor mode failed\n");
        mBle.setSensorMode(false);
        return false;
    }

    if (!mHr.connect()) {
        LOG_ERROR("Heart rate sensor not available\n");
        mBle.setSensorMode(false);
        return false;
    }

    mActive.store(true);
    LOG_INFO("Started\n");
    return true;
}

void HrBroadcast::stop()
{
    if (!mActive.exchange(false)) {
        return;
    }

    mHr.disconnect();
    mBle.setSensorMode(false);
    LOG_INFO("Stopped\n");
}

void HrBroadcast::onSdlNewData(uint16_t handle, const SDK::Sensor::Data *data,
        uint16_t count, uint16_t stride)
{
    if (!mHr.matchesDriver(handle) || count == 0) {
        return;
    }

    // A batch can hold more than one sample: only the latest is current
    SDK::Sensor::DataBatch batch(data, count, stride);
    SDK::SensorDataParser::HeartRate hr(batch[count - 1]);

    float bpm = hr.getBpm();
    if (!(bpm > 0.0f)) {
        bpm = 0.0f;         // Also catches NaN
    } else if (bpm > 255.0f) {
        bpm = 255.0f;
    }

    mService.publish(static_cast<uint8_t>(bpm + 0.5f));
}
