#pragma once
#include "lwoServerPackets.h"

namespace lwoPacketUtils {
	void createPacketHeader(unsigned char uPacketID, unsigned short sConnectionType, unsigned int iInternalPacketID, RakNet::BitStream* bitStream);
	void writeStringToPacket(std::string sString, int maxSize, RakNet::BitStream* bitStream);
	std::string readWStringAsString(unsigned int iStrStartLoc, Packet* packet);
	bool savePacket(const std::string& sFileName, const char* cData, int iLength);
}