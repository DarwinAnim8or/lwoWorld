#include "lwoWorldPackets.h"
#include "lwoPacketUtils.h"


#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

void lwoWorldPackets::validateClient(RakPeerInterface* rakServer, Packet* packet) {
	printf("TODO: client validation.\n");
} //validateClient

void lwoWorldPackets::createNewMinifig(RakPeerInterface* rakServer, Packet* packet) {
	std::cout << "TODO" << std::endl;
} //createNewMinifig

void lwoWorldPackets::sendMinifigureList(RakPeerInterface* rakServer, Packet* packet) {
	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_CHARACTER_LIST_RESPONSE, &bitStream);
	unsigned short usCharacterCount = 0;
	unsigned short usCharacterInFront = 0;
	bitStream.Write(usCharacterCount);
	bitStream.Write(usCharacterInFront);
	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
} //sendMinifigureList