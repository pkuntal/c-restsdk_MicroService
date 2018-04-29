#include <mutex>
#include "user_model.hpp"


std::string UserManager::signUp(const UserInformation & UserInfo) throw(UserManagerException)
{
	std::unique_lock<std::mutex> lock{ _userDBMutex };
	if (_user.find(UserInfo.hostname) == _user.end())
	{
		_user.insert(std::pair<std::string, UserInformation>(UserInfo.hostname, UserInfo));		
	}
	return "User registeration done.";
}

bool UserManager::isUserRegistered(std::string hostname)
{
	if (_user.find(hostname) == _user.end())
	{
		return false;
	}
	else
	{
		return true;
	}

}
std:: string UserManager::getAccessToken(std::string hostname)
{
	return _user.find(hostname)->second.access_token;
}

bool UserManager::signOn(UserInformation userInfo)
{
	if (_user.find(userInfo.hostname) != _user.end())
	{
		auto ui = _user[userInfo.hostname];
		if (ui.access_token == userInfo.access_token)
		{
			return true;
		}
	}
	return false;
}