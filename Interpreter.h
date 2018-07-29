#pragma once
#include <string>
#include <vector>
#include "Condition.h"
#include "Attribute.h"
#include "API.h"
#include <iostream>

using namespace std;
class Interpreter{
public:
	Interpreter() :
		ap(nullptr)
	{
		
	}
	virtual ~Interpreter()
	{
		
	}

    API *ap;
    string fileName;
    string split_word(string s, int* index);
    int interpreter(string s);
    
};