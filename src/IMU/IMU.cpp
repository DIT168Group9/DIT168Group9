//
// Created by Firsou on 23-Apr-18.
//

#include "IMU.hpp"
#include <iostream>
#include "cluon"

int main(int argc, char** argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);

    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("cid") ||
        0 == commandlineArguments.count("freq") || 0 == commandlineArguments.count("verbose")) {
        std::cerr << "argv[0] reads inputs from the IMU Sensors and transmits them to the car's components."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --dev=<path toIMU> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/i2c-2 --freq=100 --cid=200" << std::endl;
        returnValue = 1;
    } else {
        const std::string DEV = commandlineArguments["dev"];
        const uint16_t FREQ = (uint16_t) std::stoi(commandlineArguments["freq"]);
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);

        cluon::OD4Session od4(CID, [](cluon::data::Envelope /*&&envelope*/) noexcept {});
        auto atFrequency{[&od4, &DEV, &FREQ, &CID]() -> bool {
            FILE *file = fopen(DEV.c_str(), "rb");
            if (file != nullptr) {
                //TODO The rest
            }
            else {
                std::cout << "Error: Can't access IMU at '" << DEV << std::endl;
            }
        }};
    }
}