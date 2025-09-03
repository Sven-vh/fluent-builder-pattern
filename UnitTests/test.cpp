#include "pch.h"

struct MyStruct {};

template<>
struct type_settings<int> : svh::scope {
	int _min = std::numeric_limits<int>::min();
	int _max = std::numeric_limits<int>::max();

	type_settings& min(const int& v) { _min = v; return *this; }
	type_settings& max(const int& v) { _max = v; return *this; }

	const int& get_min() const { return _min; }
	const int& get_max() const { return _max; }
};

template<>
struct type_settings<float> : svh::scope {
	float _min = std::numeric_limits<float>::min();
	float _max = std::numeric_limits<float>::max();
	type_settings& min(const float& v) { _min = v; return *this; }
	type_settings& max(const float& v) { _max = v; return *this; }
	const float& get_min() const { return _min; }
	const float& get_max() const { return _max; }
};

TEST(Default, Single) {
	svh::scope root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop();

	auto& int_settings = root.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
}

TEST(Default, Multiple) {
	svh::scope root;
	root.push<int>()
		____.min(0)
		____.max(50)
		.pop()
		.push<float>()
		____.min(-1.0f)
		____.max(1.0f)
		.pop();

	auto& int_settings = root.get<int>();
	auto& float_settings = root.get<float>();

	EXPECT_EQ(int_settings.get_min(), 0);
	EXPECT_EQ(int_settings.get_max(), 50);

	EXPECT_EQ(float_settings.get_min(), -1.0f);
	EXPECT_EQ(float_settings.get_max(), 1.0f);
}

TEST(Default, Nested) {
	svh::scope root;
	root.push<MyStruct>()
		____.push<int>()
		________.min(10)
		________.max(20)
		____.pop()
		.pop();

	auto& mystruct_int_settings = root.get<MyStruct>().get<int>();

	EXPECT_EQ(mystruct_int_settings.get_min(), 10);
	EXPECT_EQ(mystruct_int_settings.get_max(), 20);
}

TEST(Default, Fallback) {
	svh::scope root;

	if (SVH_AUTO_INSERT) {
		auto& int_settings = root.get<int>();
		auto& float_settings = root.get<float>();
		EXPECT_EQ(int_settings.get_min(), std::numeric_limits<int>::min());
		EXPECT_EQ(int_settings.get_max(), std::numeric_limits<int>::max());
		EXPECT_EQ(float_settings.get_min(), std::numeric_limits<float>::min());
		EXPECT_EQ(float_settings.get_max(), std::numeric_limits<float>::max());
	} else {
		EXPECT_THROW(root.get<int>(), std::runtime_error);
		EXPECT_THROW(root.get<float>(), std::runtime_error);
	}
}

TEST(Default, Recursive) {
	svh::scope root;
	root.push<int>()
		____.min(0)
		____.max(50)
		.pop()
		.push<MyStruct>()
		.pop()
		.push<float>()
		____.min(-1.0f)
		____.max(1.0f)
		.pop();

	auto& mystruct_settings = root.get<MyStruct>();
	if (SVH_RECURSIVE_SEARCH) {
		auto& mystruct_int_settings = mystruct_settings.get<int>();
		auto& mystruct_float_settings = mystruct_settings.get<float>();
		EXPECT_EQ(mystruct_int_settings.get_min(), 0);
		EXPECT_EQ(mystruct_int_settings.get_max(), 50);
		EXPECT_EQ(mystruct_float_settings.get_min(), -1.0f);
		EXPECT_EQ(mystruct_float_settings.get_max(), 1.0f);
	} else {
		EXPECT_THROW(mystruct_settings.get<int>(), std::runtime_error);
		EXPECT_THROW(mystruct_settings.get<float>(), std::runtime_error);
	}
}

TEST(Default, overides) {
	svh::scope root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop()
		.push<MyStruct>()
		____.push<int>()
		/* Override */
		________.max(20)
		____.pop()
		.pop();

	auto& int_settings = root.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);

	auto& mystruct_int_settings = root.get<MyStruct>().get<int>();
	EXPECT_EQ(mystruct_int_settings.get_min(), -50);
	EXPECT_EQ(mystruct_int_settings.get_max(), 20);
}