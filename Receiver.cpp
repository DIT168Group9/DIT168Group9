#include <chrono>
#include <iostream>

#include "Receiver.hpp"
#include "cluon/UDPSender.hpp"
#include "cluon/UDPReceiver.hpp"
#include "cluon/ToProtoVisitor.hpp"
#include "cluon/FromProtoVisitor.hpp"
#include "messages.hpp"

void Receiver::receive() {

    cluon::UDPReceiver receiver("127.0.0.1", 1234,
        [](std::string &&data, std::string &&/*from*/,
           std::chrono::system_clock::time_point &&/*timepoint*/) noexcept {
        std::stringstream sstr{data};
        cluon::FromProtoVisitor decoder;
        decoder.decodeFrom(sstr);
        MyTestMessage1 receivedMsg;
        receivedMsg.accept(decoder);
        std::cout << "Message received: " << receivedMsg.myValue() << std::endl;
    });

    using namespace std::literals::chrono_literals;
    while (receiver.isRunning()) {
        std::this_thread::sleep_for(1s);
    }
}
