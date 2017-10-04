#pragma once
#include "lwoPacketIdentifiers.h"
#include "RakNetTypes.h"
#include "BitStream.h"

namespace lwoPacketHandler {
	void determinePacketHeader(RakPeerInterface* rakServer, Packet* packet);
	void handleServerConnPackets(RakPeerInterface* rakServer, Packet* packet);
	void handleAuthConnPackets(RakPeerInterface* rakServer, Packet* packet);
	void handleChatConnPackets(RakPeerInterface* rakServer, Packet* packet);
	void handleWorldConnPackets(RakPeerInterface* rakServer, Packet* packet);
};