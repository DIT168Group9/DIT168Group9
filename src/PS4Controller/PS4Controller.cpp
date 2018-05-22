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
                  << " --offset=<float Offset> --leftAngle=<int Angle> --rightAngle=<int Angle>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --dev=/dev/input/js0 --freq=100 --cid=200"
                  << " --offset=0.16 --leftAngle=5 --rightAngle=5"
                  << std::endl;
        retVal = 1;
    }
    else {
        const std::string DEV = commandlineArguments["dev"];
        const uint16_t FREQ = (uint16_t) std::stoi(commandlineArguments["freq"]);
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);

        const float m_OFFSET = std::stof(commandlineArguments["offset"]);
        const uint16_t m_MAX_STEERING_ANGLE_LEFT = (uint16_t) std::stoi(commandlineArguments["leftAngle"]);
        const uint16_t m_MAX_STEERING_ANGLE_RIGHT = (uint16_t) std::stoi(commandlineArguments["rightAngle"]);

        cluon::OD4Session od4(CID, [](cluon::data::Envelope /*&&envelope*/) noexcept {});
        auto atFrequency{[&od4, &DEV, &FREQ, &CID, &m_MAX_STEERING_ANGLE_LEFT, &m_MAX_STEERING_ANGLE_RIGHT,
                                 m_OFFSET]() -> bool {
            FILE* file = fopen(DEV.c_str(), "rb");
            if (file != nullptr) {
                auto event = (PS4Event* )malloc(sizeof(PS4Event));
                while (!feof(file)) {
                    if (fread(event, sizeof(PS4Event), 1, file)) {
                        if ((event->type &0x0F) == 1) {
                            if (event->data == 1) {
                                sendButtonPressed(findButton(event->id), &od4);
                            }
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

                                    od4.send(steeringReading);
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
                                    od4.send(pedalPositionReading);
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
        od4.timeTrigger(FREQ, atFrequency);
    }
    return retVal;
}

uint16_t findButton(uint8_t buttonId) {
    uint16_t buttonNumber = 13;
    switch (buttonId) {
        case Square:
            buttonNumber = 0;
            std::cout << "Square pressed." << std::endl;
            break;
        case X:
            buttonNumber = 1;
            std::cout << "X pressed." << std::endl;
            break;
        case Circle:
            buttonNumber = 2;
            std::cout << "Circle pressed." << std::endl;
            break;
        case Triangle:
            buttonNumber = 3;
            std::cout << "Triangle pressed." << std::endl;
            break;
        case L1:
            buttonNumber = 4;
            std::cout << "L1 pressed." << std::endl;
            break;
        case R1:
            buttonNumber = 5;
            std::cout << "R1 pressed." << std::endl;
            break;
        case L2:
            buttonNumber = 6;
            std::cout << "L2 pressed." << std::endl;
            break;
        case R2:
            buttonNumber = 7;
            std::cout << "R2 pressed." << std::endl;
            break;
        case Share:
            buttonNumber = 8;
            std::cout << "Share pressed." << std::endl;
            break;
        case Options:
            buttonNumber = 9;
            std::cout << "Options pressed." << std::endl;
            break;
        case LStick:
            buttonNumber = 10;
            std::cout << "L3 pressed." << std::endl;
            break;
        case RStick:
            buttonNumber = 11;
            std::cout << "R3 pressed." << std::endl;
            break;
        case PS:
            buttonNumber = 12;
            std::cout << "PS pressed." << std::endl;
            break;
        default:
            break;
    }
    return buttonNumber;
}

void sendButtonPressed(uint16_t button, cluon::OD4Session* od4Session) {
    if (button == 13) {
        return;
    }
    else {
        buttonPressed.buttonNumber(button);
        od4Session->send(buttonPressed);
    }
}

void roundValue(float* number) {
    *number = roundf(*number * 100) / 100.0f;
}