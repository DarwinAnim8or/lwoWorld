#include "lwoPacketUtils.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

void lwoPacketUtils::createPacketHeader(unsigned char uPacketID, unsigned short sConnectionType, unsigned int iInternalPacketID, RakNet::BitStream* bitStream) {
	bitStream->Write((unsigned char)uPacketID);
	bitStream->Write(sConnectionType);
	bitStream->Write(iInternalPacketID);
	bitStream->Write((char)0);
}

void lwoPacketUtils::writeStringToPacket(std::string sString, int maxSize, RakNet::BitStream* bitStream) {
	int size = sString.size();
	int emptySize = maxSize - size;

	if (size > maxSize) size = maxSize;

	for (int i = 0; i < size; i++) {
		bitStream->Write((char)sString[i]);
	}

	for (int i = 0; i < emptySize; i++) {
		bitStream->Write((char)0);
	}
} //writeStringToPacket

int lwoPacketUtils::readInt(int startLoc, int endLoc, Packet* packet) {
	std::vector<unsigned char> t;
	for (int i = startLoc; i <= endLoc; i++) t.push_back(packet->data[i]);
	return *(int*)t.data();
} //readInt

unsigned long lwoPacketUtils::readLong(int startLoc, int endLoc, Packet* packet) {
	std::vector<unsigned char> t;
	for (int i = startLoc; i <= endLoc; i++) t.push_back(packet->data[i]);
	return *(unsigned long*)t.data();
} //readLong

unsigned long long lwoPacketUtils::readLongLong(int startLoc, int endLoc, Packet* packet) {
	std::vector<unsigned char> t;
	for (int i = startLoc; i <= endLoc; i++) t.push_back(packet->data[i]);
	return *(unsigned long long*)t.data();
} //readLongLong

std::string lwoPacketUtils::readWStringAsString(unsigned int iStrStartLoc, Packet* packet) {
	std::string sToReturn = "";

	if (packet->length > iStrStartLoc) {
		int i = 0;
		while (packet->data[iStrStartLoc + i] != '\0' && packet->length > unsigned int((iStrStartLoc + i))) {
			sToReturn.push_back(packet->data[iStrStartLoc + i]);
			i = i + 2;
		}
	}

	return sToReturn;
} //readWStringAsString

bool lwoPacketUtils::savePacket(const std::string& sFileName, const char* cData, int iLength) {
	std::string sPath = "./packets/" + sFileName;

	std::ofstream ofFile(sPath, std::ios::binary);
	if (!ofFile.is_open()) return false;

	ofFile.write(cData, iLength);
	ofFile.close();
	return true;
} //savePacket

std::wstring lwoPacketUtils::StringToWString(const std::string& string, int size) {
	std::wstring ret;
	int newSize = size == -1 ? string.size() : (size > string.size() ? string.size() : size);

	for (int i = 0; i < newSize; ++i) {
		ret.push_back(string[i]);
	}

	return ret;
}

std::string lwoPacketUtils::WStringToString(const std::wstring& string, int size) {
	std::string ret;
	int newSize = size == -1 ? string.size() : (size > string.size() ? string.size() : size);

	for (int i = 0; i < newSize; ++i) {
		ret.push_back(string[i]);
	}

	return ret;
}
