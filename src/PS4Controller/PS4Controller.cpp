#include <cmath>
#include "includes/PS4Controller.hpp"
#include "../V2V/includes/V2VService.hpp"

int main(int argc, char** argv) {
    int retVal = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if (0 == commandlineArguments.count("dev") || 0 == commandlineArguments.count("freq") ||
            0 == commandlineArguments.count("cid")) {
        std::cerr << argv[0] << " reads the PS4Controller Wireless Controller inputs and sends it to the car's components"
                  << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --dev=<path Controller> --freq=<int Frequency> --cid=<OD4Session Session>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/input/js0 --freq=100 --cid=200" << std::endl;
        retVal = 1;
    }
    else {
        const std::string DEV = commandlineArguments["dev"];
        const uint16_t FREQ = (uint16_t) std::stoi(commandlineArguments["freq"]);
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);

        cluon::OD4Session od4(CID, [](cluon::data::Envelope /*&&envelope*/) noexcept {});
        auto atFrequency{[&od4, &DEV, &FREQ, &CID]() -> bool {
            FILE *file = fopen(DEV.c_str(), "rb");
            if (file != nullptr) {
                PS4Event *event = (PS4Event *)malloc(sizeof(PS4Event));
                while (!feof(file)) {
                    if (fread(event, sizeof(PS4Event), 1, file)) {
                        if ((event->type &0x0F) == 1) {
                            switch (event->id) {
                                case X:
                                    std::cout << "X pressed." << std::endl;
                                    break;
                                case Circle:
                                    std::cout << "Circle Pressed." << std::endl; //X
                                    break;
                                case Triangle:
                                    std::cout << "Triangle pressed." << std::endl;
                                    {
                                        opendlv::proxy::ButtonPressed buttonPressed;
                                        buttonPressed.buttonNumber(3);
                                        od4.send(buttonPressed);
                                        std::cout << "Sending Button: " << buttonPressed.buttonNumber() << std::endl;
                                    }
                                    break;
                                case Square:
                                    std::cout << "Square pressed." << std::endl;
                                    break;
                                case L1:
                                    std::cout << "L1 pressed." << std::endl;
                                    break;
                                case R1:
                                    std::cout << "R1 pressed." << std::endl;
                                    break;
                                case L2:
                                    std::cout << "L2 pressed." << std::endl;
                                    break;
                                case R2:
                                    std::cout << "R2 pressed." << std::endl;
                                    break;
                                case Share:
                                    std::cout << "Share pressed." << std::endl;
                                    break;
                                case Options:
                                    std::cout << "Options pressed." << std::endl;
                                    break;
                                case PS:
                                    std::cout << "PS Button pressed." << std::endl;
                                    break;
                                case LStick:
                                    std::cout << "L3 pressed." << std::endl;
                                    break;
                                case RStick:
                                    std::cout << "R3 pressed." << std::endl;
                                    break;
                                default:       break;
                            }
                        }
                        else if ((event->type &0x0F) == 2) {
                            switch (event->id) {
                                case LStickX: {
                                    opendlv::proxy::GroundSteeringReading steeringReading;
                                    steeringReading.groundSteering(event->data / MIN_AXES_VALUE * m_MAX_STEERING_ANGLE *
                                                                           static_cast<float>(M_PI) / 180.0f);
                                    od4.send(steeringReading);
                                    std::cout << "Sending Angle: " << steeringReading.groundSteering() << std::endl;
                                    }
                                    break;
                                case LStickY: break;
                                case L2Y: {
                                    opendlv::proxy::PedalPositionReading pedalPositionReading;
                                    pedalPositionReading.position(-(event->data) / MAX_AXES_VALUE * m_MAX_DECELERATION);
                                    od4.send(pedalPositionReading); //This value is in percent
                                    std::cout << "Sending Speed: " << pedalPositionReading.position() << std::endl;
                                    }
                                    break;
                                case RStickX: break;
                                case RStickY: break;
                                case R2Y:   {
                                    opendlv::proxy::PedalPositionReading pedalPositionReading;
                                    pedalPositionReading.position(event->data / MAX_AXES_VALUE * m_MAX_ACCELERATION);
                                    od4.send(pedalPositionReading); //This value is in percent
                                    std::cout << "Sending Speed: " << pedalPositionReading.position() << std::endl;
                                    }
                                    break;
                                case PadX:    break;
                                case PadY:    break;
                                default:      break;
                            }
                        }
                    }
                    else if (ferror(file)) {
                        std::cout << "Error: FREAD failed with error number'" << errno << std::endl;
                    }
                    else if (feof(file)) {
                        std::cout << "Error: EOF reached!" << std::endl;
                    }
                }
                free(event);
            }
            else {
                std::cout << "Error: input file at '" << DEV << "' cannot be accessed!" << std::endl;
            }
            return true;
        }};
        od4.timeTrigger(FREQ, atFrequency);
    }
    return retVal;
}