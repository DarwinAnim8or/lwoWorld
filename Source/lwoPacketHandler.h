#pragma once
#include "lwoPacketIdentifiers.h"
#include "lwoUserPool.h"
#include "RakNetTypes.h"
#include "BitStream.h"

namespace lwoPacketHandler {
	void determinePacketHeader(RakPeerInterface* rakServer, Packet* packet, lwoUserPool* userPool);
	void handleServerConnPackets(RakPeerInterface* rakServer, Packet* packet);
	void handleAuthConnPackets(RakPeerInterface* rakServer, Packet* packet);
	void handleChatConnPackets(RakPeerInterface* rakServer, Packet* packet, lwoUserPool* userPool);
	void handleWorldConnPackets(RakPeerInterface* rakServer, Packet* packet, lwoUserPool* userPool);
};