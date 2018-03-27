# DIT168Group9

| Linux & OSX Build (TravisCI) | Test Coverage | CII Best Practices |
| :--------------------------: | :-----------: | :----------------: |
[![Build Status](https://travis-ci.org/DIT168Group9/DIT168Group9.svg?branch=master)](https://travis-ci.org/DIT168Group9/DIT168Group9) | [![codecov](https://codecov.io/gh/DIT168Group9/DIT168Group9/branch/master/graph/badge.svg)](https://codecov.io/gh/DIT168Group9/DIT168Group9) | [![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/1665/badge)](https://bestpractices.coreinfrastructure.org/projects/1665) |

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This is an ongoing project for Gothenburg University's DIT168 Industrial IT and Embedded Systems course. The goal is to design and develop a self-driving miniature vehicle. The vehicle will be able to move following a lane with the help of Ultrasonic Sensors, Infrared Sensors, Cameras, Microservices, etc. At the same time it will communicate with other autonomous cars through a V2V (vehicle to vehicle) protocol.

In order to clone the project through terminal: 

```
git clone https://github.com/DIT168Group9/DIT168Group9.git
cd DIT168Group9
cmake .. 
make
```

Finally run

```
./helloworld
```

## Table of Contents
* [Current Features](#current-features)
* [Dependencies](#dependencies)

## Current Features
* Send integers to your friends, or colleagues, on a local network!
* Receive integers from your friends, or colleagues, on a local network!

## Dependencies
* A C++14 Compliant Compiler (We use G++)
* [libcluon](https://github.com/chrberger/libcluon) [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)
