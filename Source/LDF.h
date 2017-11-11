#pragma once
#include "RakPeerInterface.h"
#include "BitStream.h"
#include <string>
#include <sstream>

class LDF {
	unsigned long m_ldfCount = 0;
	RakNet::BitStream* ldfStream;
public:
	RakNet::BitStream* GetBitStream() { return this->ldfStream; }
	void SetBitStream(RakNet::BitStream *ldfStream) { this->ldfStream = ldfStream; }

	unsigned long GetKeyCount() { return m_ldfCount; }

	void WriteWString(std::wstring keyname, std::wstring key);
	void WriteBool(std::wstring keyname, bool key);
	void WriteOBJID(std::wstring keyname, unsigned long long key);
	void WriteS64(std::wstring keyname, unsigned long long key);
	void WriteFloat(std::wstring keyname, float key);
	void WriteU32(std::wstring keyname, unsigned long);
	void WriteS32(std::wstring keyname, long key);
	void WriteXML(std::wstring keyname, RakNet::BitStream* data, unsigned long xmllength);

	void WriteWSToBitStream(RakNet::BitStream* stream, std::wstring string);
};