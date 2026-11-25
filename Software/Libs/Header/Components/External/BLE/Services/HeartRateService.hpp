/**
 ******************************************************************************
 * @file    HeartRateService.hpp
 * @brief   BLE Heart Rate Service (HRS) server.
 ******************************************************************************
 * Serves the watch's heart rate to a BLE central (a PC, a bike computer, a
 * training app) the way a heart-rate strap does. Its characteristics are open
 * (no encryption), because such centrals do not pair with a passkey: they are
 * reachable in the BLE sensor mode, where only they can connect (see
 * Interface::IBle::setSensorMode()).
 ******************************************************************************
 */

#ifndef __COMPONENT_BLE_SERVICE_HRS_HPP
#define __COMPONENT_BLE_SERVICE_HRS_HPP

#include <atomic>

#include "IBle.hpp"

namespace Ble
{

/**
 * @brief   BLE Heart Rate Service (HRS) server.
 */
class HeartRateService : virtual public Interface::IBle::StatusCallback,
        virtual public Interface::IBle::CharacteristicCallback {

public:

    /**
     * @brief   Constructor. Must run before the BLE stack is enabled.
     * @param   ble: BLE instance.
     */
    explicit HeartRateService(Interface::IBle &ble);

    /**
     * @brief   Destructor.
     */
    virtual ~HeartRateService();

    /**
     * @brief   Send a heart rate to the subscribed client, if there is one.
     * @note    Never blocks: the sample is dropped when the controller's TX
     *          pool is full, as the next one is a second away.
     * @param   bpm: Heart rate in BPM; 0 means no valid reading.
     */
    void publish(uint8_t bpm);

    /**
     * @brief   Check if a client has enabled measurement notifications.
     */
    bool isSubscribed() const { return mNotify.load(); }

private:

    Interface::IBle &mBle;

    ServiceHandle mSrv { };
    CharacteristicHandle mChrMeasurement { };
    CharacteristicHandle mChrBodySensorLocation { };

    // Set on the BLE event thread, read on the sensor thread
    std::atomic<bool> mNotify { false };

    /**
     * @copydoc Interface::IBle::StatusCallback::onDisconnect()
     */
    virtual void onDisconnect(uint8_t reason) override;

    /**
     * @copydoc Interface::IBle::CharacteristicCallback::onNotifyIndicate()
     */
    virtual void onNotifyIndicate(Ble::CharacteristicHandle chrHandle,
            bool notify, bool indicate) override;

    /**
     * @copydoc Interface::IBle::CharacteristicCallback::onRead()
     */
    virtual void onRead(Ble::CharacteristicHandle chrHandle) override;
};

}  /* namespace Ble */

#endif /* __COMPONENT_BLE_SERVICE_HRS_HPP */
