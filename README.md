# DIT168 Group9

| Linux & OSX Build (TravisCI) | CII Best Practices |
| :--------------------------: | :----------------: |
[![Build Status](https://travis-ci.org/DIT168Group9/DIT168Group9.svg?branch=master)](https://travis-ci.org/DIT168Group9/DIT168Group9) | [![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/1665/badge)](https://bestpractices.coreinfrastructure.org/projects/1665) |

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This project is for Gothenburg University's DIT168 Industrial IT and Embedded Systems course.
The goal is to design and develop a self-driving miniature vehicle.
The vehicle will be able to move using a remote input method as well as
an automated protocol in order to follow another miniature vehicle.
At the same time it will communicate with other autonomous cars through DIT168 V2V (vehicle to vehicle) protocol.

In order to clone the project through terminal: 

```
git clone https://github.com/DIT168Group9/DIT168Group9.git
cd DIT168Group9
mkdir build
cd build
cmake .. 
make
```

The microservices contained within this repository are designed to be
run concurrently with the help of docker-compose.
Therefore, we recommend that you build a Docker image out of this repository
(or download our own through Docker Hub).

We provide a sample docker-compose.yml for you to use.

## Table of Contents
* [Current Features](#current-features)
* [Dependencies](#dependencies)
* [Documentation](#documentation)

## Current Features
* Manually control the car with a PS4 Controller over Bluetooth
* Communicate with other vehicles using the DIT168 V2V Protocol
* View the messages and inputs from the sensors through Group 9 Signal Viewer web interface

## Dependencies
* A C++14 Compliant Compiler (We use G++)
* [libcluon](https://github.com/chrberger/libcluon) [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)
* libcluon.js is released under the terms of the MIT License [![License: MIT](https://img.shields.io/packagist/l/doctrine/orm.svg
)](https://opensource.org/licenses/MIT)
* [opendlv-device-ultrasonic-srf08](https://github.com/chalmers-revere/opendlv-device-ultrasonic-srf08) [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)
* [opendlv-signal-viewer](https://github.com/chalmers-revere/opendlv-signal-viewer) [![License: BSD3-Clause](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg
)](https://opensource.org/licenses/BSD-3-Clause)

## Documentation
* You can find our documentation in both TeX and HTML format in the /docs directory of this repository
* You can also view it directly [here](https://dit168group9.github.io/DIT168Group9/)
