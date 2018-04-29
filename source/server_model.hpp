#pragma once 

#include "include\base_model.hpp"
#include "user_model.hpp"


class ServerModel : public BaseModel {
public:
    ServerModel() : BaseModel() {}
    ~ServerModel() {}
    void handleGet(http_request message) ;
    void handlePost(http_request message) ;
    void supportMethods() override;

private:
	void saveStatus(web::json::value data)
	{
		std::unique_lock<std::mutex> lock{ statusMutex };
		status[utility::conversions::to_utf8string(data.at(L"hostname").as_string())] = make_tuple(data.at(L"status").as_integer(), utility::conversions::to_utf8string(data.at(L"message").as_string()), "BLR");
	}
	void saveAuthorizedClients(string ip) 
	{
		std::unique_lock<std::mutex> lock{ statusMutex };
		connectedClients.push_back(ip);
	}
	bool isAuthorized(http_request req);
	char* base64(web::json::value val);
	bool sign(const uint8_t * header, size_t num_header, uint8_t * signature, string key_) const;
	map<string, std::tuple<int, string, string>> status;
	std::vector<string> connectedClients;
	std::mutex statusMutex;
	UserManager _umanager;

};