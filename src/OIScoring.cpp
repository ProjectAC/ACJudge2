#include "../include/OIScoring.h"
#include "../include/platform.h"
#include "../include/definitions.h"
#include "../include/Sandbox.h"
#include "../include/Judge.h"
#include <vector>

using namespace ACJudge2;
using namespace web;
using namespace std;

web::json::value OIScoring::JudgeSinglePoint(ID taskId, web::json::value info, Tstring userLang, Tstring spjLang)
{
	Time time;
	Memory memory;
	Tstring inId, outId;

	try
	{
		time = info[T("time")].as_number().to_uint32();
		memory = info[T("memory")].as_number().to_uint32();
		inId = info[T("input")].as_string();
		outId = info[T("output")].as_string();
	}
	catch (json::json_exception e)
	{
		return fail;
	}

	Sandbox::Run(Judge::GetRunCommand(userLang), time, memory, true, Judge::GetInputFile(taskId, inId), Judge::GetOutputFile(), Judge::GetErrorFile());
}

web::json::value OIScoring::Score(web::json::value task, Submission submission)
{
	try
	{
		Tstring spj = task.has_field(T("spj")) ? task[T("spj")].as_string() : T("");

		vector<json::value> res;
		auto &data = task[T("data")].as_array();

		for (auto &d : data)
			res.push_back(JudgeSinglePoint(task[T("id")].as_string(), d, submission.language, spj));
		
		return json::value::array(res);
	}
	catch (json::json_exception e)
	{
		return fail;
	}
}