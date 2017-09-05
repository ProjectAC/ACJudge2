#include "../include/Main.h"
#include "../include/Judge.h"
#include <cpprest/json.h>

using namespace ACJudge2;
using namespace std;
using namespace web;
using namespace http;

Main::Main(Tstring server) :
	client(server)
{
}

void Main::Start()
{
	Tcout << T("����Ϣ��������������������") << endl;

	if (!Judge::LoadCommands())
	{
		Tcout << T("�����󡿱���������ü���ʧ�ܣ�") << endl;
		return;
	}
	Tcout << T("����Ϣ������������ü�����ϡ�") << endl;

	ctn = true;
	std::thread subThread(MainLoop, ref(*this));
	subThread.detach();

	Tstring cmd;
	Tistringstream stream;
	while (ctn)
	{
		Tcout << T(">> ") << endl;
		getline(Tcin, cmd);
		stream = Tistringstream(cmd);
		stream >> cmd;

		if(cmd == T("help") || cmd == T("����"))
		{
			
		}
		else if (cmd == T("exit") || cmd == T("�˳�"))
		{
			ctn = false;
		}
		/* TODO: ���ز���
		else if (cmd == T("")) 
		{

		}*/
		else
		{
			Tcout << T("��Ǹ���޷�ʶ���ָ�") << endl;
		}
	}
}

void Main::MainLoop(Main &main)
{
	uri_builder builder;
	Judge judge;

	while (main.ctn)
	{
		sleep(1000);

		builder.set_path(T("/"));
		http_response res;

		auto response = await(main.client.request(http::methods::GET, builder.to_string()));
		if (response.status_code() != 200)
		{
			Tcout << T("�����󡿱�Ǹ���޷������Ϸ�������") << endl;
			continue;
		}

		auto json = await(response.extract_json());
		try
		{
			if (judge.Run({
				json[T("sid")].as_string(),
				json[T("tid")].as_string(),
				json[T("language")].as_string(),
				json[T("code")].as_string()
			}).has_field(T("fail")))
			{
				Tcout << T("��������������г����˴�����ο��������Ϣ����") << endl;
				continue;
			}
		}
		catch (json::json_exception e)
		{
			Tcout << T("�����󡿻�ȡ��json���������ʽ����") << endl;
			continue;
		}

		Tcout << T("����Ϣ���ɹ���ɶ�") << json[T("sid")].as_string() << T("���ύ�Ĳ��ԡ�") << endl;
	}	
}