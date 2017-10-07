#pragma once

#include <unordered_map>
#include <memory>
#include "lwoUser.h"
#include "RakNetworkFactory.h"

using namespace std;

class lwoUserPool {
public:
	lwoUserPool();
	~lwoUserPool();

	void Insert(const SystemAddress& address, shared_ptr<lwoUser>);
	void Remove(const SystemAddress& address);
	lwoUser* Find(const SystemAddress& address);

	unordered_map<unsigned int, shared_ptr<lwoUser>> Users;
};