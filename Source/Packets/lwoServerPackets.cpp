#include "lwoServerPackets.h"
#include "lwoPacketUtils.h"

bool lwoServerPackets::doHandshake(RakPeerInterface* rakServer, Packet* packet) {
	bool bReturn = false;
	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned int iClientVersion;
	inStream.Read(iClientVersion);

	unsigned int iServerVersion = 130529; //The version the client must have
	if (iClientVersion == iServerVersion) {
		printf("Client version is equal to the server's version!\n");
		sendHandshake(rakServer, packet, iServerVersion, CONN_WORLD);
	}
	else if (iClientVersion > iServerVersion) {
		printf("Received a newer client version: %i\n", iClientVersion);
	}
	else if (iClientVersion < iServerVersion) {
		printf("Received an older client version: %i\n", iClientVersion);
	}
	else {
		printf("Received unknown client version: %i\n", iClientVersion);
	}

	return bReturn;
} //doHandshake

void lwoServerPackets::sendHandshake(RakPeerInterface* rakServer, Packet* packet, unsigned int iServerVersion, unsigned int rConType) {
	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_SERVER, MSG_SERVER_VERSION_CONFIRM, &bitStream);

	unsigned int unknown = 0x93;
	unsigned int processID = 0x1410;
	unsigned short localPort = 0xff;
	std::string localIP = "127.0.0.1"; //hardcoded for now.

	bitStream.Write(iServerVersion);
	bitStream.Write(unknown);
	bitStream.Write(rConType);
	bitStream.Write(processID);
	bitStream.Write(localPort);
	lwoPacketUtils::writeStringToPacket(localIP, localIP.size(), &bitStream);
	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
} //sendHandshake

void lwoServerPackets::disconnectNotify(RakPeerInterface* rakServer, Packet* packet, int iErrorCode) {
	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_SERVER, MSG_SERVER_DISCONNECT_NOTIFY, &bitStream);

	int unknown = 0;

	bitStream.Write(iErrorCode);
	bitStream.Write(unknown);

	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	rakServer->CloseConnection(packet->systemAddress, true, 0);
} //disconnectNotify