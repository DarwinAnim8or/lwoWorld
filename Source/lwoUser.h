#pragma once

#include <string>
#include "RakPeerInterface.h"
#include "Player.h"

class lwoUser {
public:
	lwoUser(unsigned __int64 userID, const std::string& sUsername, const SystemAddress& address)
		: m_userID(userID), m_username(sUsername), m_address(address) {}

	~lwoUser();

	unsigned __int64 UserID() const {
		return m_userID;
	}

	std::string Username() const {
		return m_username;
	}

	void setUsername(const std::string& username) {
		m_username = username;
	}

	SystemAddress Address() const {
		return m_address;
	}

	int numOfChars() const {
		return m_numOfChars;
	}

	void setNumOfChars(int newNumOfChars) {
		m_numOfChars = newNumOfChars;
	}

	Player* getPlayer() {
		return m_player;
	}

	Player* createPlayer() {
		m_player = new Player();
	}

	void destroyPlayer() {
		delete m_player;
		m_player = nullptr;
	}

private:
	unsigned __int64 m_userID;
	std::string m_username;
	SystemAddress m_address;
	int m_numOfChars = 0;
	Player* m_player;
};