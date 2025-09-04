#include "scope.hpp"

struct MyStruct {};

template<>
struct type_settings<int> : public svh::scope {
	int _min = 0;
	int _max = 100;

	type_settings& min(const int& v) { _min = v; return *this; }
	type_settings& max(const int& v) { _max = v; return *this; }

	const int& get_min() const { return _min; }
	const int& get_max() const { return _max; }
};

template<>
struct type_settings<float> : svh::scope {
	float _min = 0.0f;
	float _max = 1.0f;
	type_settings& min(const float& v) { _min = v; return *this; }
	type_settings& max(const float& v) { _max = v; return *this; }
	const float& get_min() const { return _min; }
	const float& get_max() const { return _max; }
};

int main() {
	svh::scope root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop()
		.push<float>()
		____.min(-1.0f)
		____.max(1.0f)
		.pop()
		.push<MyStruct>()
		____.push<int>()
		________.max(20)
		____.pop()
		.pop();

	auto& int_settings = root.get<int>();
	auto& float_settings = root.get<float>();
	auto& mystruct_settings = root.get<MyStruct>();
	auto& mystruct_int_settings = mystruct_settings.get<int>();
	auto& mystruct_float_settings = mystruct_settings.get<float>(); // Should return root float settings
}