//
// Created by Firsou on 23-Apr-18.
// Adapted from openKorp's IMU implementation
//

#include <iostream>
#include "IMU.hpp"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char** argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);

    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("freq") ||
        0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("verbose")) {
        std::cerr << "argv[0] reads inputs from the IMU Sensors and transmits them to the car's components."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --dev=<path toIMU> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/i2c-2 --freq=100 --cid=200 " <<
                                                    "--bus=118" << std::endl;
        returnValue = 1;
    } else {
        const std::string DEV = commandlineArguments["dev"];
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);
        const float FREQ = std::stof(commandlineArguments["cid"]);
        const bool VERBOSE = commandlineArguments.count("verbose") != 0;

        int16_t deviceFile = open(DEV.c_str(), O_RDWR);

        if (deviceFile < 0) {
            std::cerr << "Failed to open the i2c bus." << std::endl;
            return 1;
        }

        int8_t status = ioctl(deviceFile, I2C_SLAVE, address);
        if (status < 0) {
            std::cerr << "Could not acquire bus access for device " << DEV << "." << std::endl;
            return 1;
        }

        uint8_t parseFirmwareBuffer[1];
        parseFirmwareBuffer[0] = 0x00;
        status = write(deviceFile, parseFirmwareBuffer, 1);
        if (status != 1) {
            std::cerr << "Could not write firmware request to device on " << DEV << "." << std::endl;
            return 1;
        }

        uint8_t firmwareBuffer[1];
        status = read(deviceFile, firmwareBuffer, 1);
        if (status != 1) {
            std::cerr << "Could not read firmware from device on " << DEV << "." << std::endl;
            return 1;
        }

        std::cout << "Connected with the MPU9250 device on " << DEV << ". Reported firmware version '"
                  << static_cast<int32_t>(firmwareBuffer[0]) << "'." << std::endl;


        cluon::OD4Session od4{CID};


    }
}

int8_t i2cAccessDevice(int16_t deviceFile, uint8_t const addr) {
    if (ioctl(deviceFile, I2C_SLAVE, addr) < 0) {
        std::cerr << "[MPU9250] Failed to acquire bus access or talk to slave device. " << std::endl;
        return -1;
    }
    return 0;
}

int8_t i2cWriteRegister(std::vector<uint8_t> a_data, int16_t deviceFile) {
    uint8_t* buffer = a_data.data();

    uint8_t status = write(deviceFile, buffer, a_data.size());

    if (status != a_data.size()) {
        std::cerr << "[MPU9250] Failed to write on I2C bus." << std::endl;
        return -1;
    }
    return 0;
}

int8_t i2cReadRegister(int16_t deviceFile, uint8_t const addr, uint8_t *data, uint8_t const length) {
    uint8_t buffer[1];
    buffer[0] = addr;
    uint8_t statusOut = write(deviceFile, buffer, 1);
    uint8_t statusIn = read(deviceFile, data, length);
    if (statusOut != 1 || statusIn != length) {
        std::cerr << "[MPU9250] Failed to read I2C on bus." << std::endl;
        return -1;
    }
    return 0;
}

void initializeMpu(int16_t deviceFile) {
    // wake up device
    // Clear sleep mode bit (6), enable all sensors
    uint8_t addr = MPU9250_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::PWR_MGMT_1;
    i2cWriteRegister(std::vector<uint8_t>{reg,0x00}, deviceFile);
    usleep(100000); // Wait for all registers to reset

    // Get stable time source
    // Auto select clock source to be PLL gyroscope reference if ready else
    i2cWriteRegister(std::vector<uint8_t>{reg,0x01}, deviceFile);
    usleep(200000);

    // Configure Gyro and Thermometer
    // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz,
    // respectively;
    // minimum delay time for this setting is 5.9 ms, which means sensor fusion
    // update rates cannot be higher than 1 / 0.0059 = 170 Hz
    // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
    // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!),
    // 8 kHz, or 1 kHz
    reg = MPU9250::CONFIG;
    i2cWriteRegister(std::vector<uint8_t>{reg,0x03}, deviceFile);

    // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
    // Use a 200 Hz rate; a rate consistent with the filter update rate
    // determined inset in CONFIG above.
    reg = MPU9250::SMPLRT_DIV;
    i2cWriteRegister(std::vector<uint8_t>{reg, 0x04}, deviceFile);

    // Set gyroscope full scale range
    // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are
    // left-shifted into positions 4:3

    // get current GYRO_CONFIG register value
    uint8_t c;
    reg = MPU9250::GYRO_CONFIG;
    i2cReadRegister(deviceFile, reg, &c, 1);
    // c = c & ~0xE0; // Clear self-test bits [7:5]
    c = c & ~0x02; // Clear Fchoice bits [1:0]
    c = c & ~0x18; // Clear AFS bits [4:3]
    c = c | m_gscale << 3; // Set full scale range for the gyro
    // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of
    // GYRO_CONFIG
    // c =| 0x00;
    // Write new GYRO_CONFIG value to register
    i2cWriteRegister(std::vector<uint8_t>{reg, c}, deviceFile);

    // Set accelerometer full-scale range configuration
    // Get current ACCEL_CONFIG register value
    reg = MPU9250::ACCEL_CONFIG;
    i2cReadRegister(deviceFile, reg, &c, 1);
    // c = c & ~0xE0; // Clear self-test bits [7:5]
    c = c & ~0x18;  // Clear AFS bits [4:3]
    c = c | m_ascale << 3; // Set full scale range for the accelerometer
    // Write new ACCEL_CONFIG register value
    i2cWriteRegister(std::vector<uint8_t>{reg, c}, deviceFile);

    // Set accelerometer sample rate configuration
    // It is possible to get a 4 kHz sample rate from the accelerometer by
    // choosing 1 for accel_fchoice_b bit [3]; in this case the bandwidth is
    // 1.13 kHz
    // Get current ACCEL_CONFIG2 register value
    reg = MPU9250::ACCEL_CONFIG2;
    i2cReadRegister(deviceFile, reg, &c, 1);
    c = c & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
    c = c | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
    // Write new ACCEL_CONFIG2 register value
    i2cWriteRegister(std::vector<uint8_t>{reg, c}, deviceFile);
    // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
    // but all these rates are further reduced by a factor of 5 to 200 Hz because
    // of the SMPLRT_DIV setting

    // Configure Interrupts and Bypass Enable
    // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH
    // until interrupt cleared, clear on read of INT_STATUS, and enable
    // I2C_BYPASS_EN so additional chips can join the I2C bus and all can be
    // controlled by the Arduino as master.
    reg = MPU9250::INT_PIN_CFG;
    i2cWriteRegister(std::vector<uint8_t>{reg, 0x22}, deviceFile);
    // Enable data ready (bit 0) interrupt
    reg = MPU9250::INT_ENABLE;
    i2cWriteRegister(std::vector<uint8_t>{reg, 0x01}, deviceFile);
    usleep(100000);
}

opendlv::proxy::AccelerationReading readAccelerometer(int16_t deviceFile) {
    uint8_t addr = MPU9250_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::ACCEL_XOUT_H;
    uint8_t rawData[6];
    i2cReadRegister(deviceFile, reg, &rawData[0], 6);

    float const c = getAscale();

    int16_t x = (((int16_t)rawData[0] << 8) | rawData[1] );
    int16_t y = (((int16_t)rawData[2] << 8) | rawData[3] );
    int16_t z = (((int16_t)rawData[4] << 8) | rawData[5] );
    opendlv::proxy::AccelerationReading accelerometerReading;
    accelerometerReading.accelerationX(x);
    accelerometerReading.accelerationY(y);
    accelerometerReading.accelerationZ(z);
    // opendlv::proxy::AccelerometerReading accelerometerReading(0,0,0);
    return accelerometerReading;
}

opendlv::proxy::MagneticFieldReading readMagnetometer(int16_t deviceFile) {
    uint8_t addr = AK8963_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::AK8963_XOUT_L;
    uint8_t rawData[6];
    i2cReadRegister(deviceFile, reg, &rawData[0], 6);

    int16_t x = (((int16_t)rawData[0] << 8) | rawData[1] );
    int16_t y = (((int16_t)rawData[2] << 8) | rawData[3] );
    int16_t z = (((int16_t)rawData[4] << 8) | rawData[5] );

    opendlv::proxy::MagneticFieldReading magnetometerReading;
    magnetometerReading.magneticFieldX(x);
    magnetometerReading.magneticFieldY(y);
    magnetometerReading.magneticFieldZ(z);

    return magnetometerReading;
}


opendlv::proxy::GyroscopeReading readGyroscope(int16_t deviceFile) {
    uint8_t addr = MPU9250_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::GYRO_XOUT_H;
    uint8_t rawData[6];
    i2cReadRegister(deviceFile, reg, &rawData[0], 6);

    float const c = getGscale(true);

    int16_t x = (((int16_t)rawData[0] << 8) | rawData[1] );
    int16_t y = (((int16_t)rawData[2] << 8) | rawData[3] );
    int16_t z = (((int16_t)rawData[4] << 8) | rawData[5] );

    opendlv::proxy::GyroscopeReading gyroscopeReading(x*c,y*c,z*c);
    // opendlv::proxy::GyroscopeReading gyroscopeReading(0,0,0);
    return gyroscopeReading;
}

opendlv::proxy::AltitudeReading readAltimeter(int16_t deviceFile) {
    opendlv::proxy::AltitudeReading altimeterReading;
    altimeterReading.altitude(0);
    return altimeterReading;
}

opendlv::proxy::TemperatureReading readThermometer(int16_t deviceFile) {
    uint8_t addr = MPU9250_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::TEMP_OUT_H;
    uint8_t rawData[2];
    i2cReadRegister(deviceFile, reg, &rawData[0], 2);

    int16_t temp = (((int16_t)rawData[0] << 8) | rawData[1]) / 1.0f;

    opendlv::proxy::TemperatureReading temperatureReading;
    temperatureReading.temperature(21.0f + temp / 333.87f);
    // opendlv::proxy::TemperatureReading temperatureReading(0);
    return temperatureReading;
}

float getGscale(bool a_radFlag) {
    float conversion = 1;
    if (a_radFlag) {
        conversion = static_cast<float>(M_PI) / 180.0f;
    }
    switch (m_ascale) {
        case GFS_250DPS:
            return (250.0f / 32768.0f) * conversion;
        case GFS_500DPS:
            return (500.0f / 32768.0f) * conversion;
        case GFS_1000DPS:
            return (1000.0f / 32768.0f) * conversion;
        case GFS_2000DPS:
            return (2000.0f / 32768.0f) * conversion;
        default:
            return 0.0f;
    }
}

float getAscale()
{
    switch (m_ascale) {
        case AFS_2G:
            return (9.82f * 2.0f / 32768.0f);
        case AFS_4G:
            return (9.82f * 4.0f / 32768.0f);
        case AFS_8G:
            return (9.82f * 8.0f / 32768.0f);
        case AFS_16G:
            return (9.82f * 16.0f / 32768.0f);
        default:
            return 0.0f;
    }
}