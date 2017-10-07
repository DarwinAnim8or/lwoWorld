#pragma once

#include <string>
#include "RakPeerInterface.h"

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

	int numOfChars() {
		return m_numOfChars;
	}

	void setNumOfChars(int newNumOfChars) {
		m_numOfChars = newNumOfChars;
	}

private:
	unsigned __int64 m_userID;
	std::string m_username;
	SystemAddress m_address;
	int m_numOfChars = 0;
};

