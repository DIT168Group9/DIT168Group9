#pragma once

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include "cluon/OD4Session.hpp"
#include "Messages.hpp"

static constexpr float MIN_AXES_VALUE = -32768.0f;              /**< Minimum value of axes range*/
static constexpr float MAX_AXES_VALUE = 32767.0f;               /**< Maximum value of axes range.*/

static constexpr float m_MAX_DECELERATION = -1.0f;               /**< Max Deceleration value, must be negative.*/
static constexpr float m_MAX_ACCELERATION = 0.16f;               /**< Max Acceleration value, must be positive.*/
//static constexpr float m_MAX_STEERING_ANGLE_RIGHT = 45.0f;      /**< Max Steering Angle, must be positive.*/
//static constexpr float m_MAX_STEERING_ANGLE_LEFT = 28.0f;       /**< Max Steering Angle, must be positive.*/
//static constexpr float m_OFFSET = -0.12f;

opendlv::proxy::PedalPositionReading pedalPositionReading;
opendlv::proxy::GroundSteeringReading steeringReading;

/**
 * An enum.
 * Associates button numbers to defined button names.
 */
typedef enum {
    Square = 0,
    X = 1,
    Circle = 2,
    Triangle = 3,
    L1 = 4,
    R1 = 5,
    L2 = 6,
    R2 = 7,
    Share = 8,
    Options = 9,
    LStick = 10,
    RStick = 11,
    PS = 12,
} PS4Button;

/**
 * An enum.
 * Associates axes numbers to defined axes names.
 */
typedef enum {
    LStickX = 0,
    LStickY = 1,
    RStickX = 2,
    L2Y = 3,
    R2Y = 4,
    RStickY = 5,
    PadX = 6,
    PadY = 7
} PS4Axis;

/**
 * A struct.
 * Contains information on events read from PS4's input file.
 * Timestamp corresponds to the time when the event was sent.
 * Data is the value of the event (for buttons, a value of 1 will correspond to a press, 0 a release)
 * Type differentiates between the type of Event (Axis or Button)
 * Id corresponds to the specific Type, (Which button or which axis is being used specifically)
 */
typedef struct {
    uint32_t timestamp;
    int16_t  data;
    uint8_t  type;
    uint8_t  id;
} PS4Event;

/**
 * This function determines which PS4 Button has been pressed.
 * The buttons should correspond to those defined in the PS4Button enum.
 * @param buttonId
 * @return Button Number to be packaged into a Button Pressed message
 */
uint16_t findButton(PS4Event event);

/**
 * This function sends the Button Number of the button pressed to the OD4 Channel.
 * It makes use of the Button Pressed message and packages the button number into the buttonNumber field.
 * @param buttonNumber
 */
void sendButtonPressed(uint16_t buttonNumber);

/**
 * This function sends the Pedal Position value to the OD4 Channel.
 * It makes use of the PedalPositionReading message and packages the value into the position field.
 * @param pedalValue
 */
void sendPedalPosition(float pedalValue);

/**
 * This function sends the Steering Angle to the OD4 Channel.
 * It makes use of the GroundSteeringReading message and packages the angle into the groundSteering field.
 * @param steeringAngle
 */
void sendSteeringAngle(float steeringAngle);