#include "lwoWorldPackets.h"
#include "lwoPacketUtils.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

//Local functions as they aren't needed by anything else, leave the implementations at the bottom!
unsigned long FindCharShirtID(unsigned long shirtColor, unsigned long shirtStyle);
unsigned long FindCharPantsID(unsigned long pantsColor);

extern std::string g_BaseIP;
extern int g_ourPort;
extern int g_ourZone;
extern unsigned int g_ourZoneRevision;

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

	if (g_ourPort != 2002) { //if not running as char, send TransferToZone/LoadStaticZone:
		RakNet::BitStream bitStream;
		lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_LOAD_STATIC_ZONE, &bitStream);
		bitStream.Write(unsigned short(g_ourZone));
		bitStream.Write(unsigned short(0)); //instance
		bitStream.Write(unsigned int(0)); //clone
		bitStream.Write(unsigned int(g_ourZoneRevision)); 
		bitStream.Write(unsigned short(0)); //???
		bitStream.Write(float(0));
		bitStream.Write(float(0));
		bitStream.Write(float(0));
		bitStream.Write(unsigned int(0)); //== 4 if battle instance
		rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	}
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
		sql::PreparedStatement* insertFig = Database::CreatePreppedStmt("INSERT INTO `minifigs`(`accountID`, `playerName`, `tempName`, `bNameApproved`, `gmlevel`, `eyes`, `eyeBrows`, `mouth`, `hair`, `hairColor`, `health`, `maxHealth`, `lastZoneID`, `lh`, `rh`) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
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
		insertFig->setInt(14, iLH);
		insertFig->setInt(15, iRH);
		insertFig->executeQuery();

		//Insert the items:
		unsigned long long playerObjectID = 0;
		sql::PreparedStatement* getObjID = Database::CreatePreppedStmt("SELECT `objectID` FROM `minifigs` WHERE `playerName`=? LIMIT 1;");
		getObjID->setString(1, sMinifigName);
		sql::ResultSet* objRes = getObjID->executeQuery();
		while (objRes->next()) {
			playerObjectID = objRes->getUInt64(1);
		}

		delete objRes;
		
		sql::PreparedStatement* insertShirt = Database::CreatePreppedStmt("INSERT INTO `items`(`ownerID`, `LOT`, `bEquipped`, `equipLocation`, `slot`, `bagID`, `count`) VALUES (?,?,?,?,?,?,?)");
		insertShirt->setInt64(1, playerObjectID);
		insertShirt->setInt(2, iShirtID);
		insertShirt->setInt(3, 1);
		insertShirt->setString(4, "torso");
		insertShirt->setInt(5, 1);
		insertShirt->setInt(6, 1);
		insertShirt->setInt(7, 1);
		insertShirt->executeQuery();

		sql::PreparedStatement* insertPants = Database::CreatePreppedStmt("INSERT INTO `items`(`ownerID`, `LOT`, `bEquipped`, `equipLocation`, `slot`, `bagID`, `count`) VALUES (?,?,?,?,?,?,?)");
		insertPants->setInt64(1, playerObjectID);
		insertPants->setInt(2, iPantsID);
		insertPants->setInt(3, 1);
		insertPants->setString(4, "legs");
		insertPants->setInt(5, 2);
		insertPants->setInt(6, 1);
		insertPants->setInt(7, 1);
		insertPants->executeQuery();

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
	unsigned __int64 i64ObjectID = lwoPacketUtils::readLongLong(0x08, 0x0F, packet);
	std::cout << "User: " << user->Username() << " wants to log into the world using objectID: " << i64ObjectID << std::endl;

	//Get the lastZoneID:
	int iLastZoneID = 1000;
	sql::PreparedStatement* getLZoneID = Database::CreatePreppedStmt("SELECT `lastZoneID` FROM `minifigs` WHERE `objectID`=? LIMIT 1;");
	getLZoneID->setUInt64(1, i64ObjectID);
	sql::ResultSet* zoneRes = getLZoneID->executeQuery();
	while (zoneRes->next()) {
		iLastZoneID = zoneRes->getInt(1);
	}

	delete zoneRes;

	//Check which server:
	std::string sIP = "";
	int iPort = 2003;
	sql::PreparedStatement* getServerQR = Database::CreatePreppedStmt("SELECT `ip`, `port` FROM `servers` WHERE `zoneID`=? LIMIT 1;");
	getServerQR->setInt(1, iLastZoneID);
	sql::ResultSet* serverRes = getServerQR->executeQuery();

	while (serverRes->next()) {
		sIP = serverRes->getString(1);
		iPort = serverRes->getInt(2);
	}

	if (serverRes->rowsCount() == 0) { //time to start this server...
		std::string cmd = "start lwoWorld.exe " + to_string(iLastZoneID);
		system(cmd.c_str()); //lazy but it works.
		sIP = g_BaseIP;
	}

	delete serverRes;

	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_TRANSFER_TO_WORLD, &bitStream);
	lwoPacketUtils::writeStringToPacket(sIP, 33, &bitStream);
	bitStream.Write(unsigned short(iPort)); //padding?
	bitStream.Write(unsigned short(iPort));
	bitStream.Write(unsigned char(0)); //bIsMythranShift
	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	lwoPacketUtils::savePacket("clientLoginRequestResponse.bin", (char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed());
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
	cout << "Got: " << usCharacterCount << " figs for that user." << endl;

	bitStream.Write((unsigned char)usCharacterCount);
	bitStream.Write((unsigned char)usCharacterInFront);

	while (infoRes->next()) {
		bitStream.Write(infoRes->getInt64(1));
		bitStream.Write(int(0)); //???

		std::string sPlayerTempName = infoRes->getString(4);
		std::wstring wsPlayerTempName = lwoPacketUtils::StringToWString(sPlayerTempName, sPlayerTempName.size());
		bitStream.Write((char*)wsPlayerTempName.data(), sizeof(wchar_t) * wsPlayerTempName.size());
		for (unsigned int i = 0; i < (66 - wsPlayerTempName.size() * 2); i++) {
			unsigned char byte = 0;
			bitStream.Write(byte);
		}

		std::string sPlayerName = infoRes->getString(3);
		std::wstring wsPlayerName = lwoPacketUtils::StringToWString(sPlayerName, sPlayerName.size());
		bitStream.Write((char*)wsPlayerName.data(), sizeof(wchar_t) * wsPlayerName.size());
		for (unsigned int i = 0; i < (66 - wsPlayerName.size() * 2); i++) {
			unsigned char byte = 0;
			bitStream.Write(byte);
		}

		bitStream.Write(unsigned char(infoRes->getBoolean(5))); //bNameApproved
		bitStream.Write(unsigned char(0)); //???
		bitStream.Write(unsigned char(0)); //???
		bitStream.Write(unsigned char(0x4D)); //???
		bitStream.Write(unsigned long long(0)); //???
		bitStream.Write(unsigned int(0)); //shirt color
		bitStream.Write(unsigned int(6)); //shirt style
		bitStream.Write(unsigned int(6)); //pants color
		bitStream.Write(infoRes->getInt(10)); //hair style
		bitStream.Write(infoRes->getInt(11)); //hair color
		bitStream.Write(infoRes->getInt(20)); //lh
		bitStream.Write(infoRes->getInt(21)); //rh
		bitStream.Write(infoRes->getInt(8)); //eyebrows
		bitStream.Write(infoRes->getInt(7)); //eyes
		bitStream.Write(infoRes->getInt(9)); //mouth
		bitStream.Write(unsigned int(0)); //???
		bitStream.Write(unsigned short(infoRes->getInt(19))); //lastZoneId
		bitStream.Write(unsigned short(0)); //lastMapInstance
		bitStream.Write(unsigned int(0)); //lastMapClone
		bitStream.Write(unsigned long long(0)); //last login timestamp

		//Add the items:
		sql::PreparedStatement* itemsQR = Database::CreatePreppedStmt("SELECT `LOT` FROM `items` WHERE `ownerID`=? AND `bEquipped`=1 LIMIT 6;");
		itemsQR->setInt64(1, infoRes->getInt64(1));
		sql::ResultSet* itemsRes = itemsQR->executeQuery();

		bitStream.Write(unsigned short(itemsRes->rowsCount())); //item count
		cout << "User: " << sPlayerName << " (" << infoRes->getInt64(1) << ") has " << itemsRes->rowsCount() << " items equipped." << endl;
		while (itemsRes->next()) {
			bitStream.Write(itemsRes->getInt(1));
		}
	}

	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
	lwoPacketUtils::savePacket("charList.bin", (char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed());
} //sendMinifigureList

vector<unsigned char> OpenPacket(const string& filename) {
	ifstream file(filename, ios::in | ios::binary | ios::ate);
	if (file.is_open()) {
		streamoff fsz = file.tellg();
		vector<unsigned char> r((unsigned int)fsz);
		file.seekg(0, ios::beg);

		file.read((char*)r.data(), fsz);

		file.close();

		return r;
	}
	return vector<unsigned char>(0);
}

void lwoWorldPackets::clientSideLoadComplete(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned short usZoneID = inStream.Read(usZoneID);
	unsigned short usInstanceID = inStream.Read(usInstanceID);
	unsigned int uiMapClone = inStream.Read(uiMapClone);
	std::cout << "User " << user->Username() << " is done loading the zone, so send charData. (" << usZoneID << ":" << usInstanceID << ":" << uiMapClone << ")" << std::endl;

	//Temporary fix
	for (unsigned int i = 1; i < 6; i++) {
		auto charData = OpenPacket(std::to_string(i) + ".bin");
		lwoPacketUtils::ServerSendPacket(rakServer, charData, packet->systemAddress);
	}
} //clientSideLoadComplete

void lwoWorldPackets::handleChatMessage(RakPeerInterface* rakServer, Packet* packet, lwoUser* user) {
	//Read chat message data
	RakNet::BitStream packetStream(packet->data, packet->length, false);
	unsigned long long header; //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned char channelID; //This is typically 0x04 for public channels
	unsigned short unknown; //Unknown
	unsigned long messageLength; //Length of the message

	packetStream.Read(header);
	packetStream.Read(channelID);
	packetStream.Read(unknown);
	packetStream.Read(messageLength);

	vector<wchar_t> msgVector;
	msgVector.reserve(messageLength);
	for (unsigned long k = 0; k < messageLength; k++) {
		wchar_t mchr = packetStream.Read(mchr);
		msgVector.push_back(mchr);
	}

	std::wstring message(msgVector.begin(), msgVector.end());
	std::string wstr(message.begin(), message.end());
}

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