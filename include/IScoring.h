#pragma once

#include <cpprest/json.h>
#include "definitions.h"
#include "Judge.h"

namespace ACJudge2
{
	class IScoring
	{
	protected:
		virtual web::json::value JudgeSinglePoint(ID taskId, web::json::value info, Tstring userLang, Tstring spjLang) {}
		Judge &judge;

	public:
		virtual web::json::value Score(web::json::value task, Submission submission) {}
		IScoring(Judge &judge) : judge(judge) {}
	};
}
