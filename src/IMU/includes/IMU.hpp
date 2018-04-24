//
// Created by Firsou on 23-Apr-18.
//

#ifndef GROUP9_IMU_HPP
#define GROUP9_IMU_HPP

#include <vector>
#include "Messages.hpp"
#include "cluon/OD4Session.hpp"

float const ACCEL_SENSITIVITY = 16384; // = 16384 LSB/g
float const GYRO_SENSITIVITY  = 131;   // = 131 LSB/degrees/sec

uint8_t const MPU9250_ADDRESS = 0x68;   /**< Bus address of Gyrometer     */
uint8_t const AK8963_ADDRESS = 0x76;    /**< Bus address of Accelerometer */

enum A_SCALE {
    AFS_2G = 0,
    AFS_4G,
    AFS_8G,
    AFS_16G
};

enum G_SCALE {
    GFS_250DPS = 0,
    GFS_500DPS,
    GFS_1000DPS,
    GFS_2000DPS
};

enum M_SCALE {
    MFS_14BITS = 0, // 0.6 mG per LSB
    MFS_16BITS      // 0.15 mG per LSB
};

enum M_MODE {
    M_8HZ = 0x02,  // 8 Hz update
    M_100HZ = 0x06 // 100 Hz continuous magnetometer
};

void setAscale(A_SCALE);
float getAscale();
void setGscale(G_SCALE);
float getGscale(bool);

// Specify sensor full scale
uint8_t m_gscale = GFS_250DPS;
uint8_t m_ascale = AFS_2G;
// Choose either 14-bit or 16-bit magnetometer resolution
uint8_t m_mscale = MFS_16BITS;
// 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
uint8_t m_mmode = M_100HZ;

/**
 * This function initializes the MPU Chip
 * @param deviceFile int16_t - value of file at specified device node
 * @param MPU9250_ADDRESS uint8_t - Bus address of IMU
 */
void initializeMPU(int16_t deviceFile, uint8_t MPU9250_ADDRESS);

/**
 * This function accesses the i2c device at the specified address
 * @param deviceFile int16_t - value of file at specified device node
 * @param addr uint8_t - the bus address of the i2c device
 * @return
 */
int8_t i2cAccessDevice(int16_t deviceFile, uint8_t const addr)

/**
 * This functions reads the register at the specified bus address
 * @param deviceFile int16_t value of file at specified device node
 * @param addr uint8_t - the bus address of the i2c device
 * @param data
 * @param length
 * @return
 */
int8_t i2cReadRegister(int16_t deviceFile, uint8_t const addr, uint8_t *data, uint8_t const length);

/**
 * This function writes to the register at the specified bus address
 * @param a_data
 * @param deviceFile int16_t - value of file at specified device node
 * @return
 */
int8_t i2cWriteRegister(std::vector<uint8_t> a_data, int16_t deviceFile);

namespace MPU9250 {
    enum RegAddr {
        //I2C bus and address definitions for Robotics Cape
                IMU_ADDR = 0x68,
        IMU_BUS = 2,

        // internal DMP sample rate limits
                DMP_MAX_RATE = 200,
        DMP_MIN_RATE = 4,
        IMU_POLL_TIMEOUT = 300, // milliseconds
        MAX_FIFO_BUFFER = 128,

        SELF_TEST_X_GYRO = 0x00,
        SELF_TEST_Y_GYRO = 0x01,
        SELF_TEST_Z_GYRO = 0x02,
        X_FINE_GAIN      = 0x03,
        Y_FINE_GAIN      = 0x04,
        Z_FINE_GAIN      = 0x05,
        SELF_TEST_X_ACCEL = 0x0D,
        SELF_TEST_Y_ACCEL = 0x0E,
        SELF_TEST_Z_ACCEL = 0x0F,
        SELF_TEST_A      = 0x10,
        XG_OFFSET_H      = 0x13,
        XG_OFFSET_L      = 0x14,
        YG_OFFSET_H      = 0x15,
        YG_OFFSET_L      = 0x16,
        ZG_OFFSET_H      = 0x17,
        ZG_OFFSET_L      = 0x18,
        SMPLRT_DIV       = 0x19,
        CONFIG           = 0x1A,
        GYRO_CONFIG      = 0x1B,
        ACCEL_CONFIG     = 0x1C,
        ACCEL_CONFIG2    = 0x1D,
        LP_ACCEL_ODR     = 0x1E,
        WOM_THR          = 0x1F,
        MOT_DUR          = 0x20,
        ZMOT_THR         = 0x21,
        ZRMOT_DUR        = 0x22,
        FIFO_EN          = 0x23,
        I2C_MST_CTRL     = 0x24,
        I2C_SLV0_ADDR    = 0x25,
        I2C_SLV0_REG     = 0x26,
        I2C_SLV0_CTRL    = 0x27,
        I2C_SLV1_ADDR    = 0x28,
        I2C_SLV1_REG     = 0x29,
        I2C_SLV1_CTRL    = 0x2A,
        I2C_SLV2_ADDR    = 0x2B,
        I2C_SLV2_REG     = 0x2C,
        I2C_SLV2_CTRL    = 0x2D,
        I2C_SLV3_ADDR    = 0x2E,
        I2C_SLV3_REG     = 0x2F,
        I2C_SLV3_CTRL    = 0x30,
        I2C_SLV4_ADDR    = 0x31,
        I2C_SLV4_REG     = 0x32,
        I2C_SLV4_DO      = 0x33,
        I2C_SLV4_CTRL    = 0x34,
        I2C_SLV4_DI      = 0x35,
        I2C_MST_STATUS   = 0x36,
        INT_PIN_CFG      = 0x37,
        INT_ENABLE       = 0x38,
        DMP_INT_STATUS   = 0x39,
        INT_STATUS       = 0x3A,
        ACCEL_XOUT_H     = 0x3B,
        ACCEL_XOUT_L     = 0x3C,
        ACCEL_YOUT_H     = 0x3D,
        ACCEL_YOUT_L     = 0x3E,
        ACCEL_ZOUT_H     = 0x3F,
        ACCEL_ZOUT_L     = 0x40,
        TEMP_OUT_H       = 0x41,
        TEMP_OUT_L       = 0x42,
        GYRO_XOUT_H      = 0x43,
        GYRO_XOUT_L      = 0x44,
        GYRO_YOUT_H      = 0x45,
        GYRO_YOUT_L      = 0x46,
        GYRO_ZOUT_H      = 0x47,
        GYRO_ZOUT_L      = 0x48,
        EXT_SENS_DATA_00 = 0x49,
        EXT_SENS_DATA_01 = 0x4A,
        EXT_SENS_DATA_02 = 0x4B,
        EXT_SENS_DATA_03 = 0x4C,
        EXT_SENS_DATA_04 = 0x4D,
        EXT_SENS_DATA_05 = 0x4E,
        EXT_SENS_DATA_06 = 0x4F,
        EXT_SENS_DATA_07 = 0x50,
        EXT_SENS_DATA_08 = 0x51,
        EXT_SENS_DATA_09 = 0x52,
        EXT_SENS_DATA_10 = 0x53,
        EXT_SENS_DATA_11 = 0x54,
        EXT_SENS_DATA_12 = 0x55,
        EXT_SENS_DATA_13 = 0x56,
        EXT_SENS_DATA_14 = 0x57,
        EXT_SENS_DATA_15 = 0x58,
        EXT_SENS_DATA_16 = 0x59,
        EXT_SENS_DATA_17 = 0x5A,
        EXT_SENS_DATA_18 = 0x5B,
        EXT_SENS_DATA_19 = 0x5C,
        EXT_SENS_DATA_20 = 0x5D,
        EXT_SENS_DATA_21 = 0x5E,
        EXT_SENS_DATA_22 = 0x5F,
        EXT_SENS_DATA_23 = 0x60,
        MOT_DETECT_STATUS = 0x61,
        I2C_SLV0_DO      = 0x63,
        I2C_SLV1_DO      = 0x64,
        I2C_SLV2_DO      = 0x65,
        I2C_SLV3_DO      = 0x66,
        I2C_MST_DELAY_CTRL = 0x67,
        SIGNAL_PATH_RESET  = 0x68,
        MOT_DETECT_CTRL  = 0x69,
        USER_CTRL        = 0x6A,
        PWR_MGMT_1       = 0x6B,
        PWR_MGMT_2       = 0x6C,
        DMP_BANK         = 0x6D,
        DMP_RW_PNT       = 0x6E,
        DMP_REG          = 0x6F,
        DMP_REG_1        = 0x70,
        DMP_REG_2        = 0x71,
        FIFO_COUNTH      = 0x72,
        FIFO_COUNTL      = 0x73,
        FIFO_R_W         = 0x74,
        WHO_AM_I_MPU9250 = 0x75,// Should return = 0x71
        XA_OFFSET_H      = 0x77,
        XA_OFFSET_L      = 0x78,
        YA_OFFSET_H      = 0x7A,
        YA_OFFSET_L      = 0x7B,
        ZA_OFFSET_H      = 0x7D,
        ZA_OFFSET_L      = 0x7E,




        /*******************************************************************
        * CONFIG register bits
        *******************************************************************/
                FIFO_MODE_REPLACE_OLD = 0,
        FIFO_MODE_KEEP_OLD    = 0x01<<6,
        EXT_SYNC_SET_DISABLE = 0,


        /*******************************************************************
        * GYRO_CONFIG register bits
        *******************************************************************/
                XGYRO_CTEN        = 0x01<<7,
        YGYRO_CTEN        = 0x01<<6,
        ZGYRO_CTEN        = 0x01<<5,
        GYRO_FSR_CFG_250    = 0x00<<3,
        GYRO_FSR_CFG_500    = 0x01<<3,
        GYRO_FSR_CFG_1000   = 0x02<<3,
        GYRO_FSR_CFG_2000   = 0x03<<3,
        FCHOICE_B_DLPF_EN   = 0x00,
        FCHOICE_B_DLPF_DISABLE  = 0x01,

        /*******************************************************************
        * ACCEL_CONFIG register bits
        *******************************************************************/
                AX_ST_EN        = 0x01<<7,
        AY_ST_EN        = 0x01<<6,
        AZ_ST_EN        = 0x01<<5,
        ACCEL_FSR_CFG_2G    = 0x00<<3,
        ACCEL_FSR_CFG_4G    = 0x01<<3,
        ACCEL_FSR_CFG_8G    = 0x02<<3,
        ACCEL_FSR_CFG_16G   = 0x03<<3,

        /*******************************************************************
        * ACCEL_CONFIG2 register bits
        *******************************************************************/
                ACCEL_FCHOICE_1KHZ    = 0x00<<3,
        ACCEL_FCHOICE_4KHZ    = 0x01<<3,


        /*******************************************************************
        * INT_PIN_CFG
        *******************************************************************/
                ACTL_ACTIVE_LOW     = 0x01<<7,
        ACTL_ACTIVE_HIGH    = 0x00,
        INT_OPEN_DRAIN      = 0x00,
        INT_PUSH_PULL     = 0x00<<6,
        LATCH_INT_EN      = 0x01<<5,
        INT_ANYRD_CLEAR     = 0x01<<4,
        ACTL_FSYNC_ACTIVE_LOW = 0x01<<3,
        ACTL_FSYNC_ACTIVE_HIGH  = 0x00<<3,
        FSYNC_INT_MODE_EN   = 0x01<<2,
        FSYNC_INT_MODE_DIS    = 0x00<<2,
        BYPASS_EN       = 0x01<<1,


        /*******************************************************************
        *INT_ENABLE register settings
        *******************************************************************/
                WOM_EN          = 0x01<<6,
        WOM_DIS         = 0x00<<6,
        FIFO_OVERFLOW_EN    = 0x01<<4,
        FIFO_OVERFLOW_DIS   = 0x00<<4,
        FSYNC_INT_EN      = 0x01<<3,
        FSYNC_INT_DIS     = 0x00<<3,
        RAW_RDY_EN        = 0x01,
        RAW_RDY_DIS       = 0x00,

        /*******************************************************************
        * FIFO_EN register settings
        *******************************************************************/
                FIFO_TEMP_EN      = 0x01<<7,
        FIFO_GYRO_X_EN      = 0x01<<6,
        FIFO_GYRO_Y_EN      = 0x01<<5,
        FIFO_GYRO_Z_EN      = 0x01<<4,
        FIFO_ACCEL_EN     = 0x01<<3,
        FIFO_SLV2_EN      = 0x01<<2,
        FIFO_SLV1_EN      = 0x01<<1,
        FIFO_SLV0_EN      = 0x01,


        /*******************************************************************
        * PWR_MGMT_1 register settings
        *******************************************************************/
                H_RESET         = 0x01<<7,
        MPU_SLEEP       = 0x01<<6,
        MPU_CYCLE       = 0x01<<5,


        /*******************************************************************
        * temperature reading constants
        *******************************************************************/
                ROOM_TEMP_OFFSET    = 0x00,
        // TEMP_SENSITIVITY    = 333.87, // degC/LSB


        /*******************************************************************
        * USER_CTRL settings bits
        *******************************************************************/
                FIFO_EN_BIT     = 0x01<<6,
        I2C_MST_EN      = 0x01<<5,
        I2C_IF_DIS      = 0x01<<4,
        FIFO_RST      = 0x01<<2,
        I2C_MST_RST     = 0x01<<1,
        SIG_COND_RST      = 0x01,

        /******************************************************************
        * Magnetometer Registers
        ******************************************************************/
                AK8963_ADDR    = 0x0C,
        WHO_AM_I_AK8963  = 0x00, // should return = 0x48
        INFO             = 0x01,
        AK8963_ST1       = 0x02,  // data ready status
        AK8963_XOUT_L    = 0x03,  // data
        AK8963_XOUT_H    = 0x04,
        AK8963_YOUT_L    = 0x05,
        AK8963_YOUT_H    = 0x06,
        AK8963_ZOUT_L    = 0x07,
        AK8963_ZOUT_H    = 0x08,
        AK8963_ST2       = 0x09,
        AK8963_CNTL      = 0x0A,  // main mode control register
        AK8963_ASTC      = 0x0C,  // Self test control
        AK8963_I2CDIS    = 0x0F,  // I2C disable
        AK8963_ASAX      = 0x10,  // x-axis sensitivity adjustment value
        AK8963_ASAY      = 0x11,  // y-axis sensitivity adjustment value
        AK8963_ASAZ      = 0x12,  // z-axis sensitivity adjustment value

        /******************************************************************
        * Magnetometer AK8963_CNTL register Settings
        ******************************************************************/
                MAG_POWER_DN  = 0x00,  // power down magnetometer
        MAG_SINGLE_MES  = 0x01,  // powers down after 1 measurement
        MAG_CONT_MES_1  = 0x02,  // 8hz continuous self-sampling
        MAG_CONT_MES_2  = 0x06,  // 100hz continuous self-sampling
        MAG_EXT_TRIG  = 0x04,  // external trigger mode
        MAG_SELF_TEST = 0x08,  // self test mode
        MAG_FUSE_ROM  = 0x0F,  // ROM read only mode
        MSCALE_16   = 0x01<<4,
        MSCALE_14   = 0x00,

        /******************************************************************
        * Magnetometer AK8963_ST2 register definitions
        ******************************************************************/
                MAGNETOMETER_SATURATION = 0x01<<3,

        /******************************************************************
        * Magnetometer AK8963_ST1 register definitions
        ******************************************************************/
                MAG_DATA_READY  = 0x01,

        /******************************************************************
        * Magnetometer sensitivity in micro Teslas to LSB
        ******************************************************************/
        // MAG_RAW_TO_uT = (4912.0/32760.0),


        BIT_I2C_MST_VDDIO   = 0x80,
        BIT_FIFO_EN         = 0x40,
        BIT_DMP_EN          = 0x80,
        BIT_FIFO_RST        = 0x04,
        BIT_DMP_RST         = 0x08,
        BIT_FIFO_OVERFLOW   = 0x10,
        BIT_DATA_RDY_EN     = 0x01,
        BIT_DMP_INT_EN      = 0x02,
        BIT_MOT_INT_EN      = 0x40,
        BITS_FSR            = 0x18,
        BITS_LPF            = 0x07,
        BITS_HPF            = 0x07,
        BITS_CLK            = 0x07,
        BIT_FIFO_SIZE_1024  = 0x40,
        BIT_FIFO_SIZE_2048  = 0x80,
        BIT_FIFO_SIZE_4096  = 0xC0,
        BIT_RESET           = 0x80,
        BIT_SLEEP           = 0x40,
        BIT_S0_DELAY_EN     = 0x01,
        BIT_S2_DELAY_EN     = 0x04,
        BITS_SLAVE_LENGTH   = 0x0F,
        BIT_SLAVE_BYTE_SW   = 0x40,
        BIT_SLAVE_GROUP     = 0x10,
        BIT_SLAVE_EN        = 0x80,
        BIT_I2C_READ        = 0x80,
        BITS_I2C_MASTER_DLY = 0x1F,
        BIT_AUX_IF_EN       = 0x20,
        BIT_ACTL            = 0x80,
        BIT_LATCH_EN        = 0x20,
        BIT_ANY_RD_CLR      = 0x10,
        BIT_BYPASS_EN       = 0x02,
        BITS_WOM_EN         = 0xC0,
        BIT_LPA_CYCLE       = 0x20,
        BIT_STBY_XA         = 0x20,
        BIT_STBY_YA         = 0x10,
        BIT_STBY_ZA         = 0x08,
        BIT_STBY_XG         = 0x04,
        BIT_STBY_YG         = 0x02,
        BIT_STBY_ZG         = 0x01,
        BIT_STBY_XYZA       = (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA),
        BIT_STBY_XYZG       = (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG),

        //GYRO_SF             (46850825LL * 200 / DMP_SAMPLE_RATE)
        // GYRO_SF             = (46850825),

        DMP_FEATURE_TAP             = 0x001,
        DMP_FEATURE_ANDROID_ORIENT  = 0x002,
        DMP_FEATURE_LP_QUAT         = 0x004,
        DMP_FEATURE_PEDOMETER       = 0x008,
        DMP_FEATURE_6X_LP_QUAT      = 0x010,
        DMP_FEATURE_GYRO_CAL        = 0x020,
        DMP_FEATURE_SEND_RAW_ACCEL  = 0x040,
        DMP_FEATURE_SEND_RAW_GYRO   = 0x080,
        DMP_FEATURE_SEND_CAL_GYRO   = 0x100,
        DMP_FEATURE_SEND_ANY_GYRO   = (DMP_FEATURE_SEND_RAW_GYRO | \
      DMP_FEATURE_SEND_CAL_GYRO)

    };
}

#endif //GROUP9_IMU_HPP
