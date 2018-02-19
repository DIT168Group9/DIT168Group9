#include <messages.hpp>

/*
 * THIS IS AN AUTO-GENERATED FILE. DO NOT MODIFY AS CHANGES MIGHT BE OVERWRITTEN!
 */


uint32_t MyTestMessage1::ID() {
    return 9001;
}

const std::string MyTestMessage1::ShortName() {
    return "MyTestMessage1";
}
const std::string MyTestMessage1::LongName() {
    return "MyTestMessage1";
}

MyTestMessage1& MyTestMessage1::myValue(const uint16_t &v) noexcept {
    m_myValue = v;
    return *this;
}
uint16_t MyTestMessage1::myValue() const noexcept {
    return m_myValue;
}



