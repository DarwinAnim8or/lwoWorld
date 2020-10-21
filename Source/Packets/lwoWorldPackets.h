#pragma once
#include "lwoServerPackets.h"
#include "../lwoUser.h"
#include "../lwoUserPool.h"
#include "../Database.h"

namespace lwoWorldPackets {
	void validateClient(RakPeerInterface* rakServer, Packet* packet, lwoUserPool* userPool);
	void createNewMinifig(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
	void clientLoginRequest(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
	void sendMinifigureList(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
	void clientSideLoadComplete(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
	void handleChatMessage(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
	void handleGameMessage(RakPeerInterface* rakServer, Packet* packet, lwoUser* user);
}

enum CharCreatePantsColor : unsigned long {
	PANTS_BRIGHT_RED = 2508,
	PANTS_BRIGHT_ORANGE = 2509,
	PANTS_BRICK_YELLOW = 2511,
	PANTS_MEDIUM_BLUE = 2513,
	PANTS_SAND_GREEN = 2514,
	PANTS_DARK_GREEN = 2515,
	PANTS_EARTH_GREEN = 2516,
	PANTS_EARTH_BLUE = 2517,
	PANTS_BRIGHT_BLUE = 2519,
	PANTS_SAND_BLUE = 2520,
	PANTS_DARK_STONE_GRAY = 2521,
	PANTS_MEDIUM_STONE_GRAY = 2522,
	PANTS_WHITE = 2523,
	PANTS_BLACK = 2524,
	PANTS_REDDISH_BROWN = 2526,
	PANTS_DARK_RED = 2527
};

enum CharCreateShirtColor : unsigned long {
	SHIRT_BRIGHT_RED = 4049,
	SHIRT_BRIGHT_BLUE = 4083,
	SHIRT_BRIGHT_YELLOW = 4117,
	SHIRT_DARK_GREEN = 4151,
	SHIRT_BRIGHT_ORANGE = 4185,
	SHIRT_BLACK = 4219,
	SHIRT_DARK_STONE_GRAY = 4253,
	SHIRT_MEDIUM_STONE_GRAY = 4287,
	SHIRT_REDDISH_BROWN = 4321,
	SHIRT_WHITE = 4355,
	SHIRT_MEDIUM_BLUE = 4389,
	SHIRT_DARK_RED = 4423,
	SHIRT_EARTH_BLUE = 4457,
	SHIRT_EARTH_GREEN = 4491,
	SHIRT_BRICK_YELLOW = 4525,
	SHIRT_SAND_BLUE = 4559,
	SHIRT_SAND_GREEN = 4593
};