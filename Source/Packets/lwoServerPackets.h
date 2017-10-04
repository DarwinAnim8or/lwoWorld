#pragma once
#include "../RakNet/RakPeerInterface.h"
#include "../lwoPacketIdentifiers.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include <string>

namespace lwoServerPackets {
	bool doHandshake(RakPeerInterface* rakServer, Packet* packet);
	void sendHandshake(RakPeerInterface* rakServer, Packet* packet, unsigned int iServerVersion, unsigned int rConType);
	void disconnectNotify(RakPeerInterface* rakServer, Packet* packet, int iErrorCode);
}