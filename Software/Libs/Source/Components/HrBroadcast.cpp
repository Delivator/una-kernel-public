/**
 ******************************************************************************
 * @file    HrBroadcast.cpp
 * @brief   Broadcasts the watch's heart rate over BLE, as a heart-rate sensor.
 ******************************************************************************
 *
 ******************************************************************************
 */

#include "HrBroadcast.hpp"
#include "App/AccessoryManager.hpp"

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
    mMutex.init("HrBroadcast");
}

HrBroadcast::~HrBroadcast()
{
    stop();
}

HrBroadcast::Result HrBroadcast::start()
{
    OS::MutexCS cs(mMutex);

    if (mActive.load()) {
        return Result::STARTED;
    }

    // AccessoryManager marks itself active before it asks for the broadcast
    // to stop (Backend::onAccessoryAcquire), so under the mutex either this
    // sees the strap or that stop() comes after and ends the broadcast.
    if (App::AccessoryManager::getInstance().isActive()) {
        LOG_INFO("Refused: a strap is in use\n");
        return Result::STRAP_IN_USE;
    }

    if (!mBle.setSensorMode(true)) {
        LOG_ERROR("BLE sensor mode failed\n");
        mBle.setSensorMode(false);
        return Result::FAILED;
    }

    if (!mHr.connect()) {
        LOG_ERROR("Heart rate sensor not available\n");
        mBle.setSensorMode(false);
        return Result::FAILED;
    }

    mActive.store(true);
    LOG_INFO("Started\n");
    return Result::STARTED;
}

bool HrBroadcast::stop()
{
    OS::MutexCS cs(mMutex);

    if (!mActive.exchange(false)) {
        return false;
    }

    mHr.disconnect();
    mBle.setSensorMode(false);
    LOG_INFO("Stopped\n");
    return true;
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
    if (hr.getTrustLevel() < skMinTrustLevel) {
        // Not a reading yet (settling, or loose on the wrist): a strap sends
        // no contact then, rather than a guess
        bpm = 0.0f;
    } else if (!(bpm > 0.0f)) {
        bpm = 0.0f;         // Also catches NaN
    } else if (bpm > 255.0f) {
        bpm = 255.0f;
    }

    mService.publish(static_cast<uint8_t>(bpm + 0.5f));
}
