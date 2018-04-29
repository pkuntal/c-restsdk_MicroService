# cpprestsdk_tutorial

Build process :

1. Clone the repository
2. Install VS and load PSMicroService.sln
3. Download boost, jsoncpp, openssl using Nuget.
4. Download cpprestsdk and build it statically with given solution
5. Build PSMicroService.sln and generate PSMicroService.exe
6. Run PSMicroService.exe to start listener :
	Server started at http://localhost:3999/restsdkserver/v1/
	
7.It provides follwing endpoints :

	GET	 /test -- testing endoint to check connection
	POST /register -- resgister users, returns JWT
		I/P - hostname details {"hostname" : "temp_hostman"}
		O/P - {"access_token": "eyJhbGciOiJIUzI1NiIsInR5cGUiOiJKV1QifQ==.eyJob3N0aWQiOiJ0ZW1wX2hvc3RtYW4ifQ==.TVnjoKm1rw5W2T9NyEu8uxkPNrtrulBbAE+WlCuQx7I=", "response": "Registered User",  "status": 200}
	POST /updateStatus -- send status to server.
		Authorization Header : Bearer : Bearer{{JWT}}
		I/P - 'status JSON' {"{"hostname":"temp_hostman","message":"data delivered","status" : 100}
	GET	 /collectStatus -- retrives list of collected status.


