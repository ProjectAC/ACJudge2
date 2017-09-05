#pragma once

#include <cpprest/json.h>
#include "platform.h"

#define LENGTH 100

namespace ACJudge2
{
	typedef unsigned short DataID;
	typedef Tstring ID;
	typedef unsigned short Score;

	typedef unsigned int Time;
	typedef unsigned int Memory;

	const web::json::value fail("{fail: -1}");

	enum ResultType
	{
		OK,
		WA,
		PE,
		RTE,
		TLE,
		MLE,
		JIE,
		ERR
	};
	
	struct Result
	{
		ResultType ret;
		Time time;
		Memory memory;
		int val;
		Tstring msg;
	};

	template<class T>
	Tstring ACJudge2::toString(T obj)
	{
		TISStream s(obj);
		Tstring res;
		s >> res;
		return res;
	}

	template<class T>
	T ACJudge2::await(Concurrency::task<T> t)
	{
		t.wait();
		return t.get();
	}

	void sleep(Time ms);
	
	struct Submission
	{
		ID submissionId;
		ID taskId;
		Tstring language;
		Tstring code;
	};

	struct Language
	{
		Tstring name, compile, run;
	};
};