
#include <headers.hpp>
#include "user_model.hpp"
#include "server_model.hpp"

using namespace web;
using namespace http;
//using namespace utility::


void ServerModel::supportMethods() {
	_listener.support(methods::GET, std::bind(&ServerModel::handleGet, this, std::placeholders::_1));
	_listener.support(methods::POST, std::bind(&ServerModel::handlePost, this, std::placeholders::_1));
}

void ServerModel::handlePost(http_request message) {
	std::cout << "Recieved POST request from " << utility::conversions::to_utf8string(message.remote_address()) << std::endl;
	auto path = uri::split_path(uri::decode(message.relative_uri().path()));

	if (!path.empty())
	{
		if (!path.empty() && path[0] == L"api" && path[1] == L"register")
		{			
			message.extract_json().then([=](pplx::task<json::value> requestTask)
			{
				try
				{
					auto response = json::value::object();
					string hostname = utility::conversions::to_utf8string(requestTask.get().at(L"hostname").as_string());

					if (!_umanager.isUserRegistered(hostname))
					{
						// create JWT here .. JSON web token 
						auto header = json::value::object();
						header[L"type"] = json::value::string(L"JWT");
						header[L"alg"] = json::value::string(L"HS256");

						auto payload = json::value::object();
						payload[L"hostid"] = json::value::string(requestTask.get().at(L"hostname").as_string());

						//base64 encoding 					
						char* encoded_header = base64(header);
						char* payload_header = base64(payload);

						string data = std::string(encoded_header) + "." + std::string(payload_header);

						std::unique_ptr<uint8_t[]> signature(new uint8_t[EVP_MD_size(EVP_sha256())]);

						if (sign(reinterpret_cast<const uint8_t *>(data.c_str()), data.size(), signature.get(), "MySecretKey" + hostname))
						{
							throw std::logic_error("unable to sign header");
						}

						std::string signe(reinterpret_cast<char *>(signature.get()), EVP_MD_size(EVP_sha256()));
						const char* input = signe.c_str();
						int length = signe.length();

						BIO *bmem, *b64;
						BUF_MEM *bptr;

						b64 = BIO_new(BIO_f_base64());
						bmem = BIO_new(BIO_s_mem());
						b64 = BIO_push(b64, bmem);
						BIO_write(b64, input, length);
						BIO_flush(b64);
						BIO_get_mem_ptr(b64, &bptr);

						char *buff = (char *)malloc(bptr->length);
						memcpy(buff, bptr->data, bptr->length - 1);
						buff[bptr->length - 1] = 0;

						BIO_free_all(b64);

						string access_token = data + "." + std::string(buff);
						cout << "access_token is" << access_token << endl;

						UserInformation userInfo
						{
							hostname, access_token
						};

						string res = _umanager.signUp(userInfo);

						string client_ip = utility::conversions::to_utf8string(message.remote_address());
						saveAuthorizedClients(client_ip);

						response[L"access_token"] = json::value::string(utility::conversions::to_string_t(access_token));
						response[L"response"] = json::value::string(utility::conversions::to_string_t(res) + L"Access_token dispatched.");
						response[L"status"] = json::value::number(100);// 100 - access token created and dispatched.
						message.reply(status_codes::OK, response);

					}
					else
					{
						response[L"response"] = json::value::string(L"Registered User");
						response[L"access_token"] = json::value::string(utility::conversions::to_string_t(_umanager.getAccessToken(hostname)));
						response[L"status"] = json::value::number(200) ; // 200 means Registered user
						message.reply(status_codes::OK, response);
					}
				}
					catch (UserManagerException & e)
					{
						message.reply(status_codes::BadRequest, e.what());
					}
					catch (json::json_exception & e)
					{
						message.reply(status_codes::BadRequest);
					}
			});
		}
		else if ((path.at(0) == L"api" && path.at(1) == L"updateStatus"))
		{
			if (isAuthorized(message)) 
			{
				cout << "Authorized" << endl;
				string client = utility::conversions::to_utf8string(message.remote_address());
				
				message.extract_json().then([=](pplx::task<json::value> requestTask)
				{
					try
					{
						ucout << "Status Recieved " << requestTask.get() << endl;
						saveStatus(requestTask.get());
						auto response = json::value::object();
						response[L"message"] = json::value::string(L"Data Recieved");
						message.reply(status_codes::OK, response);
					}
					catch (web::http::http_exception &e)
					{
						message.reply(status_codes::SeeOther, e.what());
					}
					catch (std::exception &e)
					{
						message.reply(-1, e.what());

					}
				});
			}
			else
			{
				message.reply(status_codes::Unauthorized, "User is not authorized");
			}
		}
		else
		{
			message.reply(status_codes::BadRequest);
		}
	}
	else {
		message.reply(status_codes::BadRequest);
	}
}

void ServerModel::handleGet(http_request message)
{
	cout << "Recieved GET" << endl;
	auto path = uri::split_path(uri::decode(message.relative_uri().path()));
	if (!path.empty())
	{

		if (path.at(0) == L"api" && path.at(1) == L"test") {
			auto response = json::value::object();
			response[L"version"] = json::value::string(L"0.1.1");
			response[L"message"] = json::value::string(L"Data Recieved.");
			message.reply(status_codes::OK, response);
		}
		else if (path.at(0) == L"api" && path.at(1) == L"collectStatus") 
		{			
			json::value res;
			//res[L"pstatus"] = json::value::array();
			int i = 0;
			for (map<string, tuple<int, string, string>> ::iterator iter = status.begin(); iter != status.end(); iter++)
			{
				json::value ps;
				ps[L"Hostname"] = json::value::string(utility::conversions::to_string_t(iter->first));
				ps[L"Status"] = json::value::number(std::get<0>(iter->second));
				ps[L"Message"] = json::value::string(utility::conversions::to_string_t(std::get<1>(iter->second)));
				ps[L"Site"] = json::value::string(utility::conversions::to_string_t(std::get<2>(iter->second)));

				json::value s;
				res[L"status"][i] = ps;
				i++;

			}
			message.reply(status_codes::OK, res);
		}
		else {
			auto response = json::value::object();
			response[L"version"] = json::value::string(L"0.1.1");
			response[L"message"] = json::value::string(L"DefauData Recieved.");
			message.reply(status_codes::OK, response);
		}
	}
	else {
		message.reply(status_codes::NotFound);
	}
}

bool ServerModel::isAuthorized(http_request message)
{
	auto headers = message.headers();
	if (message.headers().find(L"Authorization") == headers.end())
	{
		throw std::exception();
	}
	auto authHeader = headers[L"Authorization"];
	auto credsPos = authHeader.find(L"Bearer");
	if (credsPos == std::string::npos)
	{
		throw std::exception();
	}

	auto token = authHeader.substr(credsPos + std::string("Bearer").length() + 1);
	if (token.empty())
	{
		throw std::exception();
	}

	stringstream token_str(utility::conversions::to_utf8string(token));
	string intermediate;
	vector<string> tokens;
	while (getline(token_str, intermediate, '.'))
	{
		tokens.push_back(intermediate);
	}
	auto bytes = utility::conversions::from_base64(utility::conversions::to_string_t(tokens.at(1)));
	std::string payload(bytes.begin(), bytes.end());
	
	Json::Reader reader;
	Json::Value root;
	string hostname;
	// Parse JSON and print errors if needed
	if (!reader.parse(payload, root)) {
		std::cout << reader.getFormattedErrorMessages();
		exit(1);
	}
	else {
		std::cout << "hostid hos? " << root.isMember("hostid") << std::endl;
		hostname = root["hostid"].asString();
	}

	UserInformation userInfo;
	userInfo.access_token = utility::conversions::to_utf8string(token);
	userInfo.hostname = hostname;

	if (_umanager.signOn(userInfo))
	{
		return true;
	}
	else {
		return false;
	}
}

char * ServerModel::base64(web::json::value val)
{
	string s = utility::conversions::to_utf8string(val.serialize());
	const char* input = s.c_str();
	int length = s.length();

	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length - 1);
	buff[bptr->length - 1] = 0;

	BIO_free_all(b64);

	return buff;
}
bool ServerModel::sign(const uint8_t *header, size_t num_header, uint8_t *signature, string key_) const
{
	unsigned int result_len = 32;

	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);

	HMAC_Init_ex(&ctx, key_.c_str(), key_.size(), EVP_sha256(), NULL);
	HMAC_Update(&ctx, header, num_header);
	HMAC_Final(&ctx, signature, &result_len);

	return false;
}

