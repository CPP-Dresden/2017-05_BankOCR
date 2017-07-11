#include "OCR.h"
#include "meta.h"

namespace BankOCR {
	using ScannedDigit = std::array<char, 9>;
	using ScannedDigitList = ScannedDigit[10];
	namespace {
		const ScannedDigitList scannedDigitList = {
			meta::str_array(
				" _ "
				"| |"
				"|_|"),
			meta::str_array(
				"   "
				"  |"
				"  |"),
			meta::str_array(
				" _ "
				" _|"
				"|_ "),
			meta::str_array(
				" _ "
				" _|"
				" _|"),
			meta::str_array(
				"   "
				"|_|"
				"  |"),
			meta::str_array(
				" _ "
				"|_ "
				" _|"),
			meta::str_array(
				" _ "
				"|_ "
				"|_|"),
			meta::str_array(
				" _ "
				"  |"
				"  |"),
			meta::str_array(
				" _ "
				"|_|"
				"|_|"),
			meta::str_array(
				" _ "
				"|_|"
				" _|")
		};

		AccountDigit parseDigit(const ScannedDigit& scanned) {
			auto optional_index = meta::find_value_index(scannedDigitList, scanned);
			if (optional_index) return static_cast<AccountDigit>(*optional_index);
			return AccountDigit::Invalid;
		}
	}

	using ScannedDigitLine = std::array<ScannedDigit, AccountNumber().size()>;
	namespace {
		auto chopScannedEntry(const ScannedEntry& entry) {
			ScannedDigitLine result;
			for (auto digit = 0u; digit < AccountNumber().size(); ++digit) {
				for (auto line = 0u; line < 3; ++line)
					meta::part<3>(result[digit], line) = meta::part<3>(entry[line], digit);
			}
			return result;
		}
	}

	AccountNumbers parse(ScannedFile input)
	{
		return meta::transform_to_vector(input, [](const auto& entry)->AccountNumber {
			return meta::transform_to_array(chopScannedEntry(entry), &parseDigit);
		});
	}

	namespace {
		auto isValid(AccountNumber accountNumber) {
			return meta::all_of(accountNumber, [](auto v) { return v != AccountDigit::Invalid; });
		}

		auto calcChecksum(AccountNumber accountNumber) {
			return meta::accumulate(accountNumber, 0, [p = 9](auto accu, auto v) mutable {
				return accu + (p--) * static_cast<int>(v);
			}) % 11;
		}
	}

	CheckResults check(AccountNumbers accountNumbers)
	{
		return meta::transform_to_vector(accountNumbers, [](const auto& accountNumber) {
			CheckResult result;
			result.number = accountNumber;
			if (!isValid(accountNumber)) {
				result.state = CheckState::IllelibleCharacters;
			}
			else if (0 != calcChecksum(accountNumber)) {
				result.state = CheckState::WrongChecksum;
			}
			return result;
		});
	}

	using Alternatives = std::vector<AccountDigit>;
	using AlternativeMap = std::array<Alternatives, 10>;
	namespace {
		const AlternativeMap alternativeMap = meta::mk_array(
			Alternatives{ AccountDigit::Eight }, // 0
			Alternatives{ AccountDigit::Seven }, // 1
			Alternatives{},  // 2
			Alternatives{ AccountDigit::Nine }, // 3
			Alternatives{}, // 4
			Alternatives{ AccountDigit::Six, AccountDigit::Nine }, // 5
			Alternatives{},  // 6
			Alternatives{ AccountDigit::One }, // 7
			Alternatives{ AccountDigit::Zero, AccountDigit::Six, AccountDigit::Nine }, // 8
			Alternatives{ AccountDigit::Three, AccountDigit::Five, AccountDigit::Eight } // 9
		);

		auto gatherFixes(AccountNumber accountNumber) {
			AccountNumbers result;
			for (auto& digit : accountNumber) {
				auto backup = digit;
				for (auto alternative : alternativeMap[static_cast<int>(backup)]) {
					digit = alternative;
					if (0 == calcChecksum(accountNumber)) result.push_back(accountNumber);
				}
				digit = backup;
			}
			return result;
		}
	}

	CheckResults checkWithFixes(AccountNumbers accountNumbers)
	{
		return meta::transform_to_vector(accountNumbers, [](const auto& accountNumber) {
			CheckResult result;
			result.number = accountNumber;
			if (!isValid(accountNumber)) {
				result.state = CheckState::IllelibleCharacters;
			}
			else if (0 != calcChecksum(accountNumber)) {
				result.state = CheckState::WrongChecksum;

				auto fixes = gatherFixes(accountNumber);
				if (fixes.empty()) result.fix = FixState::Failed;
				else if (1 != fixes.size()) result.fix = FixState::Ambiguous;
				else {
					result.fix = FixState::Success;
					result.number = fixes.front();
				}
			}
			return result;
		});
	}

	namespace {
		const std::array<char, 11> accountDigitToChar = meta::str_array("0123456789?");
	}

	String toString(AccountNumber accountNumber)
	{
		return meta::transform_to_string(accountNumber, [](const auto& accountDigit) {
			return accountDigitToChar[static_cast<size_t>(accountDigit)];
		});
	}

	String toString(CheckResults checkResults)
	{
		auto lines = meta::transform_to_vector(checkResults, [](const auto& checkResult) {
			auto numberStr = toString(checkResult.number);
			switch (checkResult.state) {
			case CheckState::Passed:
				return numberStr;

			case CheckState::IllelibleCharacters:
				return numberStr + " ILL";

			case CheckState::WrongChecksum:
				switch (checkResult.fix) {
				case FixState::Success:
					return numberStr + " FIX";
				case FixState::Ambiguous:
					return numberStr + " AMB";
				default:
				case FixState::Failed:
					return numberStr + " ERR";
				}
			default:
				return String{};
			}
		});
		return meta::accumulate(lines, String{}, [](auto accu, auto line) {
			if (accu.empty()) return line;
			return accu + "\n" + line;
		});
	}

	int to_int(AccountNumber accountNumber)
	{
		return meta::accumulate(accountNumber, 0, [](auto accu, auto v) {
			if (accu >= 0 && v != AccountDigit::Invalid)
				return accu * 10 + static_cast<int>(v);
			else
				return -1;
		});
	}

}
