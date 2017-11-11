#include "LDF.h"

void LDF::WriteWSToBitStream(RakNet::BitStream * stream, std::wstring string) {
	for (uint32_t i = 0; i < string.size(); i++) {
		stream->Write(static_cast<uint16_t>(string.at(i)));
	}
}

void LDF::WriteWString(std::wstring keyname, std::wstring key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)0);
	ldfStream->Write((unsigned short)key.size());
	WriteWSToBitStream(ldfStream, key);
	m_ldfCount++;
}

void LDF::WriteBool(std::wstring keyname, bool key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)7);
	if (key)
		ldfStream->Write((char)1);
	else
		ldfStream->Write((char)0);
	m_ldfCount++;
}

void LDF::WriteOBJID(std::wstring keyname, unsigned long long key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)9);
	ldfStream->Write(key);
	m_ldfCount++;
}

void LDF::WriteS64(std::wstring keyname, unsigned long long key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)8);
	ldfStream->Write(key);
	m_ldfCount++;
}

void LDF::WriteFloat(std::wstring keyname, float key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)3);
	ldfStream->Write(key);
	m_ldfCount++;
}

void LDF::WriteU32(std::wstring keyname, unsigned long key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)5);
	ldfStream->Write(key);
	m_ldfCount++;
}

void LDF::WriteS32(std::wstring keyname, long key) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)1);
	ldfStream->Write(key);
	m_ldfCount++;
}

void LDF::WriteXML(std::wstring keyname, RakNet::BitStream* data, unsigned long xmllength) {
	ldfStream->Write((char)(keyname.size() * sizeof(wchar_t)));
	WriteWSToBitStream(ldfStream, keyname);
	ldfStream->Write((unsigned char)13);
	ldfStream->Write(xmllength);
	ldfStream->Write(data, data->GetNumberOfBytesUsed() * 8);
	m_ldfCount++;
}

