#pragma once
#include <vector>

#include "../Standards.h"

class IPAddress
{
public:
	IPAddress();
	bool IsValid();
	typedef std::vector<IPAddress> IPAddresses;
	IPAddress(const string& ip);
	IPAddress(const std::wstring& ip);
	IPAddress(Byte octet1, Byte octet2, Byte octet3, Byte octet4) noexcept;
	Byte GetOctet(Byte idx) const;
	void SetOctet(Byte idx, Byte value);
	static IPAddress LocalAddress();
	bool IsValid() const noexcept { return isValid; }
	std::string  ToString() const;


	bool operator==(const IPAddress& right);
	bool operator!=(const IPAddress& right);
	bool operator==(bool b);

	std::ostream& operator<<(std::ostream& os, const IPAddress& addr);
	std::wostream& operator<<(std::wostream& os, const IPAddress& addr);
private:
	bool isValid;
	UInt32 addr;
	
};
