#include <stdio.h>
#include <string.h>
#include <iostream>
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "MessageIdentifiers.h"
#include "Database.h"
#include <conio.h>
#include <time.h>

//========== Non-lwo includes above =|= lwo includes below ============

#include "lwoPacketIdentifiers.h"
#include "lwoPacketHandler.h"
#include "lwoUserPool.h"

//========== lwo includes above =|= code below ===========

std::string g_BaseIP = "localhost";
int g_ourPort;
int g_ourZone;
unsigned int g_ourZoneRevision;

int main(int argc, char* argv[]) {
	RakPeerInterface* rakServer = RakNetworkFactory::GetRakPeerInterface();
	Packet *packet;
	unsigned int iServerVersion = 130529; //The version the client must have
	lwoUserPool* userPool = new lwoUserPool;
	srand(time(NULL));

	std::cout << "===================================================================" << std::endl;
	std::cout << "lwoWorld, a simple test (world) server for the ALPHA version of LU." << std::endl;
	std::cout << "This project is licensed under the GPLv2, and is available at:" << std::endl;
	std::cout << "https://github.com/DarwinAnim8or/lwoWorld " << std::endl;
	std::cout << "While DLU members might work on this; it is not (related to) DLU." << std::endl;
	std::cout << "Server version: 0.0.1" << std::endl;
	std::cout << "===================================================================" << std::endl << std::endl;

	printf("Starting the server.\n");

	//Connect to MySQL:
	//TODO: make a ini reader or whatever for this.
	std::string sMySQLHost = "127.0.0.1";
	std::string sMySQLDatabase = "lwo";
	std::string sMySQLUser = "root";
	std::string sMySQLPass = "";

	unsigned short usServerPort = 2000;
	unsigned short usMaxPlayers = 10;
	unsigned __int64 i64Checksum = 0;
	int iServerGMLevel = 9; //Require dev-level access by default.
	unsigned int iZoneID = 0;

	if (argv[1] != NULL) { 
		iZoneID = atoi(argv[1]); 
	}

	try {
		Database::Connect(sMySQLHost, sMySQLDatabase, sMySQLUser, sMySQLPass);
	}
	catch (MySqlException& ex) {
		std::cout << "MySQL error! " << ex.what() << std::endl;
		std::cout << "Press ENTER to exit." << std::endl;
		std::cin.get();
		exit(0);
	}

	if (argv[1] != NULL) {
		//Gather info regarding our zone and server:
		sql::PreparedStatement* qrInfo = Database::CreatePreppedStmt("SELECT * FROM `zones` WHERE `id`=? LIMIT 1;");
		qrInfo->setInt(1, iZoneID);
		sql::ResultSet* infoRes = qrInfo->executeQuery();

		while (infoRes->next()) {
			i64Checksum = infoRes->getInt64(2);
			usMaxPlayers = infoRes->getInt(3);
			iServerGMLevel = infoRes->getInt(4);
		}

		delete infoRes;

		std::cout << "Got zone info from database:" << std::endl;
		std::cout << "ZoneID: " << iZoneID << std::endl;
		std::cout << "Checksum: " << i64Checksum << std::endl;
		std::cout << "Max players: " << usMaxPlayers << std::endl;
		std::cout << "GM level: " << iServerGMLevel << std::endl;

		g_ourZone = iZoneID;
		g_ourZoneRevision = i64Checksum;
	}
	else {
		usServerPort = 2002; //default to char server
		usMaxPlayers = 50; //since the char server is mostly idle, we can take in more people.
		iServerGMLevel = 0; //GM level restriction not needed on the char server
	}

	//Before we enter our loop, we need to update the servers table so other servers know we're up:
	//(But first we query the database to get the last used port)
	if (iZoneID != 0) {
		sql::PreparedStatement* qrPort = Database::CreatePreppedStmt("SELECT `port` FROM `servers` ORDER BY `port` DESC LIMIT 1;");
		sql::ResultSet* portRes = qrPort->executeQuery();

		while (portRes->next()) {
			usServerPort = portRes->getInt(1);
			std::cout << "Got the latest port: " << usServerPort << std::endl;
		}

		delete portRes;
	}

	//For now, I don't care about cloneID or instanceID.
	if (iZoneID != 0) usServerPort++; //Don't increment the port number if we're running in char mode.
	if (iZoneID != 0 && usServerPort <= 2002) usServerPort = 2003; //Make sure our port number is never below 2002.
	sql::PreparedStatement* qrInsertUs = Database::CreatePreppedStmt("INSERT INTO `servers`(`ip`, `port`, `version`, `zoneID`, `cloneID`, `instanceID`) VALUES(?, ?, ?, ?, ?, ?);");
	qrInsertUs->setString(1, g_BaseIP); //This is hardcoded for now, but it will be in the config file later. 
	qrInsertUs->setInt(2, usServerPort);
	qrInsertUs->setInt(3, iServerVersion);
	qrInsertUs->setInt(4, iZoneID);
	qrInsertUs->setInt(5, 0);
	qrInsertUs->setInt(6, 0);
	qrInsertUs->executeQuery();
	g_ourPort = usServerPort;

	
	if (!rakServer->Startup(usMaxPlayers, 30, &SocketDescriptor(usServerPort, 0), 1)) {
		std::cout << "RakNet Server failed to start" << std::endl;
		std::cin.get();
		exit(0);
	}
	rakServer->SetIncomingPassword("3.25 ND1", 8);
	rakServer->SetMaximumIncomingConnections(usMaxPlayers);

	std::cout << "[INFO] Pressing ANY key will quit the server. DO NOT USE ANY OTHER WAY OF CLOSING IT!" << std::endl;
	std::cout << "[INFO] Not listening to this and closing it using the 'X' or whatever will cause your database to become cluttered." << std::endl << std::endl;
	
	bool bRun = true;
	while (bRun) {
		RakSleep(30); //So that we don't use up a ton of CPU while running.
		packet = rakServer->Receive();

		if (packet) {
			switch (packet->data[0]) {
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("Our connection request has been accepted.\n");
				break;

			case ID_USER_PACKET_ENUM:
				lwoPacketHandler::determinePacketHeader(rakServer, packet, userPool); // really just a "first pass" if you will to see if it's of any use to us.
				break;

			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
					printf("A client has disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
				printf("A client lost the connection.\n");
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}

			rakServer->DeallocatePacket(packet);
		}

		if (_kbhit()) {
			bRun = false;
			std::cout << "Closing server..." << std::endl;
		}
	}

	RakNetworkFactory::DestroyRakPeerInterface(rakServer);

	//We need to delete our entry from the servers table:
	sql::PreparedStatement* qrDeleteUs = Database::CreatePreppedStmt("DELETE FROM `servers` WHERE ip=? AND port=? AND version=? AND zoneID=? AND cloneID=? AND instanceID=? LIMIT 1;");
	qrDeleteUs->setString(1, "localhost"); //This is hardcoded for now, but it will be in the config file later. 
	qrDeleteUs->setInt(2, usServerPort);
	qrDeleteUs->setInt(3, iServerVersion);
	qrDeleteUs->setInt(4, iZoneID);
	qrDeleteUs->setInt(5, 0);
	qrDeleteUs->setInt(6, 0);
	qrDeleteUs->executeQuery();

	return 0;
}
