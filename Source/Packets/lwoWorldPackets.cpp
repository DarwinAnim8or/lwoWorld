#include "lwoWorldPackets.h"
#include "lwoPacketUtils.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

//Local functions as they aren't needed by anything else, leave the implementations at the bottom!
unsigned long FindCharShirtID(unsigned long shirtColor, unsigned long shirtStyle);
unsigned long FindCharPantsID(unsigned long pantsColor);

void lwoWorldPackets::validateClient(RakPeerInterface* rakServer, Packet* packet, lwoUserPool* userPool) {
	//This packet has changed a LOT since alpha and we don't have any clue as to what all the other data is,
	//The only thing that stayed the same/looks useful to us at face value is the username.
	std::string sUsername = lwoPacketUtils::readWStringAsString(8, packet);
	std::cout << "User: " << sUsername << " is connecting!" << std::endl;
	
	unsigned __int64 iAccountID = 0;
	int iGMLevel = 0;
	sql::PreparedStatement* accInfo = Database::CreatePreppedStmt("SELECT id, gmlevel FROM accounts WHERE username=? LIMIT 1;");
	accInfo->setString(1, sUsername);
	sql::ResultSet* accRes = accInfo->executeQuery();
	while (accRes->next()) {
		iAccountID = accRes->getInt64(1);
		iGMLevel = accRes->getInt(2);
	}

	shared_ptr<lwoUser> userToInsert = make_shared<lwoUser>(iAccountID, sUsername, packet->systemAddress);
	userPool->Insert(packet->systemAddress, userToInsert);
} //validateClient

void lwoWorldPackets::createNewMinifig(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	std::string sMinifigName = lwoPacketUtils::readWStringAsString(8, packet);
	int iShirtColor = lwoPacketUtils::readInt(0x52, 0x55, packet);
	int iShirtStyle = lwoPacketUtils::readInt(0x56, 0x59, packet);
	int iPantsColor = lwoPacketUtils::readInt(0x5A, 0x5D, packet);

	int iHairStyle = lwoPacketUtils::readInt(0x5E, 0x61, packet); //Are these switched? 
	int iHairColor = lwoPacketUtils::readInt(0x62, 0x65, packet);

	int iLH = lwoPacketUtils::readInt(0x66, 0x69, packet);
	int iRH = lwoPacketUtils::readInt(0x6A, 0x6D, packet);
	int iEyebrows = lwoPacketUtils::readInt(0x6E, 0x71, packet);
	int iEyes = lwoPacketUtils::readInt(0x72, 0x75, packet);
	int iMouth = lwoPacketUtils::readInt(0x76, 0x79, packet);

	unsigned long iShirtID = FindCharShirtID(iShirtColor, iShirtStyle);
	unsigned long iPantsID = FindCharPantsID(iPantsColor);

	cout << "ShirtID: " << iShirtID << " iPantsID: " << iPantsID << endl;

	//generate a random minifig name:
	//Minifig names in alpha were basically "Minifig1664233633" etc. (which is conviently the size of a Unix Timestamp)
	std::time_t result = std::time(nullptr);
	std::asctime(std::localtime(&result));
	std::string sMinifigTempName = "Minifig" + to_string(result);
	cout << "Randomly generated minifig temp name: " << sMinifigTempName << endl;

	sql::PreparedStatement* nameCheck = Database::CreatePreppedStmt("SELECT `objectID` FROM `minifigs` WHERE `playerName`=? LIMIT 1;");
	nameCheck->setString(1, sMinifigName);
	sql::ResultSet* nameRes = nameCheck->executeQuery();

	if (nameRes->rowsCount() == 0) {
		sql::PreparedStatement* insertFig = Database::CreatePreppedStmt("INSERT INTO `minifigs`(`accountID`, `playerName`, `tempName`, `bNameApproved`, `gmlevel`, `eyes`, `eyeBrows`, `mouth`, `hair`, `hairColor`, `health`, `maxHealth`, `lastZoneID`) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)");
		insertFig->setInt64(1, user->UserID());
		insertFig->setString(2, sMinifigName);
		insertFig->setString(3, sMinifigTempName);
		insertFig->setBoolean(4, 0);
		insertFig->setInt(5, 0);
		insertFig->setInt(6, iEyes);
		insertFig->setInt(7, iEyebrows);
		insertFig->setInt(8, iMouth);
		insertFig->setInt(9, iHairStyle);
		insertFig->setInt(10, iHairColor);
		insertFig->setInt(11, 4);
		insertFig->setInt(12, 4);
		insertFig->setInt(13, 1000);
		insertFig->executeQuery();

		RakNet::BitStream bitStream;
		lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_CHARACTER_CREATE_RESPONSE, &bitStream);
		bitStream.Write(unsigned char(0));
		rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		lwoWorldPackets::sendMinifigureList(rakServer, packet, user);
	}
	else {
		/*
		Response codes:
		0 - No error
		1 - Unable to create character
		2 - That name is unavailable
		3 - The name you chose is already in use by another minifig. Please choose another name.
		*/

		RakNet::BitStream bitStream;
		lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_CHARACTER_CREATE_RESPONSE, &bitStream);
		bitStream.Write(unsigned char(3));
		rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	}	
} //createNewMinifig

void lwoWorldPackets::clientLoginRequest(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned __int64 i64ObjectID = inStream.Read(i64ObjectID);
	std::cout << "User x wants to log into the world using objectID: " << i64ObjectID << std::endl;

	//TODO: Actually redirect the player here by looking up which zone that character was last in & the server for it.
} //clientLoginRequest

void lwoWorldPackets::sendMinifigureList(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_CHARACTER_LIST_RESPONSE, &bitStream);
	unsigned short usCharacterCount = 0;
	unsigned short usCharacterInFront = 0;

	sql::PreparedStatement* infoQR = Database::CreatePreppedStmt("SELECT * FROM minifigs WHERE `accountID`=? LIMIT 4;");
	infoQR->setInt64(1, user->UserID());
	sql::ResultSet* infoRes = infoQR->executeQuery();

	usCharacterCount = infoRes->rowsCount();
	bitStream.Write(usCharacterCount);
	bitStream.Write(usCharacterInFront);

	//NOTE -- THIS IS ENTIRELY WRONG! 
	//Still need to experiment with it, DO NOT USE!
	while (infoRes->next()) {
		bitStream.Write(infoRes->getInt64(1));
		bitStream.Write(int(0));
		lwoPacketUtils::writeStringToPacket(infoRes->getString(3), 33, &bitStream);
		lwoPacketUtils::writeStringToPacket(infoRes->getString(4), 33, &bitStream);
		bitStream.Write(unsigned char(infoRes->getBoolean(5)));
		bitStream.Write(unsigned long long(0));
		bitStream.Write(unsigned short(0));
		bitStream.Write(unsigned int(0));
		bitStream.Write(unsigned int(2));
		bitStream.Write(infoRes->getInt(11));
		bitStream.Write(infoRes->getInt(10));
		bitStream.Write(unsigned int(0));
		bitStream.Write(unsigned int(0));
		bitStream.Write(infoRes->getInt(8));
		bitStream.Write(infoRes->getInt(7));
		bitStream.Write(infoRes->getInt(9));
		bitStream.Write(unsigned int(0));
		bitStream.Write(unsigned short(infoRes->getInt(19)));
		bitStream.Write(unsigned short(0));
		bitStream.Write(unsigned int(0));
		bitStream.Write(unsigned long long(0));

		//Add the items:
		bitStream.Write(unsigned short(0)); //item count
	}

	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
} //sendMinifigureList

void lwoWorldPackets::clientSideLoadComplete(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned short usZoneID = inStream.Read(usZoneID);
	unsigned short usInstanceID = inStream.Read(usInstanceID);
	unsigned int uiMapClone = inStream.Read(uiMapClone);
	std::cout << "User x is done loading the zone, so send charData. (" << usZoneID << ":" << usInstanceID << ":" << uiMapClone << ")" << std::endl;

	//TODO: Generate and send the character data (charData) now. 
} //clientSideLoadComplete

unsigned long FindCharShirtID(unsigned long shirtColor, unsigned long shirtStyle) {
	unsigned long shirtID = 0;

	switch (shirtColor) {
	case 0: {
		shirtID = shirtStyle >= 35 ? 5730 : SHIRT_BRIGHT_RED;
		break;
	}

	case 1: {
		shirtID = shirtStyle >= 35 ? 5736 : SHIRT_BRIGHT_BLUE;
		break;
	}

	case 3: {
		shirtID = shirtStyle >= 35 ? 5808 : SHIRT_DARK_GREEN;
		break;
	}

	case 5: {
		shirtID = shirtStyle >= 35 ? 5754 : SHIRT_BRIGHT_ORANGE;
		break;
	}

	case 6: {
		shirtID = shirtStyle >= 35 ? 5760 : SHIRT_BLACK;
		break;
	}

	case 7: {
		shirtID = shirtStyle >= 35 ? 5766 : SHIRT_DARK_STONE_GRAY;
		break;
	}

	case 8: {
		shirtID = shirtStyle >= 35 ? 5772 : SHIRT_MEDIUM_STONE_GRAY;
		break;
	}

	case 9: {
		shirtID = shirtStyle >= 35 ? 5778 : SHIRT_REDDISH_BROWN;
		break;
	}

	case 10: {
		shirtID = shirtStyle >= 35 ? 5784 : SHIRT_WHITE;
		break;
	}

	case 11: {
		shirtID = shirtStyle >= 35 ? 5802 : SHIRT_MEDIUM_BLUE;
		break;
	}

	case 13: {
		shirtID = shirtStyle >= 35 ? 5796 : SHIRT_DARK_RED;
		break;
	}

	case 14: {
		shirtID = shirtStyle >= 35 ? 5802 : SHIRT_EARTH_BLUE;
		break;
	}

	case 15: {
		shirtID = shirtStyle >= 35 ? 5808 : SHIRT_EARTH_GREEN;
		break;
	}

	case 16: {
		shirtID = shirtStyle >= 35 ? 5814 : SHIRT_BRICK_YELLOW;
		break;
	}

	case 84: {
		shirtID = shirtStyle >= 35 ? 5820 : SHIRT_SAND_BLUE;
		break;
	}

	case 96: {
		shirtID = shirtStyle >= 35 ? 5826 : SHIRT_SAND_GREEN;
		shirtColor = 16;
		break;
	}
	}

	// Initialize another variable for the shirt color
	unsigned long editedShirtColor = shirtID;

	// This will be the final shirt ID
	unsigned long shirtIDFinal;

	// For some reason, if the shirt color is 35 - 40,
	// The ID is different than the original... Was this because
	// these shirts were added later?
	if (shirtStyle >= 35) {
		shirtIDFinal = editedShirtColor += (shirtStyle - 35);
	}
	else {
		// Get the final ID of the shirt by adding the shirt
		// style to the editedShirtColor
		shirtIDFinal = editedShirtColor += (shirtStyle - 1);
	}

	//cout << "Shirt ID is: " << shirtIDFinal << endl;

	return shirtIDFinal;
}

unsigned long FindCharPantsID(unsigned long pantsColor) {
	unsigned long pantsID = 2508;

	switch (pantsColor) {
	case 0: {
		pantsID = PANTS_BRIGHT_RED;
		break;
	}

	case 1: {
		pantsID = PANTS_BRIGHT_BLUE;
		break;
	}

	case 3: {
		pantsID = PANTS_DARK_GREEN;
		break;
	}

	case 5: {
		pantsID = PANTS_BRIGHT_ORANGE;
		break;
	}

	case 6: {
		pantsID = PANTS_BLACK;
		break;
	}

	case 7: {
		pantsID = PANTS_DARK_STONE_GRAY;
		break;
	}

	case 8: {
		pantsID = PANTS_MEDIUM_STONE_GRAY;
		break;
	}

	case 9: {
		pantsID = PANTS_REDDISH_BROWN;
		break;
	}

	case 10: {
		pantsID = PANTS_WHITE;
		break;
	}

	case 11: {
		pantsID = PANTS_MEDIUM_BLUE;
		break;
	}

	case 13: {
		pantsID = PANTS_DARK_RED;
		break;
	}

	case 14: {
		pantsID = PANTS_EARTH_BLUE;
		break;
	}

	case 15: {
		pantsID = PANTS_EARTH_GREEN;
		break;
	}

	case 16: {
		pantsID = PANTS_BRICK_YELLOW;
		break;
	}

	case 84: {
		pantsID = PANTS_SAND_BLUE;
		break;
	}

	case 96: {
		pantsID = PANTS_SAND_GREEN;
		break;
	}
	}

	//cout << "Pants ID is: " << pantsID << endl;

	return pantsID;
}