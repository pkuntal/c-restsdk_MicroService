
#pragma once

#include <string>
#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
//#include <pplx/pplx.h>

using namespace web;
using namespace http;
using namespace http::experimental::listener;
using namespace std;

class BaseModel 
{	
    protected:
        http_listener _listener; 

    public:
        BaseModel();
        ~BaseModel();

        void setURI(const std::string &value);
        std::string serverpath() const;
        pplx::task<void> accept();
        pplx::task<void> shutdown();

		virtual void supportMethods() {};

};
