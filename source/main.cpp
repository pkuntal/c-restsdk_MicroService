

#undef BOOST_DYN_LINK
#include <iostream>
#include <Windows.h>
#include <wincrypt.h>
#include <signal.h>
#include <Cryptuiapi.h>
#include "server_model.hpp"


#pragma comment(lib, "cpprest141_2_10")

static std::condition_variable cv;
static std::mutex mute;


using namespace web;

bool ImportCert()
{
	CRYPTUI_WIZ_IMPORT_SRC_INFO importSrc;

	memset(&importSrc, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));

	importSrc.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);

	importSrc.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;

	importSrc.pwszFileName = L"C:\\Users\\pkuntal\\Desktop\\manageability\\ProvisioningServiceCert.pfx";

	importSrc.pwszPassword = L"abc123";

	importSrc.dwFlags = CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED;
	//importSrc.hCertStore = 

	bool certRes = CryptUIWizImport(CRYPTUI_WIZ_NO_UI, NULL, NULL, &importSrc, NULL);

	if (certRes == 0)
	{

		cout << "CryptUIWizImport error" << GetLastError() << endl;

	}
	return certRes;
}


void handleUserInterrupt(int signal) {
	if (signal == SIGINT) {
		std::cout << "Interupt recieved." << endl;
		cv.notify_one();
	}
}

void hookUserInteruppt() {
	signal(SIGINT, handleUserInterrupt);
}



 void waitForUserInterrupt() {
	std::unique_lock<std::mutex> lock{ mute };
	cv.wait(lock);
	std::cout << "Interupt processed." << endl;
	lock.unlock();
}

int main(int argc, const char * argv[]) {
	hookUserInteruppt();
    ServerModel listener;
	listener.setURI("http://localhost:3999/restsdkserver/v1/");
    
    try {
		listener.accept().wait();
        std::cout << "Server started at: " << listener.serverpath() << endl; 
		waitForUserInterrupt();
		listener.shutdown().wait();
    }
    catch(std::exception & e) {
        std::cerr << "Exception..." << endl;
		listener.shutdown().wait();

    }
    catch(...) {
		std::cerr << "Exception..." << endl;
		listener.shutdown().wait();
    }

    return 0;
}
