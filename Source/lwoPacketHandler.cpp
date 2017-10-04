#include "lwoPacketHandler.h"
#include "./Packets/lwoServerPackets.h"
#include "./Packets/lwoAuthPackets.h"
#include "./Packets/lwoWorldPackets.h"
#include <stdio.h>

void lwoPacketHandler::determinePacketHeader(RakPeerInterface* rakServer, Packet* packet) {
	switch (packet->data[1]) { //First byte determines the connection type.
	case CONN_SERVER:
		handleServerConnPackets(rakServer, packet);
		break;
	case CONN_AUTH:
		handleAuthConnPackets(rakServer, packet);
		break;
	case CONN_CHAT:
		handleChatConnPackets(rakServer, packet);
		break;
	case CONN_WORLD:
		handleWorldConnPackets(rakServer, packet);
		break;
	default:
		printf("Unknown connection type: %i\n", packet->data[1]);
		break;
	}
} //determinePacketHeader

void lwoPacketHandler::handleServerConnPackets(RakPeerInterface* rakServer, Packet* packet) {
	switch (packet->data[3]) {
	case MSG_SERVER_VERSION_CONFIRM: {
		printf("Received version confirm request.\n");
		lwoServerPackets::doHandshake(rakServer, packet);
		break;
	}
	case MSG_SERVER_GENERAL_NOTIFY: {
		printf("Received a general notification.\n");
		break;
	}
	case MSG_SERVER_DISCONNECT_NOTIFY: {
		printf("Received disconnection notification.\n");
		break;
	}
	default:
		printf("Unknown server packet: %i\n", packet->data[3]);
		break;
	}
} //handleServerConnPackets

void lwoPacketHandler::handleAuthConnPackets(RakPeerInterface* rakServer, Packet* packet) {
	switch (packet->data[3]) {
	case MSG_AUTH_LOGIN_REQUEST: {
		printf("Received login request.\n");
		lwoAuthPackets::handleLoginPacket(rakServer, packet);
		break;
	}
	default:
		printf("Unknown auth packet: %i\n", packet->data[3]);
		break;
	}
} //handleServerConnPackets

void lwoPacketHandler::handleChatConnPackets(RakPeerInterface* rakServer, Packet* packet) {
	//TODO
} //handleServerConnPackets

void lwoPacketHandler::handleWorldConnPackets(RakPeerInterface* rakServer, Packet* packet) {
	switch (packet->data[3]) {
	case MSG_WORLD_CLIENT_VALIDATION: {
		lwoWorldPackets::validateClient(rakServer, packet);
		break;
	}
	case MSG_WORLD_CLIENT_CHARACTER_CREATE_REQUEST: {
		lwoWorldPackets::createNewMinifig(rakServer, packet);
		break;
	}
	case MSG_WORLD_CLIENT_CHARACTER_LIST_REQUEST: {
		lwoWorldPackets::sendMinifigureList(rakServer, packet);
		break;
	}
	default:
		printf("Unknown world packet ID: %i\n", packet->data[3]);
		break;
	}
} //handleServerConnPackets