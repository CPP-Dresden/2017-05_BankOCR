#pragma once
#include <optional>
#include <algorithm>
#include <numeric>
#include <type_traits>

namespace meta {
	template<class Container, class Callback>
	auto transform_to_vector(const Container& input, Callback&& callback) {
		using std::begin;
		using std::end;
		using Input = decltype(*begin(input));
		using Output = std::result_of_t<Callback(Input)>;
		std::vector<Output> result;
		result.resize(input.size());
		std::transform(begin(input), end(input), result.begin(), (Callback&&)callback);
		return result;
	}

	template<class Container, class Callback>
	auto transform_to_string(const Container& input, Callback&& callback) {
		using std::begin;
		using std::end;
		std::string result;
		result.resize(input.size());
		std::transform(begin(input), end(input), result.begin(), (Callback&&)callback);
		return result;
	}

	template<class Container, class Callback>
	auto transform_to_array(const Container& input, Callback&& callback) {
		using std::begin;
		using std::end;
		using Input = decltype(*begin(input));
		using Output = std::result_of_t<Callback(Input)>;
		std::array<Output, Container().size()> result;
		std::transform(begin(input), end(input), result.begin(), (Callback&&)callback);
		return result;
	}

	template<size_t Size, class Callback>
	auto generate_to_array(Callback&& callback) {
		using Output = std::result_of_t<Callback(size_t)>;
		std::array<std::remove_const_t<Output>, Size> result;
		std::generate(result.begin(), result.end(), [index = 0u, cb = (Callback&&)callback]() mutable {
			return cb(index++);
		});
		return result;
	}

	template<class Container, class Target, class Callback>
	auto accumulate(const Container& input, const Target& accu, Callback&& callback) {
		using std::begin;
		using std::end;
		return std::accumulate(begin(input), end(input), accu, (Callback&&)callback);
	}

	template<class Container, class Callback>
	auto all_of(const Container& input, Callback&& callback) {
		using std::begin;
		using std::end;
		return std::all_of(begin(input), end(input), (Callback&&)callback);
	}

	template<class Container, class Value>
	auto find_value_index(const Container& container, const Value& value) {
		using std::begin;
		using std::end;
		using optional_index = std::optional<size_t>;
		auto it = std::find(begin(container), end(container), value);
		if (it == end(container)) return optional_index();
		auto index = std::distance(begin(container), it);
		return optional_index(index);
	}

	template<size_t count, class Container>
	auto& part(Container& container, size_t index) {
		using std::begin;
		using Element = std::remove_reference_t<decltype(*begin(container))>;
		using Array = std::array<std::remove_const_t<Element>, count>;
		using ConstFixed = std::conditional_t<std::is_const_v<Element>, const Array*, Array*>;
		return *reinterpret_cast<ConstFixed>(&container[index*count]);
	}

	template<class E, size_t S>
	constexpr auto str_array(E(&v)[S]) {
		std::array<std::remove_const_t<E>, S - 1> result;
		for (auto i = 0u; i < S - 1; ++i) result[i] = v[i];
		return result;
	}

	template<class E, class... A>
	constexpr auto mk_array(E e, A... a) {
		std::array<std::remove_const_t<E>, 1 + sizeof...(A)> result = { { e, a... } };
		return result;
	}

	template<class E, size_t count_outer, size_t count_inner>
	constexpr auto& flatten(const std::array<std::array<E, count_inner>, count_outer>& a) {
		return reinterpret_cast<const std::array<E, count_inner*count_outer>&>(a);
	}
}
