#include "OCR.h"
#include "meta.h"

namespace BankOCR {
	using ScannedDigit = std::array<char, 9>;
	namespace {
		const auto scannedDigitList = meta::mk_array(
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
		);

		AccountDigit toAccountDigit(ScannedDigit scannedDigit) {
			auto optional_index = meta::find_value_index(scannedDigitList, scannedDigit);
			if (!optional_index) return AccountDigit::Invalid; 
			return static_cast<AccountDigit>(*optional_index);			
		}

		auto toScannedDigit(ScannedEntry entry, size_t digit) {
			return meta::flatten(meta::generate_to_array<3>([&](auto line) {
				return meta::part<3>(entry[line], digit);
			}));
		}
		auto toScannedDigitLine(ScannedEntry entry) {
			return meta::generate_to_array<AccountNumber().size()>([&](auto digit) {
				return toScannedDigit(entry, digit);
			});
		}
		auto toAccountNumber(ScannedEntry entry) {
			return meta::transform_to_array(toScannedDigitLine(entry), &toAccountDigit);
		}
	}

	AccountNumbers parse(ScannedFile input) {
		return meta::transform_to_vector(input, &toAccountNumber);
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

	CheckResults check(AccountNumbers accountNumbers) {
		return meta::transform_to_vector(accountNumbers, [](auto accountNumber) {
			CheckResult result;
			result.number = accountNumber;
			result.state = [&] {
				if (!isValid(accountNumber)) return CheckState::IllelibleCharacters;
				if (0 != calcChecksum(accountNumber)) return CheckState::WrongChecksum;
				return CheckState::Passed;
			}();
			return result;
		});
	}

	using Alternatives = std::vector<AccountDigit>;
	namespace {
		const auto alternativeMap = meta::mk_array(
			Alternatives{ AccountDigit::Eight }, // 0
			Alternatives{ AccountDigit::Seven }, // 1
			Alternatives{}, // 2
			Alternatives{ AccountDigit::Nine }, // 3
			Alternatives{}, // 4
			Alternatives{ AccountDigit::Six, AccountDigit::Nine }, // 5
			Alternatives{}, // 6
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

	CheckResults checkWithFixes(AccountNumbers accountNumbers) {
		return meta::transform_to_vector(accountNumbers, [](auto accountNumber) {
			CheckResult result;
			result.number = accountNumber;
			result.state = [&] {
				if (!isValid(accountNumber)) return CheckState::IllelibleCharacters;
				if (0 != calcChecksum(accountNumber)) {
					result.fix = [&, fixes = gatherFixes(accountNumber)] {
						if (fixes.empty()) return FixState::Failed;
						if (1 != fixes.size()) return FixState::Ambiguous;
						result.number = fixes.front();
						return FixState::Success;
					}();
					return CheckState::WrongChecksum;
				}
				return CheckState::Passed;
			}();
			return result;
		});
	}

	namespace {
		const std::array<char, 11> accountDigitToChar = meta::str_array("0123456789?");
	}

	String toString(AccountNumber accountNumber) {
		return meta::transform_to_string(accountNumber, [](auto accountDigit) {
			return accountDigitToChar[static_cast<size_t>(accountDigit)];
		});
	}

	String toString(CheckResult checkResult) {
		auto extraStr = [&] {
			switch (checkResult.state) {
			case CheckState::Passed: return "";
			case CheckState::IllelibleCharacters: return " ILL";
			default:
			case CheckState::WrongChecksum:
				switch (checkResult.fix) {
				case FixState::Success: return " FIX";
				case FixState::Ambiguous: return " AMB";
				default:
				case FixState::Failed: return " ERR";
				}
			}
		}();
		return toString(checkResult.number) + extraStr;
	}

	String toString(CheckResults checkResults) {
		return meta::accumulate(checkResults, String{}, [](auto accu, auto checkResult) {
			return accu + toString(checkResult) + "\n";
		});
	}

	int to_int(AccountNumber accountNumber) {
		return meta::accumulate(accountNumber, 0, [](auto accu, auto v) {
			if (accu >= 0 && v != AccountDigit::Invalid)
				return accu * 10 + static_cast<int>(v);
			else
				return -1;
		});
	}
}
