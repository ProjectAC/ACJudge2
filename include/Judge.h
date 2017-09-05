#pragma once

#include <vector>
#include <cpprest/json.h>
#include <map>
#include "definitions.h"
#include "platform.h"

namespace ACJudge2
{
	class Judge
	{
	public:
		web::json::value Run(Submission submission);

		static Tstring GetTaskPath(ID taskId);
		static Tstring GetBoxPath();

		static Tstring GetInputFile(ID taskId, ID inId);
		static Tstring GetOutputFile();
		static Tstring GetAnswerFile(ID taskId, ID outId);
		static Tstring GetErrorFile();

		static Tstring GetRunCommand(Tstring language);
		static Tstring GetCompareCommand(ID taskId, ID inId, ID outId, Tstring language);
		static Tstring GetPrepareCommand(Tstring language);
		static Tstring GetPrepareSpjCommand(ID taskID, Tstring language);

		/* Command Loading

		Load the compile & run command for each language.
		Config locates at /config/languages.cfg
		*/
		static bool LoadCommands();

	private:

		static std::map<Tstring, Language> languages;

		inline web::json::value GetTaskConfig(ID taskId);
		inline bool PrepareSpj(Tstring taskId, Tstring language);
		inline bool PrepareUserCode(Submission s);

		/* Preparement
		  
		* submission: submission info from server

		  Prepare user program and special judge.
		  And judge whether the task config file is legal
		*/
		inline web::json::value Prepare(Submission submission);

	};
}
