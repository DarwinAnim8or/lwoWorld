#include "Player.h"
#include "BitStream.h"
#include "StringTable.h"
#include "ReplicaManager.h"
#include <stdio.h>

extern ReplicaManager replicaManager;

Player::Player() : position{ Vector3(0.0f, 0.0f, 0.0f) }, rotation{ Quaternion(0.0f, 0.0f, 0.0f, 0.0f) }, velocity{ Vector3(0.0f, 0.0f, 0.0f) }, angularVelocity{ Vector3(0.0f, 0.0f, 0.0f) }
{
	// Setup my object

	// Objects are only processed by the system after you tell the manager to replicate them.
	// Here I do it in the constructor, but in a real game you would probably do it after the object is done loading
	// This does NOT call serialize automatically - it only sends a call to create the remote object.
	replicaManager.Construct(this, false, UNASSIGNED_SYSTEM_ADDRESS, true);

	// Calling
	// replicaManager.SetScope(this, true, UNASSIGNED_SYSTEM_ADDRESS)
	// here would work for existing players, but not for future players.  So we call it in SendConstruction instead, which is called once for all players

		// For security, as a server disable all receives except REPLICA_RECEIVE_SERIALIZE
		// I could do this manually by putting if (isServer) return; at the top of all my receive functions too.
		replicaManager.DisableReplicaInterfaces(this, REPLICA_RECEIVE_DESTRUCTION | REPLICA_RECEIVE_SCOPE_CHANGE );
}
Player::~Player()
{
	printf("Inside ~Player\n");

	// Dereplicate the object before it is destroyed, or you will crash the next time the ReplicaManager updates
	replicaManager.Destruct(this, UNASSIGNED_SYSTEM_ADDRESS, true);
	replicaManager.DereferencePointer(this);
}
ReplicaReturnResult Player::SendConstruction( RakNetTime currentTime, SystemAddress systemAddress, unsigned int &flags, RakNet::BitStream *outBitStream, bool *includeTimestamp )
{
	// This string was pre-registered in main with RakNet::StringTable::Instance()->AddString so we can send it with the string table and save bandwidth

	// This means that this object starts in scope this player.  In a real game, you would only start in scope if an object
	// were immediately visible to that player.  This only really applies to the server in a client/server game, since clients wouldn't hide their
	// own object updates from the server.
	// We could have left this line out by calling ReplicaManager::SetDefaultScope(true); in main()
	replicaManager.SetScope(this, true, systemAddress, false);

	printf("Sending player to %i:%i\n", systemAddress.binaryAddress, systemAddress.port);

	outBitStream->Write(true);
	outBitStream->Write((short)0);
	outBitStream->Write(objectID);
	outBitStream->Write((long)templateID);

	outBitStream->Write((char)objectName.size());
	lwoPacketUtils::writeWStringToPacket(outBitStream, objectName);

	//From Matt, somehow works? 
	/*for (int i = 0; i < 50; i++) {
		outBitStream->Write((short)0);
	}*/

	outBitStream->Write(true);
	outBitStream->Write((short)0);
	outBitStream->Write(objectID);
	outBitStream->Write(templateID);
	outBitStream->Write(unsigned char(objectName.length()));
	lwoPacketUtils::writeWStringToPacket(outBitStream, objectName);
	outBitStream->Write(unsigned int(0));
	outBitStream->Write(false); //extraData?
	outBitStream->Write(false); //trigger_id (is trigger or not)
	outBitStream->Write(false); //has spawner ID
	outBitStream->Write(false); //has spawner node ID
	outBitStream->Write(false); //?? float
	outBitStream->Write(false); //objectWorldState
	outBitStream->Write(true);
	outBitStream->Write(gmLevel);

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

	outBitStream->Write(false);
	outBitStream->Write(false);

	outBitStream->Write(false);
	outBitStream->Write(false);
	outBitStream->Write(false);

	outBitStream->Write(true); //including ControllablePhysics
	outBitStream->Write(position.x);
	outBitStream->Write(position.y);
	outBitStream->Write(position.z);

	outBitStream->Write(rotation.x);
	outBitStream->Write(rotation.y);
	outBitStream->Write(rotation.z);
	outBitStream->Write(rotation.w);

	outBitStream->Write(bIsOnGround);
	outBitStream->Write(false);
	outBitStream->Write(false); //noaccel
	outBitStream->Write(false); //novel
	outBitStream->Write(false);


	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::SendDestruction(RakNet::BitStream *outBitStream, SystemAddress systemAddress, bool *includeTimestamp)
{
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::ReceiveDestruction(RakNet::BitStream *inBitStream, SystemAddress systemAddress, RakNetTime timestamp)
{	
	delete this;

	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, SystemAddress systemAddress, bool *includeTimestamp)
{
	if (inScope)
		printf("Sending scope change to true in Player\n");
	else
		printf("Sending scope change to false in Player\n");

	// Up to you to write this.  If you write nothing, the system will treat that as if you wanted to cancel the scope change
	outBitStream->Write(inScope);
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::ReceiveScopeChange(RakNet::BitStream *inBitStream, SystemAddress systemAddress, RakNetTime timestamp)
{
	bool inScope;
	inBitStream->Read(inScope);
	if (inScope)
		printf("Received message that scope is now true in Player\n");
	else
		printf("Received message that scope is now false in Player\n");
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, SystemAddress systemAddress, unsigned int &flags)
{
	if (lastSendTime==0)
		printf("First call to Player::Serialize for %i:%i\n", systemAddress.binaryAddress, systemAddress.port);

	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, SystemAddress systemAddress )
{
	if (lastDeserializeTime==0)
		printf("First call to Player::Deserialize\n");
	else
		printf("Got Player::Deserialize\n");

	return REPLICA_PROCESSING_DONE;
}