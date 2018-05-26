#pragma once

#include "../Global.h"

class UserManager : public Component ,public std::enable_shared_from_this<UserManager>
{
public:
	//Post �Լ�
	std::shared_ptr<class User> CreateUser(int key);
	bool InsertUser(std::shared_ptr<class User> user);
	void SendAllUsers(const PS & symbol, std::shared_ptr<flatbuffers::FlatBufferBuilder> fbb);
	
	bool RemoveUser(int key);

private:
	std::unordered_map<int, std::shared_ptr<class User>> users;
	
	//������
public:
	UserManager();
	~UserManager();
};