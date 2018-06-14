#include "lwoWorldPackets.h"
#include "lwoPacketUtils.h"
#include "../LDF.h"
#include "../XMLData.h"
#include "ReplicaManager.h"

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
void ConstructObject(RakPeerInterface* rakServer, Packet* packet, unsigned long long objectID, std::wstring name, unsigned long LOT);
void SendCharData(RakPeerInterface* rakServer, Packet* packet, unsigned long long objectID, std::wstring playerName);

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

	int iHairStyle = lwoPacketUtils::readInt(0x5E, 0x61, packet);
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

void testSendCharEndMarker(RakPeerInterface* rakServer, Packet* packet)
{
	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, 62, &bitStream);
	bool bIsServerOnline = true;
	bitStream.Write((unsigned char)bIsServerOnline);
	rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
}

void lwoWorldPackets::clientSideLoadComplete(RakPeerInterface* rakServer, Packet* packet, lwoUser* user, ReplicaManager* replicaManager) {
	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned short usZoneID = inStream.Read(usZoneID);
	unsigned short usInstanceID = inStream.Read(usInstanceID);
	unsigned int uiMapClone = inStream.Read(uiMapClone);
	std::cout << "User " << user->Username() << " is done loading the zone, so send charData. (" << usZoneID << ":" << usInstanceID << ":" << uiMapClone << ")" << std::endl;

	//Get the objectID:
	__int64 objectID;
	std::string playerName;
	std::string tempName;
	bool bNameApproved = false;

	sql::PreparedStatement* minifigQR = Database::CreatePreppedStmt("SELECT `objectID`, `playerName`, `tempName`, `bNameApproved` FROM `minifigs` WHERE `accountID`=? LIMIT 1;");
	minifigQR->setInt64(1, user->UserID());
	sql::ResultSet* res = minifigQR->executeQuery();
	while (res->next()) {
		objectID = res->getInt64(1);
		playerName = res->getString(2);
		tempName = res->getString(3);
		bNameApproved = res->getBoolean(4);
	}

	std::wstring nameToUse;
	if (bNameApproved) nameToUse = lwoPacketUtils::StringToWString(playerName, playerName.size());
	else nameToUse = lwoPacketUtils::StringToWString(tempName, tempName.size());

	std::cout << "Creating player " << lwoPacketUtils::WStringToString(nameToUse, nameToUse.size()) << ":" << objectID << std::endl;
	SendCharData(rakServer, packet, objectID, nameToUse);

	Player* playerObj = new Player{};
	playerObj->objectName = nameToUse;
	playerObj->objectID = objectID;
	playerObj->position = Vector3{ -629.0f, 613.4f, -30.0f };
	playerObj->gmLevel = 9; //temp

	testSendCharEndMarker(rakServer, packet);
	//ConstructObject(rakServer, packet, objectID, nameToUse, 1);
	replicaManager->Construct(playerObj, false, UNASSIGNED_SYSTEM_ADDRESS, true);
} //clientSideLoadComplete

void lwoWorldPackets::positionUpdate(RakPeerInterface * rakServer, Packet * packet, lwoUser * user, ReplicaManager * replicaManager) {
	/*
		[L:4] - player pos x, float
		[L:4] - player pos y, float
		[L:4] - player pos z, float
		[L:4] - player rotation x (or z), float
		[L:4] - player rotation y, float
		[L:4] - player rotation z (or x), float
		[L:4] - player rotation w, float
		[L:BIT1] - is player on ground, bool
		[L:BIT1] - ???
		[L:BIT1] - flag
			[L:4] - velocity x, float
			[L:4] - velocity y, float
			[L:4] - velocity z, float
		[L:BIT1] - flag
			[L:4] - angular velocity x, float
			[L:4] - angular velocity y, float
			[L:4] - angular velocity z, float
		[L:BIT1] - flag
			[L:8] - ???, seemed like an object id in the 53-04-00-16 captures
			[L:4] - ???, float
			[L:4] - ???, float
			[L:4] - ???, float
			[L:BIT1] - flag
				[L:4] - ???, float
				[L:4] - ???, float
				[L:4] - ???, float
	*/

	RakNet::BitStream inStream(packet->data, packet->length, false);
	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	Vector3 position{ 0.0f, 0.0f, 0.0f };
	Quaternion rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
	Vector3 velocity{ 0.0f, 0.0f, 0.0f };
	Vector3 angVelocity{ 0.0f, 0.0f, 0.0f };
	
	position.x = inStream.Read(position.x);
	position.y = inStream.Read(position.x);
	position.z = inStream.Read(position.z);
	rotation.x = inStream.Read(rotation.x);
	rotation.y = inStream.Read(rotation.y);
	rotation.z = inStream.Read(rotation.z);
	rotation.w = inStream.Read(rotation.w);

	bool bIsOnGround = inStream.Read(bIsOnGround);
	bool bUnknown = inStream.Read(bUnknown);
	bool bHasVelocityUpdate = inStream.Read(bHasVelocityUpdate);

	if (bHasVelocityUpdate) {
		velocity.x = inStream.Read(velocity.x);
		velocity.y = inStream.Read(velocity.y);
		velocity.z = inStream.Read(velocity.z);
	}

	bool bHasAngularVelocityUpdate = inStream.Read(bHasAngularVelocityUpdate);

	if (bHasAngularVelocityUpdate) {
		angVelocity.x = inStream.Read(angVelocity.x);
		angVelocity.y = inStream.Read(angVelocity.y);
		angVelocity.z = inStream.Read(angVelocity.z);
	}

	//Apply them:
	if (user->getPlayer() != nullptr) {
		printf("Applied pos update\n");
		user->getPlayer()->position = position;
		user->getPlayer()->rotation = rotation;
		user->getPlayer()->bIsOnGround = bIsOnGround;

		if (bHasVelocityUpdate) user->getPlayer()->velocity = velocity;
		if (bHasAngularVelocityUpdate) user->getPlayer()->angularVelocity = angVelocity;
	}
}

class AddItemToInventory {
	static const unsigned short MsgID = 225;

public:
	AddItemToInventory() {
		bBound = false;
		bBuyback = false;
		bMailItemsIfInvFull = false;
		eLootTypeSource = 0;
		iLootTypeSourceID = 0;
		iSubkey = 0;
		invType = 0;
		itemCount = 1;
		itemsTotal = 0;
		showFlyingLoot = true;
	}

	AddItemToInventory(unsigned int _extraInfo, unsigned long long _iObjID, unsigned int _iObjTemplate, Vector3 _ni3FlyingLootPosit, unsigned long long _outDestObjID, bool _wasAdded, bool _bBound = false, bool _bBuyback = false, bool _bMailItemsIfInvFull = false, int _eLootTypeSource = 0, unsigned long long _iLootTypeSourceID = 0, unsigned long long _iSubkey = 0, int _invType = 0, unsigned int _itemCount = 1, unsigned int _itemsTotal = 0, bool _showFlyingLoot = true) {
		bBound = _bBound;
		bBuyback = _bBuyback;
		bMailItemsIfInvFull = _bMailItemsIfInvFull;
		eLootTypeSource = _eLootTypeSource;
		extraInfo = _extraInfo;
		iLootTypeSourceID = _iLootTypeSourceID;
		iObjID = _iObjID;
		iObjTemplate = _iObjTemplate;
		iSubkey = _iSubkey;
		invType = _invType;
		itemCount = _itemCount;
		itemsTotal = _itemsTotal;
		ni3FlyingLootPosit = _ni3FlyingLootPosit;
		outDestObjID = _outDestObjID;
		showFlyingLoot = _showFlyingLoot;
		wasAdded = _wasAdded;
	}

	AddItemToInventory(RakNet::BitStream* stream) {
		bBound = false;
		bBuyback = false;
		bMailItemsIfInvFull = false;
		eLootTypeSource = 0;
		iLootTypeSourceID = 0;
		iSubkey = 0;
		invType = 0;
		itemCount = 1;
		itemsTotal = 0;
		showFlyingLoot = true;

		Deserialize(stream);
	}

	~AddItemToInventory() {
	}

	void Serialize(RakNet::BitStream* stream) {
		stream->Write((unsigned short)MsgID);

		stream->Write(bBound);
		stream->Write(bBuyback);
		stream->Write(bMailItemsIfInvFull);

		stream->Write(eLootTypeSource != 0);
		if (eLootTypeSource != 0) stream->Write(eLootTypeSource);

		stream->Write(extraInfo);

		stream->Write(iLootTypeSourceID != 0);
		if (iLootTypeSourceID != 0) stream->Write(iLootTypeSourceID);

		stream->Write(iObjID);
		stream->Write(iObjTemplate);

		stream->Write(iSubkey != 0);
		if (iSubkey != 0) stream->Write(iSubkey);

		stream->Write(invType != 0);
		if (invType != 0) stream->Write(invType);

		stream->Write(itemCount != 1);
		if (itemCount != 1) stream->Write(itemCount);

		stream->Write(itemsTotal != 0);
		if (itemsTotal != 0) stream->Write(itemsTotal);

		stream->Write(ni3FlyingLootPosit);
		stream->Write(outDestObjID);
		stream->Write(showFlyingLoot);
		stream->Write(wasAdded);
	}

	bool Deserialize(RakNet::BitStream* stream) {
		stream->Read(bBound);
		stream->Read(bBuyback);
		stream->Read(bMailItemsIfInvFull);

		bool eLootTypeSourceIsDefault;
		stream->Read(eLootTypeSourceIsDefault);
		if (eLootTypeSourceIsDefault != 0) stream->Read(eLootTypeSource);

		stream->Read(extraInfo);

		bool iLootTypeSourceIDIsDefault;
		stream->Read(iLootTypeSourceIDIsDefault);
		if (iLootTypeSourceIDIsDefault != 0) stream->Read(iLootTypeSourceID);

		stream->Read(iObjID);
		stream->Read(iObjTemplate);

		bool iSubkeyIsDefault;
		stream->Read(iSubkeyIsDefault);
		if (iSubkeyIsDefault != 0) stream->Read(iSubkey);

		bool invTypeIsDefault;
		stream->Read(invTypeIsDefault);
		if (invTypeIsDefault != 0) stream->Read(invType);

		bool itemCountIsDefault;
		stream->Read(itemCountIsDefault);
		if (itemCountIsDefault != 0) stream->Read(itemCount);

		bool itemsTotalIsDefault;
		stream->Read(itemsTotalIsDefault);
		if (itemsTotalIsDefault != 0) stream->Read(itemsTotal);

		stream->Read(ni3FlyingLootPosit);
		stream->Read(outDestObjID);
		stream->Read(showFlyingLoot);
		stream->Read(wasAdded);

		return true;
	}

	bool bBound;
	bool bBuyback;
	bool bMailItemsIfInvFull;
	int eLootTypeSource;
	unsigned int extraInfo;
	unsigned long long iLootTypeSourceID;
	unsigned long long iObjID;
	unsigned int iObjTemplate;
	unsigned long long iSubkey;
	int invType;
	unsigned int itemCount;
	unsigned int itemsTotal;
	Vector3 ni3FlyingLootPosit;
	unsigned long long outDestObjID;
	bool showFlyingLoot;
	bool wasAdded;
};

/*
class AddItemToInventoryClientSync {
	static const GAME_MSG MsgID = 227;

public:
	AddItemToInventoryClientSync() {
		eLootTypeSource = LOOTTYPE_NONE;
		iSubkey = LWOOBJID_EMPTY;
		invType = INVENTORY_DEFAULT;
		itemCount = 1;
		itemsTotal = 0;
		showFlyingLoot = true;
	}

	AddItemToInventoryClientSync(bool _bBound, bool _bIsBOE, bool _bIsBOP, LwoNameValue _extraInfo, LOT _iObjTemplate, LWOOBJID _newObjID, NiPoint3 _ni3FlyingLootPosit, int _slotID, int _eLootTypeSource = LOOTTYPE_NONE, LWOOBJID _iSubkey = LWOOBJID_EMPTY, int _invType = INVENTORY_DEFAULT, unsigned int _itemCount = 1, unsigned int _itemsTotal = 0, bool _showFlyingLoot = true) {
		bBound = _bBound;
		bIsBOE = _bIsBOE;
		bIsBOP = _bIsBOP;
		eLootTypeSource = _eLootTypeSource;
		extraInfo = _extraInfo;
		iObjTemplate = _iObjTemplate;
		iSubkey = _iSubkey;
		invType = _invType;
		itemCount = _itemCount;
		itemsTotal = _itemsTotal;
		newObjID = _newObjID;
		ni3FlyingLootPosit = _ni3FlyingLootPosit;
		showFlyingLoot = _showFlyingLoot;
		slotID = _slotID;
	}

	AddItemToInventoryClientSync(RakNet::BitStream* stream) {
		eLootTypeSource = LOOTTYPE_NONE;
		iSubkey = LWOOBJID_EMPTY;
		invType = INVENTORY_DEFAULT;
		itemCount = 1;
		itemsTotal = 0;
		showFlyingLoot = true;

		Deserialize(stream);
	}

	~AddItemToInventoryClientSync() {
	}

	void Serialize(RakNet::BitStream* stream) {
		stream->Write((unsigned short)MsgID);

		cout << endl;

		stream->Write(bBound);
		stream->Write(bIsBOE);
		stream->Write(bIsBOP);

		stream->Write(eLootTypeSource != LOOTTYPE_NONE);
		if (eLootTypeSource != LOOTTYPE_NONE) stream->Write(eLootTypeSource);

		//stream->Write(extraInfo.length);
		//stream->Write((char*)extraInfo.name.data(), sizeof(wchar_t) * extraInfo.name.size());
		wcout << L"AddItemToInventoryClientSync extraInfo: " << extraInfo.name << L"\n";
		stream->Write(extraInfo.length);
		if (extraInfo.length > 0) {
			for (int i = 0; i < extraInfo.length; i++) {
				wcout << L"-> Writing character: " << extraInfo.name[i] << L"\n";
				stream->Write(extraInfo.name[i]);
			}
			stream->Write((short)0);
		}

		stream->Write(iObjTemplate);
		cout << "AddItemToInventoryClientSycn iObjTemplate: " << iObjTemplate << endl;
		cout << endl;

		stream->Write(iSubkey != LWOOBJID_EMPTY);
		if (iSubkey != LWOOBJID_EMPTY) stream->Write(iSubkey);

		stream->Write(invType != INVENTORY_DEFAULT);
		if (invType != INVENTORY_DEFAULT) stream->Write(invType);

		stream->Write(itemCount != 1);
		if (itemCount != 1) stream->Write(itemCount);

		stream->Write(itemsTotal != 0);
		if (itemsTotal != 0) stream->Write(itemsTotal);

		stream->Write(newObjID);
		stream->Write(ni3FlyingLootPosit);
		stream->Write(showFlyingLoot);
		stream->Write(slotID);
	}

	bool Deserialize(RakNet::BitStream* stream) {
		stream->Read(bBound);
		stream->Read(bIsBOE);
		stream->Read(bIsBOP);

		bool eLootTypeSourceIsDefault;
		stream->Read(eLootTypeSourceIsDefault);
		if (eLootTypeSourceIsDefault != 0) stream->Read(eLootTypeSource);

		stream->Read(extraInfo);
		stream->Read(iObjTemplate);

		bool iSubkeyIsDefault;
		stream->Read(iSubkeyIsDefault);
		if (iSubkeyIsDefault != 0) stream->Read(iSubkey);

		bool invTypeIsDefault;
		stream->Read(invTypeIsDefault);
		if (invTypeIsDefault != 0) stream->Read(invType);

		bool itemCountIsDefault;
		stream->Read(itemCountIsDefault);
		if (itemCountIsDefault != 0) stream->Read(itemCount);

		bool itemsTotalIsDefault;
		stream->Read(itemsTotalIsDefault);
		if (itemsTotalIsDefault != 0) stream->Read(itemsTotal);

		stream->Read(newObjID);
		stream->Read(ni3FlyingLootPosit);
		stream->Read(showFlyingLoot);
		stream->Read(slotID);

		return true;
	}

	bool bBound;
	bool bIsBOE;
	bool bIsBOP;
	int eLootTypeSource;
	LwoNameValue extraInfo;
	LOT iObjTemplate;
	LWOOBJID iSubkey;
	int invType;
	unsigned int itemCount;
	unsigned int itemsTotal;
	LWOOBJID newObjID;
	NiPoint3 ni3FlyingLootPosit;
	bool showFlyingLoot;
	int slotID;
};*/

void lwoWorldPackets::handleGameMessage(RakPeerInterface * rakServer, Packet * packet, lwoUser * user) {
	RakNet::BitStream inStream(packet->data, packet->length, false);
	lwoPacketUtils::savePacket("handleGameMessage.bin", (char*)inStream.GetData(), inStream.GetNumberOfBytesUsed());

	unsigned long long header = inStream.Read(header); //Skips ahead 8 bytes, SetReadOffset doesn't work for some reason.
	unsigned long long objectID;
	unsigned short messageID;

	//inStream.Read(objectID);
	inStream.Read(messageID);
	std::cout << " sent us GM: " << messageID << std::endl;

	RakNet::BitStream bitStream;
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_GAME_MSG, &bitStream);

	switch (messageID) {
	case GAME_MSG_INVENTORY_SYNCH: {
		for (int i{}; i < 999; i++) {
			bitStream.Write((unsigned short)227);
			//std::cout << "Sending MSG " << 4 + i << std::endl;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			bool showFlyingLoot = true;
			int eLootTypeSource = 0;
			int iObjTemplate = 1727; //LOT
			unsigned long long iSubKey = 0;
			int slotID = 0;
			unsigned long long newObjID = 1;
			int invType = 0;
			unsigned int itemCount = 0;
			unsigned int itemsTotal = 1;
			bool bIsBOE = false;
			bool bIsBOP = false;
			bool bBound = false;
			unsigned int extraInfoLength = 0;
			wstring wsExtraInfo(L"");

			bitStream.Write(bBound);
			bitStream.Write(bIsBOE);
			bitStream.Write(bIsBOP);
			bitStream.Write(false);
			bitStream.Write(extraInfoLength);
			bitStream.Write(iObjTemplate);
			bitStream.Write(false); //iSubKey
			bitStream.Write(false); //InventoryType
			bitStream.Write(false); //ItemCount
			bitStream.Write(false); //itemTotal
			bitStream.Write(newObjID);
			bitStream.Write(x);
			bitStream.Write(y);
			bitStream.Write(z);
			bitStream.Write(showFlyingLoot);
			bitStream.Write(slotID);
			//rakServer->Send(&bitStream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}
		break;
	}
	}
} //handleGameMessage

void ConstructObject(RakPeerInterface* rakServer, Packet* packet, unsigned long long objectID, std::wstring name, unsigned long LOT) {
	//WARNING, DO NOT USE! This is a temporary function Matt made to test out world loading. 
	RakNet::BitStream* stream = new RakNet::BitStream();

	stream->Write((unsigned char)0x24);
	stream->Write(true);
	stream->Write((short)0);

	stream->Write(objectID);
	stream->Write((long)LOT);

	stream->Write((char)name.size());
	lwoPacketUtils::writeWStringToPacket(stream, name);

	for (int i = 0; i < 50; i++) {
		stream->Write((short)0);
	}

	rakServer->Send(stream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
}

void SendCharData(RakPeerInterface* rakServer, Packet* packet, unsigned long long objectID, std::wstring playerName) {
	RakNet::BitStream* stream = new RakNet::BitStream();
	lwoPacketUtils::createPacketHeader(ID_USER_PACKET_ENUM, CONN_CLIENT, MSG_CLIENT_CREATE_CHARACTER, stream);

	RakNet::BitStream* ldfData = new RakNet::BitStream();
	LDF* ldf = new LDF();

	ldf->SetBitStream(ldfData);

	ldf->WriteS64(L"accountID", 1);
	ldf->WriteOBJID(L"objid", objectID);
	ldf->WriteS32(L"template", 1);
	ldf->WriteBool(L"editor_enabled", true);
	ldf->WriteS32(L"editor_level", 9);
	ldf->WriteS32(L"gmlevel", 9);
	ldf->WriteWString(L"name", playerName);
	ldf->WriteFloat(L"position.x", -629.0f);
	ldf->WriteFloat(L"position.y", 613.4f);
	ldf->WriteFloat(L"position.z", -30.0f);

	stream->Write((unsigned long)((ldfData->GetNumberOfBytesUsed() + 8 + 4 + 4)));
	stream->Write((unsigned char)0);
	stream->Write(ldf->GetKeyCount());
	stream->Write(ldfData);

	lwoPacketUtils::savePacket("SendCharData.bin", (char*)stream->GetData(), stream->GetNumberOfBytesUsed());
	rakServer->Send(stream, SYSTEM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
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