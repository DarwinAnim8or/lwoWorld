#include "lwoUserPool.h"
#include <iostream>
#include <memory>

lwoUserPool::lwoUserPool() {
}

void lwoUserPool::Insert(const SystemAddress& address, shared_ptr<lwoUser> user) {
	Users.insert(std::pair<unsigned int, shared_ptr<lwoUser>>(address.port, user));
}

void lwoUserPool::Remove(const SystemAddress& address) {
	Users.erase(address.port);
}

lwoUser* lwoUserPool::Find(const SystemAddress& address) {
	return Users.find(address.port)->second.get();
}

lwoUserPool::~lwoUserPool() {
}