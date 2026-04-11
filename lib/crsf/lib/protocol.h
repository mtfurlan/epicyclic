#ifndef CRSF_PROTOCOL_H
#define CRSF_PROTOCOL_H
// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md
// TODO maybe look at https://github.com/ExpressLRS/ExpressLRS/blob/master/src/lib/CrsfProtocol/crsf_protocol.h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

// from ELRS
#define CRSF_CHANNEL_VALUE_EXT_MIN 0   // 880us with E.Limits on (-121.1%)
#define CRSF_CHANNEL_VALUE_MIN     172 // 987us - actual CRSF min is 0 with E.Limits on
#define CRSF_CHANNEL_VALUE_1000    191
#define CRSF_CHANNEL_VALUE_MID     992 // 1500us
#define CRSF_CHANNEL_VALUE_2000    1792
#define CRSF_CHANNEL_VALUE_MAX     1811 // 2012us - actual CRSF max is 1984 with E.Limits on
#define CRSF_CHANNEL_VALUE_EXT_MAX 1984 // 2120us with E.Limits on (+121.1%)
#define CRSF_TICKS_TO_US(x)        ((x - CRSF_CHANNEL_VALUE_MID) * 5 / 8 + 1500)
#define US_TO_CRSF_TICKS(x)        ((x - 1500) * 8 / 5 + CRSF_CHANNEL_VALUE_MID)

// Center (1500µs) = 992
#define CRSF_BAUDRATE        420000 // for receiver only
#define CRSF_NUM_CHANNELS    16
#define CRSF_MAX_PACKET_SIZE 64 // max declared len is 62+SYNC+LEN on top of that = 64
#define CRSF_MAX_PAYLOAD_LEN \
    (CRSF_MAX_PACKET_SIZE - 4) // Max size of payload in [sync] [len] [type] [payload] [crc8]

typedef enum : uint8_t {
    CSRF_FRAMETYPE_GPS = 0x02,
    CSRF_FRAMETYPE_GPS_TIME = 0x03,
    CSRF_FRAMETYPE_GPS_EXTENDED = 0x06,
    CSRF_FRAMETYPE_VARIOMETER_SENSOR = 0x07,
    CSRF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CSRF_FRAMETYPE_BAROMETRIC_ALTITUDE_VERTICAL_SPEED = 0x09,
    CSRF_FRAMETYPE_AIRSPEED = 0x0A,
    CSRF_FRAMETYPE_HEARTBEAT = 0x0B,
    CSRF_FRAMETYPE_RPM = 0x0C,
    CSRF_FRAMETYPE_TEMP = 0x0D,
    CSRF_FRAMETYPE_VOLTAGES = 0x0E,
    CSRF_FRAMETYPE_DISCONTINUED = 0x0F,
    CSRF_FRAMETYPE_VTX_TELEMETRY = 0x10,
    CSRF_FRAMETYPE_BAROMETER = 0x11,
    CSRF_FRAMETYPE_MAGNETOMETER = 0x12,
    CSRF_FRAMETYPE_ACCEL_GYRO = 0x13,
    CSRF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD = 0x16,
    CSRF_FRAMETYPE_SUBSET_RC_CHANNELS_PACKED = 0x17,
    CSRF_FRAMETYPE_RESERVED_RC_CHANNELS_PACKED_11_BITS = 0x18,
    CSRF_FRAMETYPE_RESERVED_CROSSFIRE_1 = 0x19,
    CSRF_FRAMETYPE_RESERVED_CROSSFIRE_2 = 0x1A,
    CSRF_FRAMETYPE_RESERVED_CROSSFIRE_3 = 0x1B,
    CSRF_FRAMETYPE_LINK_STATISTICS_RX = 0x1C,
    CSRF_FRAMETYPE_LINK_STATISTICS_TX = 0x1D,
    CSRF_FRAMETYPE_ATTITUDE = 0x1E,
    CSRF_FRAMETYPE_MAVLINK_FC = 0x1F,
    CSRF_FRAMETYPE_FLIGHT_MODE = 0x21,
    CSRF_FRAMETYPE_ESP_NOW_MESSAGES = 0x22,
    CSRF_FRAMETYPE_RESERVED_1 = 0x27,
    // extended header hereafter
    CSRF_FRAMETYPE_PARAMETER_PING_DEVICES = 0x28,
    CSRF_FRAMETYPE_PARAMETER_DEVICE_INFORMATION = 0x29,
    CSRF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CSRF_FRAMETYPE_PARAMETER_SETTINGS_READ = 0x2C,
    CSRF_FRAMETYPE_PARAMETER_VALUE_WRITE = 0x2D,
    CSRF_FRAMETYPE_DIRECT_COMMANDS = 0x32,
    CSRF_FRAMETYPE_LOGGING = 0x34,
    CSRF_FRAMETYPE_RESERVED_2 = 0x36,
    CSRF_FRAMETYPE_RESERVED_3 = 0x38,
    CSRF_FRAMETYPE_REMOTE_RELATED_FRAMES = 0x3A,
    CSRF_FRAMETYPE_GAME = 0x3C,
    CSRF_FRAMETYPE_RESERVED_4 = 0x3E,
    CSRF_FRAMETYPE_RESERVED_5 = 0x40,
    CSRF_FRAMETYPE_RESERVED_KISSFC_1 = 0x78,
    CSRF_FRAMETYPE_RESERVED_KISSFC_2 = 0x79,
    CSRF_FRAMETYPE_MSP_REQUEST = 0x7A,
    CSRF_FRAMETYPE_MSP_REPONSE = 0x7B,
    CSRF_FRAMETYPE_RESERVED_ARDUPILOT_LEGACY = 0x7F,
    CSRF_FRAMETYPE_RESERVED_ARDUPILOT_PASSTHROUGH_FRAME = 0x80,
    CSRF_FRAMETYPE_RESERVED_MLRS_1 = 0x81,
    CSRF_FRAMETYPE_RESERVED_MLRS_2 = 0x82,
    CSRF_FRAMETYPE_CRSF_MAVLINK_ENVELOPE = 0xAA,
    CSRF_FRAMETYPE_CRSF_MAVLINK_SYSTEM_STATUS_SENSOR = 0xAC,
} crsf_frame_type_e;

#define CRSF_TYPE_EXTENDED(t) (t >= 0x28)

typedef enum : uint8_t {
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_CLOUD = 0x0E,
    CRSF_ADDRESS_USB_DEVICE = 0x10,
    CRSF_ADDRESS_BLUETOOTH_WIFI = 0x12,
    CRSF_ADDRESS_WIFI_RECEIVER = 0x13, // mobile game/simulator
    CRSF_ADDRESS_VIDEO_RECEIVER = 0x14,
    // 0x20-0x7F Dynamic address space for NAT
    CRSF_ADDRESS_OSD = 0x80, // OSD / TBS CORE PNP PRO
    CRSF_ADDRESS_ESC_1 = 0x90,
    CRSF_ADDRESS_ESC_2 = 0x91,
    CRSF_ADDRESS_ESC_3 = 0x92,
    CRSF_ADDRESS_ESC_4 = 0x93,
    CRSF_ADDRESS_ESC_5 = 0x94,
    CRSF_ADDRESS_ESC_6 = 0x95,
    CRSF_ADDRESS_ESC_7 = 0x96,
    CRSF_ADDRESS_ESC_8 = 0x97,
    CRSF_ADDRESS_RESERVED_1 = 0x8A,
    CRSF_ADDRESS_CROSSFIRE_RESERVED_1 = 0xB0,
    CRSF_ADDRESS_CROSSFIRE_RESERVED_2 = 0xB2,
    CRSF_ADDRESS_VOLTAGE_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED_2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_VTX = 0xCE,
    CRSF_ADDRESS_CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE,
    CRSF_ADDRESS_RESERVED_3 = 0xF0,
    CRSF_ADDRESS_RESERVED_4 = 0xF2,
} crsf_addr_e;

#define CRSF_SERIAL_SYNC_BYTE 0XC8

#define CRSF_VALID_ADDRESS(addr)                                                               \
    (addr == CRSF_ADDRESS_BROADCAST || addr == CRSF_ADDRESS_CLOUD                              \
     || addr == CRSF_ADDRESS_USB_DEVICE || addr == CRSF_ADDRESS_BLUETOOTH_WIFI                 \
     || addr == CRSF_ADDRESS_WIFI_RECEIVER || addr == CRSF_ADDRESS_VIDEO_RECEIVER              \
     || (addr >= 0x20 && addr <= 0x7F) || addr == CRSF_ADDRESS_OSD                             \
     || (addr >= 0x91 && addr <= 0x97) || addr == CRSF_ADDRESS_RESERVED_1                      \
     || addr == CRSF_ADDRESS_CROSSFIRE_RESERVED_1 || addr == CRSF_ADDRESS_CROSSFIRE_RESERVED_2 \
     || addr == CRSF_ADDRESS_VOLTAGE_CURRENT_SENSOR || addr == CRSF_ADDRESS_GPS                \
     || addr == CRSF_ADDRESS_TBS_BLACKBOX || addr == CRSF_ADDRESS_FLIGHT_CONTROLLER            \
     || addr == CRSF_ADDRESS_RESERVED_2 || addr == CRSF_ADDRESS_RACE_TAG                       \
     || addr == CRSF_ADDRESS_VTX || addr == CRSF_ADDRESS_CRSF_ADDRESS_RADIO_TRANSMITTER        \
     || addr == CRSF_ADDRESS_CRSF_ADDRESS_CRSF_RECEIVER                                        \
     || addr == CRSF_ADDRESS_CRSF_ADDRESS_CRSF_TRANSMITTER || addr == CRSF_ADDRESS_RESERVED_3  \
     || addr == CRSF_ADDRESS_RESERVED_4)
#define CRSF_SYNC_BYTE(input) (input == CRSF_SERIAL_SYNC_BYTE || CRSF_VALID_ADDRESS(input))
typedef struct {
    uint8_t sync_byte; //
    uint8_t frame_size; // counts size after this byte, so it must be the payload size + 2 (type and crc)
    crsf_frame_type_e type : 8; // from crsf_frame_type_e
} PACKED crsf_header_t;


typedef struct {
    int32_t latitude;     // degree / 10`000`000
    int32_t longitude;    // degree / 10`000`000
    uint16_t groundspeed; // km/h / 100
    uint16_t heading;     // degree / 100
    uint16_t altitude;    // meter - 1000m offset
    uint8_t satellites;   // # of sats in view
} PACKED crsf_gps_t;

// This frame is needed for synchronization with the ublox time pulse. The maximum offset of time is +/-10ms.
typedef struct {
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
} PACKED crsf_gps_time_t;

typedef struct {
    uint8_t fix_type;      // Current GPS fix quality
    int16_t n_speed;       // Northward (north = positive) Speed [cm/sec]
    int16_t e_speed;       // Eastward (east = positive) Speed [cm/sec]
    int16_t v_speed;       // Vertical (up = positive) Speed [cm/sec]
    int16_t h_speed_acc;   // Horizontal Speed accuracy cm/sec
    int16_t track_acc;     // Heading accuracy in degrees scaled with 1e-1 degrees times 10)
    int16_t alt_ellipsoid; // Meters Height above GPS Ellipsoid (not MSL)
    int16_t h_acc;         // horizontal accuracy in cm
    int16_t v_acc;         // vertical accuracy in cm
    uint8_t reserved;
    uint8_t hDOP; // Horizontal dilution of precision,Dimensionless in nits of.1.
    uint8_t vDOP; // vertical dilution of precision, Dimensionless in nits of .1.
} PACKED crsf_gps_extended_t;

typedef struct {
    int16_t v_speed; // Vertical speed cm/s
} PACKED crsf_variometer_sensor_t;

typedef struct {
    // TODO: TBS spec says voltage and current are signed?
    // TODO: TBS spec says 10 μV, other places says dV
    uint16_t voltage;            // Voltage (LSB = 10 µV)
    uint16_t current;            // Current (LSB = 10 µA)
    uint32_t capacity_used : 24; // Capacity used (mAh)
    uint8_t remaining;           // Battery remaining (percent)
} PACKED crsf_battery_sensor_t;


// Due to OpenTX counts any 0xFFFF value as incorrect,
// the maximum sending value is limited to 0xFFFE (32766 meters)
uint16_t get_altitude_packed(int32_t altitude_dm);

// vertical speed is represented in cm/s with logarithmic scale and (un)packed by functions:
// Such constants give ±2500cm/s range and 3cm/s precision at low speeds and 70cm/s precision at speed about 25m/s;
int8_t get_vertical_speed_packed(int16_t vertical_speed_cm_s);

int16_t get_vertical_speed_cm_s(int8_t vertical_speed_packed);

typedef struct {
    uint16_t altitude_packed;     // Altitude above start (calibration) point
                                  // See description below.
    int8_t vertical_speed_packed; // vertical speed. See description below.
} PACKED crsf_barometric_altitude_vertical_speed_t;

typedef struct {
    uint16_t speed; // Airspeed in 0.1 * km/h (hectometers/h)
} PACKED crsf_airspeed_t;

typedef struct {
    int16_t origin_address; // Origin Device address
} PACKED crsf_heartbeat_t;

// Frame type used to transmit RPM (revolutions per minute) telemetry data from the craft to the transmitter. This frame can be used to report motor or propeller RPM for monitoring performance or diagnostics.
typedef struct {
    int32_t value : 24;
} int24_thing_t;
typedef struct {
    uint8_t rpm_source_id; // Identifies the source of the RPM data (e.g., 0 = Motor 1, 1 = Motor 2, etc.)
    int24_thing_t rpm_value
            [18]; // 1 - 19 RPM values with negative ones representing the motor spinning in reverse
} PACKED crsf_rpm_t;


// Frame type used to transmit temperature telemetry data from the vehicle to the transmitter. This frame can be used to report temperature readings from various sources on the vehicle, such as motors, ESCs, or the environment.
typedef struct {
    uint8_t temp_source_id; // Identifies the source of the temperature data (e.g., 0 = FC including all ESCs, 1 = Ambient, etc.)
    int16_t temperature
            []; // up to 20 temperature values in deci-degree (tenths of a degree) Celsius (e.g., 250 = 25.0°C, -50 = -5.0°C)
} PACKED crsf_temp_t;


/**
 * Used to transmit voltage telemetry from the craft to the transmitter. Can be used to report battery cell voltages, or a group of associated voltages from a single source.
 *
 * Interpretation of the type of voltages is dependent on the source_id selected for reporting:
 * - 0..127: Interpret as cell voltages of a single battery, up to 29S. Multiple batteries may be reported using multiple 0x0E frames with different source_ids. e.g. 0 = battery 1 cells, 1 = battery 2 cells, etc,
 * - 128..255: Interpret as general voltages measured from a single source. For example, an ESC might report incoming voltage, BEC output voltage, and MCU voltage as a single source_id and use additional source_ids for reporting multiple ESCs. e.g. 128 = ESC 1, 129 = ESC 2, etc
 */
typedef struct {
    uint8_t Voltage_source_id; // source of the voltages
    uint16_t Voltage_values[]; // Up to 29 voltages in millivolts (e.g. 3.850V = 3850)
} PACKED crsf_voltages_t;


typedef struct {
    uint8_t origin_address;
    uint8_t power_dBm;           // VTX power in dBm
    uint16_t frequency_MHz;      // VTX frequency in MHz
    uint8_t pit_mode        : 1; // 0=Off, 1=On
    uint8_t pitmode_control : 2; // 0=Off, 1=On, 2=Switch, 3=Failsafe
    uint8_t pitmode_switch  : 4; // 0=Ch5, 1=Ch5 Inv, … , 15=Ch12 Inv
} PACKED crsf_vtx_telemetry_t;

typedef struct {
    int32_t pressure_pa; // Pascals
    int32_t baro_temp;   // centidegrees
} PACKED crsf_barometer_t;

typedef struct {
    int16_t field_x; // milligauss * 3
    int16_t field_y; // milligauss * 3
    int16_t field_z; // milligauss * 3
} PACKED crsf_magnetometer_t;


/**
 * Raw accel and gyro data in NEU bodyframe, samples are raw data averaged over the sample interval
 *
 * Accel: +ve X = foward
 *        +ve Y = right
 *        +ve Z = up
 * Gyro:  +ve X = roll left
 *        +ve Y = pitch up
 *        +ve Z = yaw clockwise
 */
typedef struct {
    uint32_t sample_time; // Timestamp of the sample in us
    int16_t gyro_x;       // LSB = INT16_MAX/2000 DPS
    int16_t gyro_y;       // LSB = INT16_MAX/2000 DPS
    int16_t gyro_z;       // LSB = INT16_MAX/2000 DPS
    int16_t acc_x;        // LSB = INT16_MAX/16 G
    int16_t acc_y;        // LSB = INT16_MAX/16 G
    int16_t acc_z;        // LSB = INT16_MAX/16 G
    int16_t gyro_temp;    // centidegrees
} PACKED crsf_accel_gyro_t;

// Uplink is the connection from the ground to the UAV and downlink the opposite direction
typedef struct {
    uint8_t up_rssi_ant1;      // Uplink RSSI Antenna 1 (dBm * -1)
    uint8_t up_rssi_ant2;      // Uplink RSSI Antenna 2 (dBm * -1)
    uint8_t up_link_quality;   // Uplink Package success rate / Link quality (%)
    int8_t up_snr;             // Uplink SNR (dB)
    uint8_t active_antenna;    // number of currently best antenna
    uint8_t rf_profile;        // enum {4fps = 0 , 50fps, 150fps}
    uint8_t up_rf_power;       // enum {0mW = 0, 10mW, 25mW, 100mW,
                               // 500mW, 1000mW, 2000mW, 250mW, 50mW}
    uint8_t down_rssi;         // Downlink RSSI (dBm * -1)
    uint8_t down_link_quality; // Downlink Package success rate / Link quality (%)
    int8_t down_snr;           // Downlink SNR (dB)
} PACKED crsf_link_statistics_t;


// 16 channels packed into 22 bytes. In case of a Failsafe, this frame will no longer be sent (when the failsafe type is set to "cut"). It is recommended to wait for 1 second before starting the FC failsafe routine.

// TODO TBS spec says these are signed
typedef struct {
    unsigned channel_1  : 11;
    unsigned channel_2  : 11;
    unsigned channel_3  : 11;
    unsigned channel_4  : 11;
    unsigned channel_5  : 11;
    unsigned channel_6  : 11;
    unsigned channel_7  : 11;
    unsigned channel_8  : 11;
    unsigned channel_9  : 11;
    unsigned channel_10 : 11;
    unsigned channel_11 : 11;
    unsigned channel_12 : 11;
    unsigned channel_13 : 11;
    unsigned channel_14 : 11;
    unsigned channel_15 : 11;
    unsigned channel_16 : 11;
} PACKED crsf_rc_channels_packed_payload_t;


typedef struct {
    uint8_t rssi_db;      // RSSI (dBm * -1)
    uint8_t rssi_percent; // RSSI in percent
    uint8_t link_quality; // Package success rate / Link quality (%)
    int8_t snr;           // SNR (dB)
    uint8_t rf_power_db;  // rf power in dBm
} PACKED crsf_link_statistics_rx_t;

typedef struct {
    uint8_t rssi_db;      // RSSI (dBm * -1)
    uint8_t rssi_percent; // RSSI in percent
    uint8_t link_quality; // Package success rate / Link quality (%)
    int8_t snr;           // SNR (dB)
    uint8_t rf_power_db;  // rf power in dBm
    uint8_t fps;          // rf frames per second (fps / 10)
} PACKED crsf_link_statistics_tx_t;


// Angle values must be in -180° +180° range!
typedef struct {
    int16_t pitch; // Pitch angle (LSB = 100 µrad)
    int16_t roll;  // Roll angle  (LSB = 100 µrad)
    int16_t yaw;   // Yaw angle   (LSB = 100 µrad)
} PACKED crsf_attitude_t;

/**
 * Official MAVLink Documentation:
 * - [MAV_MODE_FLAG enum](https://mavlink.io/en/messages/common.html#MAV_MODE_FLAG)
 * - [MAV_AUTOPILOT enum](https://mavlink.io/en/messages/common.html#MAV_AUTOPILOT)
 * - [MAV_TYPE enum](https://mavlink.io/en/messages/common.html#MAV_TYPE)
 */
typedef struct {
    int16_t airspeed;
    uint8_t base_mode;      // vehicle mode flags, defined in MAV_MODE_FLAG enum
    uint32_t custom_mode;   // autopilot-specific flags
    uint8_t autopilot_type; // FC type; defined in MAV_AUTOPILOT enum
    uint8_t firmware_type;  // vehicle type; defined in MAV_TYPE enum
} PACKED crsf_mavlink_fc_t;


typedef char* crsf_flight_mode_t;

typedef struct {
    uint8_t VAL1;       // Used for Seat Position of the Pilot
    uint8_t VAL2;       // Used for the Current Pilots Lap
    char VAL3[15];      // 15 characters for the lap time current/split
    char VAL4[15];      // 15 characters for the lap time current/split
    char FREE_TEXT[20]; // Free text of 20 character at the bottom of the screen
} PACKED crsf_esp_now_messages_t;


//// ============================================== EXTENDED ===================


// The host can ping a specific device (destination node address of device) or all devices (destination node address 0x00 Broadcast address) and they will answer with the [Parameter device information frame](#0x29-parameter-device-information). The frame has no payload.
typedef struct {
} PACKED crsf_parameter_ping_devices_t;


typedef struct {
    crsf_header_t header;
    union {
        uint8_t raw[CRSF_MAX_PAYLOAD_LEN];
        crsf_gps_t gps;
        crsf_gps_time_t gps_time;
        crsf_gps_extended_t gps_extended;
        crsf_variometer_sensor_t variometer_sensor;
        crsf_battery_sensor_t battery_sensor;
        crsf_barometric_altitude_vertical_speed_t barometric_altitude_vertical_speed;
        crsf_airspeed_t airspeed;
        crsf_heartbeat_t heartbeat;
        crsf_rpm_t rpm;
        crsf_temp_t temp;
        crsf_voltages_t voltages;
        crsf_vtx_telemetry_t vtx_telemetry;
        crsf_barometer_t barometer;
        crsf_magnetometer_t magnetometer;
        crsf_accel_gyro_t accel_gyro;
        crsf_link_statistics_t link_statistics;
        crsf_rc_channels_packed_payload_t rc_channels_packed_payload;
        // > This frame is discouraged for implementation. Revision is in progress.
        // crsf_subset_rc_channels_packed_t subset_rc_channels_packed;
        crsf_link_statistics_rx_t link_statistics_rx;
        crsf_link_statistics_tx_t link_statistics_tx;
        crsf_attitude_t attitude;
        crsf_mavlink_fc_t mavlink_fc;
        crsf_flight_mode_t flight_mode;
        crsf_esp_now_messages_t esp_now_messages;
    };
} PACKED crsf_packet_t;

// not implemented cause it seemed hard
// typedef struct {
//     crsf_header_t header;
//     crsf_addr_e destination : 8;
//     crsf_addr_e source : 8;
//     union {
//         uint8_t raw[CRSF_MAX_PAYLOAD_LEN];
//         crsf_parameter_ping_devices_t parameter_ping_devices;
//         crsf_parameter_device_information_t parameter_device_information;
//         crsf_parameter_settings_entry_t parameter_settings_entry;
//         crsf_parameter_settings_read_t parameter_settings_read;
//         crsf_parameter_value_write_t parameter_value_write;
//         crsf_direct_commands_t direct_commands;
//         crsf_logging_t logging;
//         crsf_remote_related_frames_t remote_related_frames;
//         crsf_game_t game;
//         crsf_reserved_kissfc_1_t reserved_kissfc_1;
//         crsf_reserved_kissfc_2_t reserved_kissfc_2;
//         crsf_msp_request_t msp_request;
//         crsf_msp_reponse_t msp_reponse;
//         crsf_reserved_ardupilot_legacy_t reserved_ardupilot_legacy;
//         crsf_reserved_ardupilot_passthrough_frame_t reserved_ardupilot_passthrough_frame;
//         crsf_reserved_mlrs_1_t reserved_mlrs_1;
//         crsf_reserved_mlrs_2_t reserved_mlrs_2;
//         crsf_crsf_mavlink_envelope_t crsf_mavlink_envelope;
//         crsf_crsf_mavlink_system_status_sensor_t crsf_mavlink_system_status_sensor;
//     }
// } PACKED crsf_packet_extended_t;

uint8_t crc8(const uint8_t* ptr, uint8_t len);
#endif //CRSF_PROTOCOL_H
