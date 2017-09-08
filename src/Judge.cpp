#include <cstdio>
#include "../include/Judge.h"
#include "../include/Sandbox.h"
#include "../include/OIScoring.h"

using namespace std;
using namespace ACJudge2;
using namespace web;

std::map<Tstring, Language> Judge::languages;

bool Judge::LoadCommands()
{
	Tifstream file(T("../lang.cfg"));
	if (!file)
		return false;

	try
	{
		json::value cfg = json::value::parse(file);
		for (auto &d : cfg.as_array())
			languages[d[T("name")].as_string()] = { d[T("name")].as_string(), d[T("compile")].as_string(), d[T("run")].as_string() };
	}
	catch (json::json_exception e)
	{
		return false;
	}
	return true;
}

Tstring Judge::GetInputFile(ID taskId, ID inId)
{
	return GetTaskPath(taskId) + inId;
}

Tstring Judge::GetOutputFile()
{
	return GetBoxPath() + T("out");
}

Tstring Judge::GetAnswerFile(ID taskId, ID outId)
{
	return GetTaskPath(taskId) + outId;
}

Tstring Judge::GetErrorFile()
{
	return GetBoxPath() + T("err");
}

Tstring Judge::GetRunCommand(Tstring lang)
{
	if (languages.find(lang) == languages.end())
		throw T("Undefined language!");
	Language language = languages[lang];

	Tchar tmp[LENGTH];
	Tsprintf(tmp, language.run.c_str(), (GetBoxPath() + T("bar")).c_str());
	return tmp;
}

Tstring Judge::GetCompareCommand(ID taskId, ID inId, ID outId, Tstring lang)
{
	if (lang == T(""))
		return T("");  // Default comparer

	if (languages.find(lang) == languages.end())
		throw T("Undefined language!");
	Language language = languages[lang];

	Tchar tmp[LENGTH];
	Tstring in = GetInputFile(taskId, inId);
	Tstring out = GetOutputFile();
	Tstring ans = GetAnswerFile(taskId, outId);
	Tsprintf(tmp, (language.run + T(" ") + in + T(" ") + out + T(" ") + ans).c_str(), (GetBoxPath() + T("spj")).c_str());
	return tmp;
}

Tstring Judge::GetPrepareCommand(Tstring lang)
{
	if (languages.find(lang) == languages.end())
		throw T("Undefined language!");
	Language language = languages[lang];

	Tchar tmp[LENGTH];
	Tsprintf(tmp, language.compile.c_str(), (GetBoxPath() + T("bar")).c_str(), (GetBoxPath() + T("bar")).c_str());
	return tmp;
}

Tstring Judge::GetPrepareSpjCommand(ID taskId, Tstring lang)
{
	if (languages.find(lang) == languages.end())
		throw T("Undefined language!");
	Language language = languages[lang];

	Tchar tmp[LENGTH];
	Tsprintf(tmp, language.compile.c_str(), (GetTaskPath(taskId) + T("spj")).c_str(), (GetBoxPath() + T("spj")).c_str());

	return tmp;
}

Tstring Judge::GetTaskPath(ID taskId)
{
	return T("../tasks/") + taskId + T("/");
}

Tstring Judge::GetBoxPath()
{
	return T("../foo/");
}

json::value Judge::GetTaskConfig(ID taskId)
{
	try
	{
		Tifstream config;
		json::value cfg;

		config.open(GetTaskPath(taskId) + T("config"));
		cfg = json::value::parse(config);

		Tstring type = cfg[T("type")].as_string();
		if (type == T("OI"))
		{
			auto &data = cfg[T("data")].as_array();
			int score = 0;
			for (auto &d : data)
				score += d[T("score")].as_integer();
			if (score != 100)
				return fail;
		}

		return cfg;
	}
	catch (json::json_exception e)
	{
		return fail;
	}
}

bool Judge::PrepareSpj(Tstring taskId, Tstring lang)
{
	try
	{
		Sandbox().Run(GetPrepareSpjCommand(taskId, lang), 5000, INFINITE, false);
	}
	catch (Tstring s)
	{
		return false;
	}

	return true;
}

bool Judge::PrepareUserCode(Submission s)
{
	try
	{
		Sandbox().Run(GetPrepareCommand(s.language), 5000, INFINITE, false);
	}
	catch (exception e)
	{
		return false;
	}

	return true;
}

json::value Judge::Prepare(Submission s)
{
	try
	{
		auto &ret = GetTaskConfig(s.taskId);
		if (ret.has_field(T("fail")))
			return fail;
		if (ret.has_field(T("spj")))
			if (PrepareSpj(s.taskId, ret[T("spj")].as_string()) == false)
				return fail;
		if (PrepareUserCode(s) == false)
			return fail;
		return ret;
	}
	catch (json::json_exception e)
	{
		return fail;
	}
}

web::json::value Judge::Run(Submission submission)
{
	auto task = Prepare(submission);
	if (task.has_field(T("fail")))
		return fail;

	Tstring type = task[T("type")].as_string();

	if (type == T("OI"))
	{
		return OIScoring(*this).Score(task, submission);
	}
	else if (type == T("ACM"))
	{
		return OIScoring(*this).Score(task, submission);
	}
	else if (type == T("CF"))
	{
		return OIScoring(*this).Score(task, submission);
	}
	else
	{
		return fail;
	}
}