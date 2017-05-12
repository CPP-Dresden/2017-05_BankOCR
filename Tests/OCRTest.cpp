#include "OCR.h"

#include <gtest/gtest.h>

TEST(OCRTest, readsValidInput) {
	OCR ocr;
	std::string input = 
		"    _  _     _  _  _  _  _ "
		"  | _| _||_||_ |_   ||_||_|"
		"  ||_  _|  | _||_|  ||_| _|"
		"                           ";
	auto ret=ocr.read(input);

	EXPECT_EQ(std::vector<int>({ 1,2,3,4,5,6,7,8,9 }), ret);
}

TEST(OCRTest, checkChecksum) {
	std::vector<int> ret = { 3,4,5,8,8,2,8,6,5 };
	auto chsu = getCheckSum(ret);
	EXPECT_EQ(0, chsu);
}

TEST(OCRTest, checkFailed) {
	EXPECT_EQ("457508000", getCheck({ 4,5,7,5,0, 8,0,0,0 }));
	EXPECT_EQ("664371495 ERR", getCheck({ 6,6,4,3,7, 1,4,9,5 }));
	EXPECT_EQ("86110??36 ILL", getCheck({ 8,6,1,1,0,-1,-1,3,6 }));
}


TEST(OCRTest, checkReplacement) {
	using t = std::vector<std::vector<int>>;
	EXPECT_EQ(t({ { 4, 9, 0, 8, 6, 7, 7, 1, 5 },{ 4, 9, 0, 0, 6, 7, 1, 1, 5 } }), checkReplace({ 4,9,0,0,6,7,7,1,5 }));
}

TEST(OCRTest, checkPlus) {
	EXPECT_EQ("490067715 AMB", getCheckPlus({ 4,9,0,0,6,7,7,1,5 }));
	EXPECT_EQ("123456789", getCheckPlus({ 1,2,3,4,5,6,7,8,9 }));
	EXPECT_EQ("86110??36 ILL", getCheckPlus({ 8,6,1,1,0,-1,-1,3,6 }));
	EXPECT_EQ("664371485 FIX", getCheckPlus({ 6,6,4,3,7, 1,4,9,5 }));
}
