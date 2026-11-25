/**
 * Host unit tests for Ble::HrsMeasurement — the Heart Rate Measurement
 * (0x2A37) encoder behind the watch's own Heart Rate Service. Checked against
 * the specification's byte layout and round-tripped through Ble::ProfileHrs,
 * the parser the watch uses for heart-rate straps.
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "Components/External/BLE/Services/HrsMeasurement.hpp"
#include "Components/External/BLE/Accessory/ProfileHrs.hpp"

namespace {

using Ble::AccessorySample;
using Ble::HrsMeasurement;
using Ble::ProfileHrs;

} // namespace

TEST(HrsMeasurement, UsesHeartRateServiceUuids)
{
    EXPECT_EQ(HrsMeasurement::kServiceUuid, 0x180D);
    EXPECT_EQ(HrsMeasurement::kMeasurementUuid, 0x2A37);
    EXPECT_EQ(HrsMeasurement::kBodySensorLocUuid, 0x2A38);
    EXPECT_EQ(HrsMeasurement::kBodySensorLocWrist, 0x02);
}

TEST(HrsMeasurement, ValidReadingIs8BitWithContactDetected)
{
    uint8_t out[HrsMeasurement::kSize] {};
    EXPECT_EQ(HrsMeasurement::encode(72, out), 2);

    // Flags: bit0 = 0 (8-bit BPM), bits 1-2 = 0b11 (contact supported and
    // detected), no energy expended (bit3), no RR intervals (bit4).
    EXPECT_EQ(out[0], 0x06);
    EXPECT_EQ(out[1], 72);
}

TEST(HrsMeasurement, NoReadingIsZeroWithContactNotDetected)
{
    uint8_t out[HrsMeasurement::kSize] {};
    EXPECT_EQ(HrsMeasurement::encode(0, out), 2);

    EXPECT_EQ(out[0], 0x04);    // contact supported, not detected
    EXPECT_EQ(out[1], 0);
}

TEST(HrsMeasurement, FullRangeFitsInEightBits)
{
    uint8_t out[HrsMeasurement::kSize] {};
    HrsMeasurement::encode(255, out);
    EXPECT_EQ(out[0] & 0x01, 0);  // never the 16-bit format
    EXPECT_EQ(out[1], 255);
}

TEST(HrsMeasurement, RoundTripsThroughStrapParser)
{
    for (unsigned bpm = 1; bpm <= 255; ++bpm) {
        uint8_t out[HrsMeasurement::kSize] {};
        uint8_t len = HrsMeasurement::encode(static_cast<uint8_t>(bpm), out);

        ProfileHrs parser;
        AccessorySample sample {};
        ASSERT_TRUE(parser.parseNotification(0, out, len, sample)) << bpm;
        EXPECT_EQ(sample.kind, AccessorySample::Kind::HeartRate);
        EXPECT_EQ(sample.value, bpm);
        EXPECT_EQ(sample.trust, 3) << "contact detected reads as good contact";
    }
}

TEST(HrsMeasurement, NoReadingIsRejectedByStrapParser)
{
    // A receiver must not take the dropout frame for a heart rate
    uint8_t out[HrsMeasurement::kSize] {};
    uint8_t len = HrsMeasurement::encode(0, out);

    ProfileHrs parser;
    AccessorySample sample {};
    EXPECT_FALSE(parser.parseNotification(0, out, len, sample));
}
