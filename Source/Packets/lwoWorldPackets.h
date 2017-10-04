#pragma once
#include "lwoServerPackets.h"

namespace lwoWorldPackets {
	void validateClient(RakPeerInterface* rakServer, Packet* packet);
	void createNewMinifig(RakPeerInterface* rakServer, Packet* packet);
	void sendMinifigureList(RakPeerInterface* rakServer, Packet* packet);
}