#pragma once

#include "../Global.h"

class UserManager : public Component
{
public:
	//Post �Լ�
	std::shared_ptr<class User> CreateUser(int key);
	
private:
	std::unordered_map<int, std::shared_ptr<class User>> users;

	//������
public:
	UserManager();
	~UserManager();
	

};