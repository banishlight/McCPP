#include <Standards.hpp>
#include <network/IPAddress.hpp>
#include <regex>
const std::regex IPRegex(R":(^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$):");
const std::wregex IPRegexW(LR":(^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$):");

/* default constructor is invalid*/
IPAddress::IPAddress() : isValid(false), addr(0){

}
IPAddress::IPAddress(const string& ip)
    : isValid(false), addr(0){
    std::sregex_iterator begin(ip.begin(), ip.end(), IPRegex);
    std::sregex_iterator end;

    if (begin == end) return; // m_Valid = false

    std::smatch match = *begin;

    const Int32 octet1 = atoi(string(match[1]).c_str());
    const Int32 octet2 = atoi(string(match[2]).c_str());
    const Int32 octet3 = atoi(string(match[3]).c_str());
    const Int32 octet4 = atoi(string(match[4]).c_str());

    addr = (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
    isValid = true;
}
IPAddress::IPAddress(Byte octet1, Byte octet2, Byte octet3, Byte octet4) noexcept
    : isValid(true){
    addr = (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
}


IPAddress LocalAddress() {
    return IPAddress((Byte)127, (Byte)0, (Byte)0, (Byte)1);
}

string IPAddress::ToString() const{
    std::stringstream ss;

    for (int i = 0; i < 4; ++i) {
        if (i != 0) ss << ".";
        ss << static_cast<UInt32>(GetOctet(i + 1));
    }

    return ss.str();
}

Byte IPAddress::GetOctet(uint8_t num) const {
    if (num == 0 || num > 4) throw std::invalid_argument("Invalid argument in IPAddress:GetOctet.");

    return (addr >> (8 * (4 - num))) & 0xFF;
}
