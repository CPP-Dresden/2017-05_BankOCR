#include "OCR.h"

#include <gtest/gtest.h>

template<class E, class... A>
constexpr auto mk_array(E e, A... a) {
	std::array<std::remove_const_t<E>, 1 + sizeof...(A)> result = { {e, a...} };
	return result;
}

template<class E, size_t S>
constexpr auto str_array(E(&v)[S]) {
	std::array<std::remove_const_t<E>, S-1> result;
	for (auto i = 0u; i < S-1; ++i) result[i] = v[i];
	return result;
}

TEST(OCRTest, parseOneToNine) {
	auto input = BankOCR::ScannedFile{
		{
			str_array("    _  _     _  _  _  _  _ "),
			str_array("  | _| _||_||_ |_   ||_||_|"),
			str_array("  ||_  _|  | _||_|  ||_| _|"),
			str_array("                           ")
		}
	};
	auto ret = BankOCR::parse(input);
	using AD = BankOCR::AccountDigit;
	auto expected = mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine);
	EXPECT_EQ(expected, ret.front());
}

TEST(OCRTest, parseInvalidAndZero) {
	auto input = BankOCR::ScannedFile{
		{
			str_array("    _  _     _  _  _  _  _ "),
			str_array(" _| _| _||_||_ |_   ||_|| |"),
			str_array("  ||_  _|  | _||_|  ||_||_|"),
			str_array("                           ")
		}
	};
	auto ret = BankOCR::parse(input);
	using AD = BankOCR::AccountDigit;
	auto expected = mk_array(AD::Invalid, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Zero);
	EXPECT_EQ(expected, ret.front());
}

TEST(OCRTest, checkValid) {
	using AD = BankOCR::AccountDigit;
	auto inputs = BankOCR::AccountNumbers{ mk_array(AD::Three, AD::Four, AD::Five, AD::Eight, AD::Eight, AD::Two, AD::Eight, AD::Six, AD::Five) };
	auto ret = BankOCR::check(inputs);
	EXPECT_EQ(BankOCR::CheckState::Passed, ret.front().state);
}

TEST(OCRTest, checkWrongChecksum) {
	using AD = BankOCR::AccountDigit;
	auto inputs = BankOCR::AccountNumbers{ mk_array(AD::Six, AD::Six, AD::Four, AD::Three, AD::Seven, AD::One, AD::Four, AD::Nine, AD::Five) };
	auto ret = BankOCR::check(inputs);
	EXPECT_EQ(BankOCR::CheckState::WrongChecksum, ret.front().state);
}

TEST(OCRTest, checkIllelibleCharacters) {
	using AD = BankOCR::AccountDigit;
	auto inputs = BankOCR::AccountNumbers{ mk_array(AD::Six, AD::Six, AD::Four, AD::Three, AD::Seven, AD::One, AD::Four, AD::Invalid, AD::Five) };
	auto ret = BankOCR::check(inputs);
	EXPECT_EQ(BankOCR::CheckState::IllelibleCharacters, ret.front().state);
}

TEST(OCRTest, checkFixAmbigious) {
	using AD = BankOCR::AccountDigit;
	auto inputs = BankOCR::AccountNumbers{ mk_array(AD::Four, AD::Nine, AD::Zero, AD::Zero, AD::Six, AD::Seven, AD::Seven, AD::One, AD::Five) };
	auto ret = BankOCR::checkWithFixes(inputs);
	EXPECT_EQ(BankOCR::CheckState::WrongChecksum, ret.front().state);
	EXPECT_EQ(BankOCR::FixState::Ambiguous, ret.front().fix);
}

TEST(OCRTest, checkFixSuccess) {
	using AD = BankOCR::AccountDigit;
	auto inputs = BankOCR::AccountNumbers{ mk_array(AD::Six, AD::Six, AD::Four, AD::Three, AD::Seven, AD::One, AD::Four, AD::Nine, AD::Five) };
	auto ret = BankOCR::checkWithFixes(inputs);
	EXPECT_EQ(BankOCR::CheckState::WrongChecksum, ret.front().state);
	EXPECT_EQ(BankOCR::FixState::Success, ret.front().fix);
}

TEST(OCRTest, checkResultsString) {
	using AD = BankOCR::AccountDigit;
	using CheckState = BankOCR::CheckState;
	using FixState = BankOCR::FixState;
	auto inputs = BankOCR::CheckResults{
		{ CheckState::Passed, FixState::None, mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine) },
		{ CheckState::IllelibleCharacters, FixState::None, mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine) },
		{ CheckState::WrongChecksum, FixState::Success, mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine) },
		{ CheckState::WrongChecksum, FixState::Ambiguous, mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine) },
		{ CheckState::WrongChecksum, FixState::Failed, mk_array(AD::One, AD::Two, AD::Three, AD::Four, AD::Five, AD::Six, AD::Seven, AD::Eight, AD::Nine) }
	};
	auto ret = BankOCR::toString(inputs);
	EXPECT_EQ(R"(123456789
123456789 ILL
123456789 FIX
123456789 AMB
123456789 ERR)", ret);
}
