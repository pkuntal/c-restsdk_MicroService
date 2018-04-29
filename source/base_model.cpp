#undef U
#include "include\base_model.hpp"


BaseModel::BaseModel() {}

BaseModel::~BaseModel()
{

}
void BaseModel::setURI(const std::string &value)
{
	uri uri(utility::conversions::to_string_t(value));
	uri_builder uribuilder;

	uribuilder.set_scheme(uri.scheme());
	uribuilder.set_host(L"localhost");
	uribuilder.set_port(uri.port());
	uribuilder.set_path(uri.path());

	_listener = http_listener(uribuilder.to_uri());

}

std::string BaseModel::serverpath() const
{
	return utility::conversions::to_utf8string(_listener.uri().to_string());
}

pplx::task<void> BaseModel::accept()
{
	supportMethods();
	return _listener.open();
}

pplx::task<void> BaseModel::shutdown()
{
	return _listener.close();
}
