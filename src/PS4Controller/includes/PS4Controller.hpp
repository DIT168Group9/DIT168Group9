#pragma once

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include "cluon/OD4Session.hpp"
#include "Messages.hpp"

static constexpr float MIN_AXES_VALUE = -32768.0f;      /**< Minimum value of axes range*/
static constexpr float MAX_AXES_VALUE = 32767.0f;       /**< Maximum value of axes range.*/

static constexpr float m_MAX_DECELERATION = 0.25f;    /**< Max Deceleration value, must be negative.*/
static constexpr float m_MAX_ACCELERATION = 0.25f;       /**< Max Acceleration value, must be positive.*/
static constexpr float m_MAX_STEERING_ANGLE = 38.0f;     /**< Max Steering Angle, must be positive.*/

static constexpr float m_OFFSET = 0.16f;

opendlv::proxy::PedalPositionReading pedalPositionReading;

/**
 * An enum.
 * Associates button numbers to defined button names.
 */
typedef enum {
    X = 1,
    Circle = 2,
    Triangle = 3,
    Square = 0,
    L1 = 4,
    R1 = 5,
    L2 = 6,
    R2 = 7,
    Share = 8,
    Options = 9,
    PS = 12,
    LStick = 10,
    RStick = 11
} PS4Button;

/**
 * An enum.
 * Associates axes numbers to defined axes names.
 */
typedef enum {
    LStickX = 0,
    LStickY = 1,
    L2Y = 3,
    RStickX = 2,
    RStickY = 5,
    R2Y = 4,
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