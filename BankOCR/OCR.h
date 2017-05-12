#pragma once
#include <string>
#include <vector>

class OCR
{
public:
	// multiline string => vector of digits
	static std::vector<int> read(const std::string& input);


	static std::string getPos(const std::string& input, int i);
	static int getNumber(const std::string& str);
};

int getCheckSum(const std::vector<int>& in);

std::string getCheck(const std::vector<int>& in);
std::string getCheckPlus(const std::vector<int>& in);

std::vector<std::vector<int>> checkReplace(std::vector<int> in);