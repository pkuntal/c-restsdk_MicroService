#pragma once
#include <headers.hpp>

typedef struct 
{
	std::string hostname;
	std::string access_token;

} UserInformation;

class UserManagerException : public std::exception
{
	std::string mes;
public:
	UserManagerException(const std::string & message) : mes(message) { }
	const char * what() const throw() { return mes.c_str(); }
};

class UserManager 
{
public:
	std::string signUp(const UserInformation &UserInfo) throw(UserManagerException);
	bool signOn(UserInformation UserInfo);
	bool isUserRegistered(std::string hostname);
	std::string getAccessToken(std::string hostname);
private:
	std::map<std::string, UserInformation> _user;
	std::mutex _userDBMutex;
};

