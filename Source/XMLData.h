#include "BitStream.h"
#include <string>
#include <sstream>

class XMLDataWriter {
	RakNet::BitStream* xmlData;
public:
	std::stringstream rawData;
	XMLDataWriter() {}
	XMLDataWriter(RakNet::BitStream* data) { 
		this->xmlData = data; 
	}
	RakNet::BitStream* GetBitStream() { 
		return this->xmlData; 
	}
	void SetBitStream(RakNet::BitStream* xmlData) { 
		this->xmlData = xmlData; 
	}

	void WriteXML(std::string xmlString) {
		for (unsigned int i = 0; i < xmlString.size(); i++) {
			xmlData->Write(xmlString.at(i));
			rawData << xmlString.at(i);
		}
	}
};