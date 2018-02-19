#include <chrono>
#include <iostream>

#include "Sender.hpp"
#include "cluon/UDPSender.hpp"
#include "cluon/UDPReceiver.hpp"
#include "cluon/ToProtoVisitor.hpp"
#include "cluon/FromProtoVisitor.hpp"
#include "messages.hpp"

void Sender::send() {
    std::cout << "Welcome to Sender." << std::endl;

    cluon::UDPSender sender{"127.0.0.1", 1234};

    uint16_t value;
    std::cout << "Enter an integer: ";
    std::cin >> value;
    MyTestMessage1 msg;
    msg.myValue(value);
    cluon::ToProtoVisitor encoder;
    msg.accept(encoder);
    std::string data{encoder.encodedData()};
    sender.send(std::move(data));
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(5s);
}
