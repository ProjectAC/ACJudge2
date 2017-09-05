#pragma once

#include "platform.h"
#include <cpprest/http_client.h>

namespace ACJudge2
{
	class Main
	{
	public:

		/* Online Mode

		* server: domain name or ip address of ACOJ6+ web server
		* port: port id of ACOJ6+ web server

		Connects to server and automatically fetch submissions.
		Never stops before you shut it down.
		*/
		Main(Tstring server);

		/* Start The Process

		This function never returns.
		Unless when receiving quit command.
		*/
		void Start();

	private:

		web::http::client::http_client client;
		bool ctn;

		/* Main Procedure

		  Fork a subprocess for running.
		  The main process will wait for commands.
		*/
		static void MainLoop(Main &main);
	};

}