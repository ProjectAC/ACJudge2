#pragma once

#include <cpprest/json.h>
#include "IScoring.h"

namespace ACJudge2
{
	class OIScoring : public IScoring
	{
	protected:
		web::json::value JudgeSinglePoint(ID taskId, web::json::value info, Tstring userLang, Tstring spjLang) override;

	public:
		web::json::value Score(web::json::value task, Submission submission) override;
	};
}