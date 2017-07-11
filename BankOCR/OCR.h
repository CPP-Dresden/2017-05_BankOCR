#pragma once
#include <array>
#include <string>
#include <vector>

namespace BankOCR
{
	using String = std::string;

	using ScannedLine = std::array<char, 27>;
	using ScannedEntry = std::array<ScannedLine, 4>;
	using ScannedFile = std::vector<ScannedEntry>;

	enum class AccountDigit {
		Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Invalid
	};
	using AccountNumber = std::array<AccountDigit, 9>;
	using AccountNumbers = std::vector<AccountNumber>;

	enum class CheckState {
		Passed, WrongChecksum, IllelibleCharacters,
	};
	enum class FixState {
		None, Success, Ambiguous, Failed,
	};
	struct CheckResult {
		CheckState state = CheckState::Passed;
		FixState fix = FixState::None;
		AccountNumber number;
	};
	using CheckResults = std::vector<CheckResult>;


	AccountNumbers parse(ScannedFile);

	CheckResults check(AccountNumbers);
	CheckResults checkWithFixes(AccountNumbers);

	String toString(AccountNumber);
	String toString(CheckResults);

	int to_int(AccountNumber);
}
