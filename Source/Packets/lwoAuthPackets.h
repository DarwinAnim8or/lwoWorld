#pragma once
#include "lwoServerPackets.h"

namespace lwoAuthPackets {
	void handleLoginPacket(RakPeerInterface* rakServer, Packet* packet);
}