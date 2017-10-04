#include "lwoAuthPackets.h"
#include "lwoPacketUtils.h"
#include "BitStream.h"
#include "../Database.h"
#include "../sha256.h"

void lwoAuthPackets::handleLoginPacket(RakPeerInterface* rakServer, Packet* packet) {
	//Only read the username and pass, we don't care about the rest for now.
	std::string sUsername = lwoPacketUtils::readWStringAsString(8, packet);
	std::string sPassword = lwoPacketUtils::readWStringAsString(0x4A, packet);
	sPassword = sha256(sPassword);
	printf("User %s connected with password: %s\n", sUsername.c_str(), sPassword.c_str());

	//TODO: add a login check here & get the server IP for the world server.
	unsigned char cLoginState = 0; //0 = incorrect info, 1 = login ok, 2 = banned

	sql::PreparedStatement* usrQr = Database::CreatePreppedStmt("SELECT `username`, `password`, `bBanned` FROM `accounts` WHERE `username`=? LIMIT 1;");
	usrQr->setString(1, sUsername);
	sql::ResultSet* usrRes = usrQr->executeQuery();
	while (usrRes->next()) {
		if (usrRes->getString(1) == sUsername) {
			if (sPassword == usrRes->getString(2)) {
				cLoginState = 1; //Login info correct

				if (usrRes->getBoolean(3)) {
					cLoginState = 2; //Is banned
					std::cout << "User " << sUsername << " is banned!" << std::endl;
				}
			}
			else {
				std::cout << "User " << sUsername << " entered an incorrect password." << std::endl;
			}
		}
		else {
			std::cout << "No account with name: " << sUsername << std::endl;
		}
	}

	delete usrRes;

	std::string sServerToSendTo = "255.255.255.255"; //so we don't accidentally send people to their localhost
	unsigned short usServerPort = 2002; //default (reserved) port for character create/select

	//Here I query for zoneID = 0, because lwoWorld will run in "character server-mode" when running on zoneID 0.
	//(Port 2002 is also reserved for the "character" server! Other servers start at 2003 but their port is not tied to the world they host.)
	sql::PreparedStatement* serverQr = Database::CreatePreppedStmt("SELECT `ip`, `port` FROM `servers` WHERE `zoneID`='0' LIMIT 1;");
	sql::ResultSet* serverRes = serverQr->executeQuery();
	while (serverRes->next()) {
		if (serverRes->getString(1) != "" && serverRes->getInt(2) == 2002) {
			sServerToSendTo = serverRes->getString(1);
			usServerPort = serverRes->getInt(2);
		}
	}

	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_LOGIN_RESPONSE, &bitStream);
	
	bitStream.Write(cLoginState);
	
	for (int i = 0; i < 66; i++) { //For some reason there's a ton of space here?
		bitStream.Write(unsigned char(0));
	}

	lwoPacketUtils::writeStringToPacket(sServerToSendTo, sServerToSendTo.size(), &bitStream);

	for (int i = 0; i < 57; i++) { //For some reason there's a ton of space here?
		bitStream.Write(unsigned char(0));
	}

	bitStream.Write(usServerPort);
	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
} //handleLoginPacket