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

        const float m_OFFSET = std::stof(commandlineArguments["offset"]);
        const uint16_t m_MAX_STEERING_ANGLE_LEFT = (uint16_t) std::stoi(commandlineArguments["leftAngle"]);
        const uint16_t m_MAX_STEERING_ANGLE_RIGHT = (uint16_t) std::stoi(commandlineArguments["rightAngle"]);

        std::shared_ptr<cluon::OD4Session> od4 = std::make_shared<cluon::OD4Session>(CID, [](cluon::data::Envelope /*&&envelope*/) noexcept {});
        auto atFrequency{[&od4, &DEV, &FREQ, &CID, &m_MAX_STEERING_ANGLE_LEFT, &m_MAX_STEERING_ANGLE_RIGHT,
                                 m_OFFSET]() -> bool {
            FILE* file = fopen(DEV.c_str(), "rb");
            if (file != nullptr) {
                auto event = (PS4Event* )malloc(sizeof(PS4Event));
                while (!feof(file)) {
                    if (fread(event, sizeof(PS4Event), 1, file)) {
                        if ((event->type &0x0F) == 1) {
                            sendButtonPressed(findButton(event), od4);
                        }
                        else if ((event->type &0x0F) == 2) {
                            switch (event->id) {
                                case LStickX: {
                                    steeringReading.groundSteering(m_OFFSET);
                                    float value = 0;
                                    if (event->data < 0) {
                                        value = event->data / MIN_AXES_VALUE * m_MAX_STEERING_ANGLE_LEFT *
                                                      static_cast<float>(M_PI) / 180.0f + m_OFFSET;
                                        roundValue(&value);
                                        steeringReading.groundSteering(value);
                                    }
                                    else if (event->data >= 0) {
                                        value = event->data / MIN_AXES_VALUE * m_MAX_STEERING_ANGLE_RIGHT *
                                                static_cast<float>(M_PI) / 180.0f + m_OFFSET;
                                        roundValue(&value);
                                        steeringReading.groundSteering(value);
                                    }

                                    od4->send(steeringReading);
                                    std::cout << "Sending Angle: " << steeringReading.groundSteering() << std::endl;
                                    }
                                    break;
                                case LStickY: break;
                                case L2Y:     break;
                                case RStickX: break;
                                case RStickY: {
                                    float value = 0;
                                    pedalPositionReading.position(value);
                                    if(event->data < 0) {
                                        value = event->data / MIN_AXES_VALUE * m_MAX_ACCELERATION;
                                        roundValue(&value);
                                        pedalPositionReading.position(value);
                                    }
                                    else if (event->data >= 0) {
                                        value = event->data / MAX_AXES_VALUE * m_MAX_DECELERATION;
                                        roundValue(&value);
                                        pedalPositionReading.position(value);
                                    }
                                    od4->send(pedalPositionReading);
                                    std::cout << "Sending speed: " << value << std::endl;
                                    }
                                    break;
                                case R2Y:     break;
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
        od4->timeTrigger(FREQ, atFrequency);
    }
    return retVal;
}

uint16_t findButton(PS4Event* event) {
    switch (event->id) {
        case Square:
            if (event->data == 1) {
                std::cout << "Square pressed." << std::endl;
                return {0};
            }
            break;
        case X:
            if (event->data == 1) {
                return {1};
            }
            break;
        case Circle:
            if (event->data == 1) {
                return {2};
            }
            break;
        case Triangle:
            if (event->data == 1) {
                return {3};
            }
            break;
        case L1:
            if (event->data == 1) {
                return {4};
            }
            break;
        case R1:
            if (event->data == 1) {
                return {5};
            }
            break;
        case L2:
            if (event->data == 1) {
                return {6};
            }
            break;
        case R2:
            if (event->data == 1) {
                return {7};
            }
            break;
        case Share:
            if (event->data == 1) {
                return {8};
            }
            break;
        case Options:
            if (event->data == 1) {
                return {9};
            }
            break;
        case LStick:
            if (event->data == 1) {
                return {10};
            }
            break;
        case RStick:
            if (event->data == 1) {
                return {11};
            }
            break;
        case PS:
            if (event->data == 1) {
                return {12};
            }
            break;
        default:
            break;
    }
}

void sendButtonPressed(uint16_t button, std::shared_ptr<cluon::OD4Session> od4Session) {
    buttonPressed.buttonNumber(button);
    od4Session->send(buttonPressed);
}

void roundValue(float* number) {
    *number = roundf(*number * 100) / 100.0f;
}