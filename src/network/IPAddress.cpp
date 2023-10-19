#include "../../include/Standards.h"
#include "../../include/network/IPAddress.h"
#include <regex>
const std::regex IPRegex(R":(^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$):");
const std::wregex IPRegexW(LR":(^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$):");

/* default constructor is invalid*/
IPAddress::IPAddress() noexcept : isValid(false), addr(0)
{

}
IPAddress::IPAddress(const std::string& ip)
    : isValid(false), addr(0)
{
    std::sregex_iterator begin(ip.begin(), ip.end(), IPRegex);
    std::sregex_iterator end;

    if (begin == end) return; // m_Valid = false

    std::smatch match = *begin;

    const Int32 octet1 = atoi(std::string(match[1]).c_str());
    const Int32 octet2 = atoi(std::string(match[2]).c_str());
    const Int32 octet3 = atoi(std::string(match[3]).c_str());
    const Int32 octet4 = atoi(std::string(match[4]).c_str());

    addr = (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;
    isValid = true;
}
IPAddress LocalAddress() {
    return IPAddress(127,0,0,1);
}