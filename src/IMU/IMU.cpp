//
// Created by Firas Cheaib on 23-Apr-18.
// Adapted from openKorp's IMU implementation
// Comments and code modified with permission
//

#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include "IMU.hpp"

int main(int argc, char** argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);

    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("freq") ||
        0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("verbose")) {
        std::cerr << "argv[0] reads inputs from the IMU Sensors and transmits them to the car's components."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --dev=<path toIMU> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/i2c-2 --freq=100 --cid=200 " << std::endl;
        returnValue = 1;
    }
    else {
        const std::string DEV = commandlineArguments["dev"];
        std::cout << DEV << std::endl;
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);
        const float FREQ = std::stof(commandlineArguments["freq"]);
        const bool VERBOSE = commandlineArguments.count("verbose") != 0;

        int16_t deviceFile = open(DEV.c_str(), O_RDWR);

        if (deviceFile < 0) {
            std::cerr << "Failed to open the i2c bus." << std::endl;
            return 1;
        }

        initializeMpu(deviceFile);
        initializeAK8963(deviceFile);
        setGyroOffset(calibrateMPU9250(deviceFile), deviceFile);

        cluon::OD4Session od4{CID};

        auto atFrequency{[&deviceFile, &VERBOSE, &od4]() -> bool {
            opendlv::proxy::GyroscopeReading gyroscopeReading = readGyroscope(deviceFile);
            od4.send(gyroscopeReading);

            opendlv::proxy::AccelerationReading accelerometerReading = readAccelerometer(deviceFile);
            od4.send(accelerometerReading);

            opendlv::proxy::MagneticFieldReading magnetometerReading = readMagnetometer(deviceFile);
            od4.send(magnetometerReading);

            opendlv::proxy::AltitudeReading altimeterReading = readAltimeter(deviceFile);
            od4.send(altimeterReading);

            opendlv::proxy::TemperatureReading thermometerReading = readThermometer(deviceFile);
            od4.send(thermometerReading);

            if (VERBOSE) {
                std::cout << "[OD4] Sending IMU data: " << std::endl
                          << "Gyroscope -" <<
                                    " X: " << gyroscopeReading.GyroscopeReadingX() <<
                                    " Y: " << gyroscopeReading.GyroscopeReadingY() <<
                                    " Z: " << gyroscopeReading.GyroscopeReadingZ() << std::endl
                          << "Accelerometer -" <<
                                    " X: " << accelerometerReading.accelerationX() <<
                                    " Y: " << accelerometerReading.accelerationY() <<
                                    " Z: " << accelerometerReading.accelerationZ() << std::endl
                          << "Magnetometer-" <<
                                    " X: " << magnetometerReading.magneticFieldX() <<
                                    " Y: " << magnetometerReading.magneticFieldY() <<
                                    " Z: " << magnetometerReading.magneticFieldZ() << std::endl
                          << "Altimeter -" <<
                                    " Altitude: " << altimeterReading.altitude() << std::endl
                          << "Thermometer -" <<
                                    " Temperature: "<< thermometerReading.temperature() << std::endl;
            }
            return true;
        }};
        od4.timeTrigger(FREQ, atFrequency);
    }
    return 0;
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
    c = c & ~0x02; // Clear Fchoice bits [1:0]
    c = c & ~0x18; // Clear AFS bits [4:3]
    c = c | m_gscale << 3; // Set full scale range for the gyro
    // Write new GYRO_CONFIG value to register
    i2cWriteRegister(std::vector<uint8_t>{reg, c}, deviceFile);

    // Set accelerometer full-scale range configuration
    // Get current ACCEL_CONFIG register value
    reg = MPU9250::ACCEL_CONFIG;
    i2cReadRegister(deviceFile, reg, &c, 1);
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
    // I2C_BYPASS_EN so additional chips can join the I2C bus (such as the Magnetometer)
    reg = MPU9250::INT_PIN_CFG;
    i2cWriteRegister(std::vector<uint8_t>{reg, 0x22}, deviceFile);
    // Enable data ready (bit 0) interrupt
    reg = MPU9250::INT_ENABLE;
    i2cWriteRegister(std::vector<uint8_t>{reg, 0x01}, deviceFile);
    usleep(100000);
}

void initializeAK8963(int16_t deviceFile) {
    // First extract the factory calibration for each magnetometer axis
    uint8_t rawData[3];  // x/y/z gyro calibration data stored here
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::AK8963_CNTL, 0x00}, deviceFile); // Power down magnetometer
    usleep(100000);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::AK8963_CNTL, 0x0F}, deviceFile); // Enter Fuse ROM access mode
    usleep(100000);
    i2cReadRegister(deviceFile, MPU9250::AK8963_ASAX, &rawData[0], 3);  // Read the x-, y-, and z-axis calibration values
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::AK8963_CNTL, 0x00}, deviceFile); // Power down magnetometer
    usleep(100000);
    // Configure the magnetometer for continuous read and highest resolution
    // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
    // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::AK8963_CNTL, 1 << 4 | 0110}, deviceFile); // Set magnetometer data resolution and sample ODR
    usleep(100000);
}

std::vector<float> calibrateMPU9250(int16_t deviceFile) {

    std::cout << "[MPU9250] Starting calibration...\n";
    uint8_t rawData[12];

    i2cAccessDevice(deviceFile, MPU9250_ADDRESS);

    i2cWriteRegister(std::vector<uint8_t>{MPU9250::PWR_MGMT_1, 0x80}, deviceFile);
    usleep(100000);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::PWR_MGMT_1, 0x01}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::PWR_MGMT_2, 0x00}, deviceFile);
    usleep(200000);

    i2cWriteRegister(std::vector<uint8_t>{MPU9250::INT_ENABLE, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::FIFO_EN, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::PWR_MGMT_1, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::I2C_MST_CTRL, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::USER_CTRL, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::USER_CTRL, 0x0C}, deviceFile);
    usleep(15000);

    i2cWriteRegister(std::vector<uint8_t>{MPU9250::CONFIG, 0x01}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::SMPLRT_DIV, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::GYRO_CONFIG, 0x00}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::ACCEL_CONFIG, 0x00}, deviceFile);

    float const gyroSens  = (250.0f / 32768.0f * static_cast<float>(M_PI) / 180.0f);

    i2cWriteRegister(std::vector<uint8_t>{MPU9250::USER_CTRL, 0x40}, deviceFile);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::FIFO_EN, 0x78}, deviceFile);
    usleep(40000);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::FIFO_EN, 0x00}, deviceFile);

    i2cReadRegister(deviceFile, MPU9250::FIFO_COUNTH, &rawData[0], 2);

    uint16_t fifoCount = ((uint16_t) rawData[0] <<  8) | rawData[1];
    std::cout << "[MPU9250] FIFO Count: " << fifoCount << std::endl;
    uint16_t packetCount = fifoCount/12;
    std::cout << "[MPU9250] Packet Count: " << packetCount << std::endl;

    int32_t gyroBias[3] = {0,0,0};
    for (uint8_t i = 0; i < packetCount; i++) {
        int16_t gyroSampl[3] = {0,0,0};
        i2cReadRegister(deviceFile, MPU9250::FIFO_R_W, &rawData[0], 12);

        gyroSampl[0]  = (int16_t) (((int16_t)rawData[6] << 8) | rawData[7]  );
        gyroSampl[1]  = (int16_t) (((int16_t)rawData[8] << 8) | rawData[9]  );
        gyroSampl[2]  = (int16_t) (((int16_t)rawData[10] << 8) | rawData[11]);

        gyroBias[0] += (int32_t) gyroSampl[0];
        gyroBias[1] += (int32_t) gyroSampl[1];
        gyroBias[2] += (int32_t) gyroSampl[2];
    }

    gyroBias[0] /= packetCount;
    gyroBias[1] /= packetCount;
    gyroBias[2] /= packetCount;


    // std::cout << "[MPU9250] Gyro bias: " << gyroBias.at(0) << ", " << gyroBias.at(1) << ", " << gyroBias.at(2) << std::endl;
    std::vector<float> gyroBiasVec;
    gyroBiasVec.push_back(gyroBias[0] * gyroSens);
    gyroBiasVec.push_back(gyroBias[1] * gyroSens);
    gyroBiasVec.push_back(gyroBias[2] * gyroSens);
    std::cout << "[MPU9250] Gyro bias: " << gyroBiasVec.at(0) << ", " << gyroBiasVec.at(1) << ", " << gyroBiasVec.at(2) << std::endl;
    return gyroBiasVec;
}

int8_t setGyroOffset(std::vector<float> const a_offset, int16_t deviceFile) {
    if (a_offset.size() != 3) {
        std::cerr << "[MPU9250] setGyroOffset received a vector of a length not supported." << std::endl;
        return -1;
    }

    float const gyroSens  = (250.0f / 32768.0f * static_cast<float>(M_PI) / 180.0f);

    int32_t xOffset = std::lround(a_offset.at(0) / gyroSens);
    int32_t yOffset = std::lround(a_offset.at(1) / gyroSens);
    int32_t zOffset = std::lround(a_offset.at(2) / gyroSens);

    uint8_t xh = (-xOffset/4 >> 8);
    uint8_t xl = ((-xOffset/4) & 0xFF);
    uint8_t yh = (-yOffset/4 >> 8);
    uint8_t yl = ((-yOffset/4) & 0xFF);
    uint8_t zh = (-zOffset/4 >> 8);
    uint8_t zl = ((-zOffset/4) & 0xFF);

    i2cAccessDevice(deviceFile, MPU9250_ADDRESS);
    i2cWriteRegister(std::vector<uint8_t>{MPU9250::XG_OFFSET_H, xh, xl, yh, yl, zh, zl}, deviceFile);

    return 0;
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
    accelerometerReading.accelerationX(x*c);
    accelerometerReading.accelerationY(y*c);
    accelerometerReading.accelerationZ(z*c);
    return accelerometerReading;
}

opendlv::proxy::MagneticFieldReading readMagnetometer(int16_t deviceFile) {
    uint8_t addr = AK8963_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::AK8963_XOUT_L;
    uint8_t rawData[7];
    i2cReadRegister(deviceFile, reg, &rawData[0], 7);

    float const c = getMscale();

    int16_t x = (((int16_t) rawData[0] << 8) | rawData[1]);
    int16_t y = (((int16_t) rawData[2] << 8) | rawData[3]);
    int16_t z = (((int16_t) rawData[4] << 8) | rawData[5]);

    opendlv::proxy::MagneticFieldReading magneticFieldReading;
    magneticFieldReading.magneticFieldX(x);
    magneticFieldReading.magneticFieldY(y);
    magneticFieldReading.magneticFieldZ(z);

    return magneticFieldReading;
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

    opendlv::proxy::GyroscopeReading gyroscopeReading;
    gyroscopeReading.GyroscopeReadingX(x*c);
    gyroscopeReading.GyroscopeReadingY(y*c);
    gyroscopeReading.GyroscopeReadingZ(z*c);
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
    switch (m_gscale) {
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

float getAscale() {
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

float getMscale() {
    switch (m_mscale) {
        // Possible magnetometer scales (and their register bit settings) are:
        // 14 bit resolution (0) and 16 bit resolution (1)
        case MFS_14BITS:
            return 10.0f * 4912.0f / 8190.0f; // Proper scale to return milliGauss
        case MFS_16BITS:
            return 10.0f * 4912.0f / 32760.0f; // Proper scale to return milliGauss
        default:
            return 0.0f;
    }
}
