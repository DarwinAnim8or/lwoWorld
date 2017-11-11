#pragma once
#include "lwoServerPackets.h"

namespace lwoPacketUtils {
	void createPacketHeader(unsigned char uPacketID, unsigned short sConnectionType, unsigned int iInternalPacketID, RakNet::BitStream* bitStream);
	void writeStringToPacket(std::string sString, int maxSize, RakNet::BitStream* bitStream);
	void writeWStringToPacket(RakNet::BitStream* stream, std::wstring str);
	int readInt(int startLoc, int endLoc, Packet* packet);
	unsigned long readLong(int startLoc, int endLoc, Packet* packet);
	unsigned long long readLongLong(int startLoc, int endLoc, Packet* packet);
	std::string readWStringAsString(unsigned int iStrStartLoc, Packet* packet);
	bool savePacket(const std::string& sFileName, const char* cData, int iLength);
	std::wstring StringToWString(const std::string& string, int size);
	std::string WStringToString(const std::wstring& string, int size);
}