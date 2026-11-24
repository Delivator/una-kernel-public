/**
 ******************************************************************************
 * @file    HrsMeasurement.hpp
 * @brief   Encodes the Heart Rate Measurement characteristic (0x2A37).
 ******************************************************************************
 * The server side of what Ble::ProfileHrs parses: the watch's own heart rate,
 * as a heart-rate strap would send it. Pure logic, no BlueNRG/OS deps, so it
 * is exercised directly by host unit tests.
 ******************************************************************************
 */

#ifndef __COMPONENT_BLE_SERVICE_HRS_MEASUREMENT_HPP
#define __COMPONENT_BLE_SERVICE_HRS_MEASUREMENT_HPP

#include <cstdint>

namespace Ble
{

/**
 * @brief   Heart Rate Measurement (0x2A37) encoder.
 */
struct HrsMeasurement {

    static constexpr uint16_t kServiceUuid         = 0x180D;
    static constexpr uint16_t kMeasurementUuid     = 0x2A37;
    static constexpr uint16_t kBodySensorLocUuid   = 0x2A38;

    // Body Sensor Location (0x2A38) value for a watch.
    static constexpr uint8_t kBodySensorLocWrist   = 0x02;

    // Flags, first byte. The BPM is always sent in the 8-bit format.
    static constexpr uint8_t kFlagContactDetected  = 0x02;
    static constexpr uint8_t kFlagContactSupported = 0x04;

    // Flags + 8-bit BPM. No energy expended, no RR intervals.
    static constexpr uint8_t kSize = 2;

    /**
     * @brief   Encode one measurement.
     * @param   bpm: Heart rate from the sensor layer; 0 means no valid reading
     *          (off the wrist, warming up, or lost).
     * @param   out: Destination, at least kSize bytes.
     * @retval  Number of bytes written (kSize).
     *
     * A valid reading goes out with sensor contact detected. No reading goes
     * out as 0 BPM with contact supported but not detected, the way a strap
     * reports being off the body, so a receiver shows a dropout rather than
     * holding the last value.
     */
    static uint8_t encode(uint8_t bpm, uint8_t out[kSize])
    {
        out[0] = kFlagContactSupported | (bpm != 0 ? kFlagContactDetected : 0);
        out[1] = bpm;
        return kSize;
    }
};

}  /* namespace Ble */

#endif /* __COMPONENT_BLE_SERVICE_HRS_MEASUREMENT_HPP */
