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
	Tcout << T("【信息】评测器正在启动……") << endl;

	if (!Judge::LoadCommands())
	{
		Tcout << T("【错误】编程语言配置加载失败！") << endl;
		return;
	}
	Tcout << T("【信息】编程语言配置加载完毕。") << endl;

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

		if(cmd == T("help") || cmd == T("帮助"))
		{
			
		}
		else if (cmd == T("exit") || cmd == T("退出"))
		{
			ctn = false;
		}
		/* TODO: 本地测试
		else if (cmd == T("")) 
		{

		}*/
		else
		{
			Tcout << T("抱歉，无法识别此指令。") << endl;
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
			Tcout << T("【错误】抱歉，无法连接上服务器。") << endl;
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
				Tcout << T("【错误】评测过程中出现了错误（请参考上面的信息）。") << endl;
				continue;
			}
		}
		catch (json::json_exception e)
		{
			Tcout << T("【错误】获取的json不完整或格式有误。") << endl;
			continue;
		}

		Tcout << T("【信息】成功完成对") << json[T("sid")].as_string() << T("号提交的测试。") << endl;
	}	
}