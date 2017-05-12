#include "OCR.h"
#include <cassert>


std::vector<int> OCR::read(const std::string& input)
{
	assert(4 * 27 == input.size());
	std::vector<int> result;
	for (int i = 0; i < 9; ++i) {
		std::string sign = getPos(input, i);
		result.push_back( getNumber(sign) );
	}
	return result;
}

namespace {
	std::string charArray[10] = {
		" _ "
		"| |"
		"|_|",
		"   "
		"  |"
		"  |",
		" _ "
		" _|"
		"|_ ",
		" _ "
		" _|"
		" _|",
		"   "
		"|_|"
		"  |",
		" _ "
		"|_ "
		" _|",
		" _ "
		"|_ "
		"|_|",
		" _ "
		"  |"
		"  |",
		" _ "
		"|_|"
		"|_|",
		" _ "
		"|_|"
		" _|" };
}

std::string OCR::getPos(const std::string& input, int i) {
	return 
		input.substr(i * 3, 3)
		+ input.substr(27 + i* 3, 3) 
		+ input.substr(2*27 + i * 3, 3);
}

int OCR::getNumber(const std::string& str) {
	auto i = 0;
	for (auto ch : charArray) {
		if (ch == str) return i;
		i++;
	}
	return -1;
}


int getCheckSum(const std::vector<int>& in) {
	auto ret = 0, p = 0;
	for (auto v : in) ret += (9 - p++) * v;
	return ret % 11;
}

std::string getCheck(const std::vector<int>& in)
{
	std::string ret = "";
	for (int i : in) ret += i < 0 ? '?' : '0'+i;

	for (auto n : in) {
		if (n < 0) return ret + " ILL";
	}
	return ret + (getCheckSum(in) ? " ERR" : "");
}

std::vector<std::vector<int>> replacements = { { 8 },{ 7 },{},{ 9 },{},{ 6,9 },{},{ 1 },{ 0,6,9 },{ 3,5,8 } };

std::vector<std::vector<int>> checkReplace(std::vector<int> in)
{
	std::vector<std::vector<int>> results;
	for (auto &i : in) {
		int org = i;
		for (auto r : replacements[i])
		{
			i = r;
			if (getCheckSum(in) == 0 ) {
				results.push_back( in );
				if (results.size() > 1) {
					return results;
				}
			};
		}
		i = org;
	}
	return results;
}

std::string getCheckPlus(const std::vector<int>& in)
{
	std::string ret = "";
	for (int i : in) ret += i < 0 ? '?' : '0' + i;

	for (auto n : in) {
		if (n < 0) return ret + " ILL";
	}
	if (0 == getCheckSum(in)) return ret;
	auto v = checkReplace(in);

	if (v.empty()) return ret + " ERR";
	if (v.size() != 1) return ret + " AMB";

	ret = "";
	for (int i : v[0]) ret += '0' + i;
	return ret + " FIX";
}

