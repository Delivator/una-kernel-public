/**
 ******************************************************************************
 * @file    HrBroadcast.hpp
 * @brief   Broadcasts the watch's heart rate over BLE, as a heart-rate sensor.
 ******************************************************************************
 * While on, the watch leaves its phone link and puts BLE in sensor mode (see
 * Interface::IBle::setSensorMode()), so a PC, a bike computer or a training
 * app can connect and subscribe to the Heart Rate Service without pairing.
 * The heart rate comes from the sensor layer's HEART_RATE sensor (optical, or
 * a connected strap when the source arbiter prefers it), which keeps the
 * optical sensor running for as long as the broadcast is on.
 *
 * The broadcast is not persisted: the watch starts with its phone link.
 ******************************************************************************
 */

#ifndef __COMPONENT_HR_BROADCAST_HPP
#define __COMPONENT_HR_BROADCAST_HPP

#include <atomic>

#include "IBle.hpp"
#include "Services/HeartRateService.hpp"
#include "SensorLayer/Core/SensorConnection.hpp"
#include "SDK/Interfaces/ISensorDataListener.hpp"

class HrBroadcast : public SDK::Interface::ISensorDataListener {
public:

    /**
     * @brief   Constructor.
     * @param   ble: BLE instance.
     * @param   service: Heart Rate Service to publish to.
     */
    HrBroadcast(Interface::IBle &ble, Ble::HeartRateService &service);

    /**
     * @brief   Destructor. Stops the broadcast.
     */
    virtual ~HrBroadcast();

    /**
     * @brief   Start broadcasting: BLE sensor mode on, heart rate sensor on.
     * @retval  'true' - broadcasting, 'false' - failed (and left off).
     */
    bool start();

    /**
     * @brief   Stop broadcasting: heart rate sensor off, the phone link back.
     */
    void stop();

    /**
     * @brief   Check if the broadcast is on.
     */
    bool isActive() const { return mActive.load(); }

private:

    // Heart Rate Measurement once a second, like a strap
    static constexpr float skPeriodMs = 1000.0f;

    Interface::IBle &mBle;
    Ble::HeartRateService &mService;
    Sensor::Connection mHr;
    std::atomic<bool> mActive { false };

    /**
     * @copydoc SDK::Interface::ISensorDataListener::onSdlNewData()
     */
    virtual void onSdlNewData(uint16_t handle, const SDK::Sensor::Data *data,
            uint16_t count, uint16_t stride) override;
};

#endif /* __COMPONENT_HR_BROADCAST_HPP */
