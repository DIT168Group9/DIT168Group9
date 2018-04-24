//
// Created by Firsou on 23-Apr-18.
//

#include <iostream>
#include "IMU.hpp"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);

    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("freq") ||
        0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("bus") ||
            0 == commandlineArguments.count("verbose")) {
        std::cerr << "argv[0] reads inputs from the IMU Sensors and transmits them to the car's components."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --dev=<path toIMU> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/i2c-2 --freq=100 --cid=200 " <<
                                                    "--bus=118" << std::endl;
        returnValue = 1;
    } else {
        const std::string DEV = commandlineArguments["dev"];
        const uint32_t ID{(commandlineArguments["id"].size() != 0) ? static_cast<uint32_t>(std::stoi(commandlineArguments["id"])) : 0};
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);
        const float FREQ = std::stof(commandlineArguments["cid"]);
        const bool VERBOSE = commandlineArguments.count("verbose") != 0;

        int16_t deviceFile = open(DEV.c_str(), O_RDWR);

        if (deviceFile < 0) {
            std::cerr << "Failed to open the i2c bus." << std::endl;
            return 1;
        }

        const uint8_t address = std::stoi(commandlineArguments["bus"]);

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

void initializeMPU(int16_t deviceFile, uint8_t MPU9250_ADDRESS) {
    uint8_t addr = MPU9250_ADDRESS;
    i2cAccessDevice(deviceFile, addr);
    uint8_t reg = MPU9250::PWR_MGMT_1;
    i2cWriteRegister(std::vector<uint8_t>{reg,0x00}, deviceFile);
    usleep(100000);
}