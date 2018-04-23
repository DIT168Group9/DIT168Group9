//
// Created by firsou on 23/04/18.
//

#include <iostream>
#include "device-imu.hpp"
#include "Messages.hpp"

int main(int argc, char** argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("freq") ||
        0 == commandlineArguments.count("cid")) {
        std::cerr << argv[0]
                  << " reads the IMU sensors inputs and transmits it to the car's components"
                  << std::endl;
        std::cerr << "Usage:   " << argv[0]
                  << " --dev=<path toIMU> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/input/js2 --freq=100 --cid=200" << std::endl;
        returnValue = 1;
    }
    else {
        
    }
}