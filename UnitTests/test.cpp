#include "pch.h"

template<class T, class Enable = void>
struct type_settings : svh::scope<type_settings> {};

struct MyStruct {};

template<>
struct type_settings<int> : svh::scope<type_settings> {
	int _min = std::numeric_limits<int>::min();
	int _max = std::numeric_limits<int>::max();

	type_settings& min(const int& v) { _min = v; return *this; }
	type_settings& max(const int& v) { _max = v; return *this; }

	const int& get_min() const { return _min; }
	const int& get_max() const { return _max; }
};

template<>
struct type_settings<float> : svh::scope<type_settings> {
	float _min = std::numeric_limits<float>::min();
	float _max = std::numeric_limits<float>::max();
	type_settings& min(const float& v) { _min = v; return *this; }
	type_settings& max(const float& v) { _max = v; return *this; }
	type_settings& balls() { return *this; } // For testing
	const float& get_min() const { return _min; }
	const float& get_max() const { return _max; }
};

TEST(Default, push_single) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop();

	auto& int_settings = root.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
}

TEST(Default, push_single_const) {
	svh::scope<type_settings> root;
	root.push<const int>()
		____.min(-50)
		____.max(50)
		.pop();

	auto& int_settings = root.get<const int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
}

TEST(Default, push_multiple) {
	svh::scope<type_settings> root;
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

TEST(Default, pop_to_root) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.push<bool>()
		________.push<float>()
		____________.min(-1.0f)
		____________.max(1.0f)
		.pop_to_root()
		.push<float>()
		____.min(-50.0f)
		____.max(50.0f)
		.pop();

	auto& float_settings = root.get<float>();
	EXPECT_EQ(float_settings.get_min(), -50.0f);
	EXPECT_EQ(float_settings.get_max(), 50.0f);
}

TEST(Default, push_nested) {
	svh::scope<type_settings> root;
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

TEST(Default, push_and_set) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		____.push<float>()
		________.min(-1.0f)
		________.max(1.0f)
		____.pop()
		.pop();

	auto& int_settings = root.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);

	auto& float_settings = int_settings.get<float>();
	EXPECT_EQ(float_settings.get_min(), -1.0f);
	EXPECT_EQ(float_settings.get_max(), 1.0f);
}

TEST(Default, default_fallback) {
	svh::scope<type_settings> root;

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

TEST(Default, recusrive_fallback) {
	svh::scope<type_settings> root;
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
	auto& mystruct_int_settings = mystruct_settings.get<int>();
	auto& mystruct_float_settings = mystruct_settings.get<float>();
	EXPECT_EQ(mystruct_int_settings.get_min(), 0);
	EXPECT_EQ(mystruct_int_settings.get_max(), 50);
	EXPECT_EQ(mystruct_float_settings.get_min(), -1.0f);
	EXPECT_EQ(mystruct_float_settings.get_max(), 1.0f);
}

TEST(Default, overides) {
	svh::scope<type_settings> root;
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

TEST(Default, push_default) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop()
		.push<MyStruct>()
		____.push_default<int>()
		________.max(20)
		____.pop()
		.pop();

	auto& int_settings = root.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);

	auto& mystruct_int_settings = root.get<MyStruct>().get<int>();
	EXPECT_EQ(mystruct_int_settings.get_min(), std::numeric_limits<int>::min());
	EXPECT_EQ(mystruct_int_settings.get_max(), 20);
}

// Function parameter passing
static void func(const svh::scope<type_settings>& s) {
	auto& int_settings = s.get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
}

TEST(Default, func_param) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop();
	func(root);
}

TEST(Default, typed_func_param) {
	svh::scope<type_settings> root;
	root.push<MyStruct>()
		____.push<int>()
		________.min(-50)
		________.max(50)
		____.pop()
		.pop();

	func(root.get<MyStruct>());
}

TEST(Default, multi_push) {
	svh::scope<type_settings> root;
	root.push<MyStruct, int>()
		____.min(-50)
		____.max(50)
		.pop()
		.pop();

	auto& int_settings = root.get<MyStruct>().get<int>();
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
}

TEST(Default, multi_pop) {
	svh::scope<type_settings> root_a;
	root_a.push<MyStruct, float, bool, int>()
		____.min(-50)
		____.max(50)
		.pop(2) // Pop twice back to float
		.push<int>()
		____.min(-100)
		____.max(100)
		.pop(3);

	/* Is same as */

	svh::scope<type_settings> root_b;
	root_b.push<MyStruct>()
		____.push<float>()
		________.push<bool>()
		____________.push<int>()
		________________.min(-50)
		________________.max(50)
		____________.pop()
		________.pop()
		________.push<int>()
		____________.min(-100)
		____________.max(100)
		________.pop()
		____.pop()
		.pop();


	auto& int_settings_a = root_a.get<MyStruct>().get<float>().get<bool>().get<int>();
	auto& int_settings_b = root_b.get<MyStruct>().get<float>().get<bool>().get<int>();
	EXPECT_EQ(int_settings_a.get_min(), int_settings_b.get_min());
	EXPECT_EQ(int_settings_a.get_max(), int_settings_b.get_max());

	auto& int_settings_a2 = root_a.get<MyStruct>().get<float>().get<int>();
	auto& int_settings_b2 = root_b.get<MyStruct>().get<float>().get<int>();
	EXPECT_EQ(int_settings_a2.get_min(), int_settings_b2.get_min());
	EXPECT_EQ(int_settings_a2.get_max(), int_settings_b2.get_max());
}

TEST(Default, multi_get) {
	svh::scope<type_settings> root;
	root.push<MyStruct, bool, float, int>()
		________.min(-50)
		________.max(50)
		.pop();

	auto& int_settings = root.get<MyStruct, bool, float, int>();
	/* Same as */
	auto& int_settings2 = root.get<MyStruct>().get<bool>().get<float>().get<int>();

	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
	EXPECT_EQ(int_settings.get_min(), int_settings2.get_min());
	EXPECT_EQ(int_settings.get_max(), int_settings2.get_max());
}

TEST(Default, get_nonexistent) {
	svh::scope<type_settings> root;
	if (SVH_AUTO_INSERT) {
		auto& mystruct_settings = root.get<MyStruct>();
		EXPECT_TRUE(&mystruct_settings != nullptr);
	} else {
		EXPECT_THROW(root.get<MyStruct>(), std::runtime_error);
	}
}

/* Member variables tests*/
struct TestStruct {
	int a;
	int b;
};

TEST(Default, member_variable) {
	svh::scope<type_settings> root;
	root.push<TestStruct>()
		____.push_member<&TestStruct::a>()
		________.min(0)
		________.max(10)
		____.pop()
		____.push_member<&TestStruct::b>()
		________.min(20)
		________.max(30)
		____.pop()
		.pop();

	auto& a_settings = root.get<TestStruct>().get_member<&TestStruct::a>();
	EXPECT_EQ(a_settings.get_min(), 0);
	EXPECT_EQ(a_settings.get_max(), 10);

	auto& b_settings = root.get<TestStruct>().get_member<&TestStruct::b>();
	EXPECT_EQ(b_settings.get_min(), 20);
	EXPECT_EQ(b_settings.get_max(), 30);
}

TEST(Default, member_variable_runtime) {
	svh::scope<type_settings> root;
	root.push<TestStruct>()
		____.push_member<&TestStruct::a>()
		________.min(0)
		________.max(10)
		____.pop()
		____.push_member<&TestStruct::b>()
		________.min(20)
		________.max(30)
		____.pop()
		.pop();

	TestStruct instance{ 1, 2 };

	auto& a_settings = root.get<TestStruct>().get_member(instance, instance.a);
	EXPECT_EQ(a_settings.get_min(), 0);
	EXPECT_EQ(a_settings.get_max(), 10);

	auto& b_settings = root.get<TestStruct>().get_member(instance, instance.b);
	EXPECT_EQ(b_settings.get_min(), 20);
	EXPECT_EQ(b_settings.get_max(), 30);
}

TEST(Default, member_fallback) {
	svh::scope<type_settings> root;
	root.push<TestStruct>()
		____.push<int>()
		________.min(0)
		________.max(5)
		____.pop()
		____.push_member<&TestStruct::b>()
		________.max(10)
		____.pop()
		.pop();

	TestStruct instance{ 1, 2 };
	auto& b_settings = root.get<TestStruct>().get_member(instance, instance.a);
	EXPECT_EQ(b_settings.get_min(), 0);
	EXPECT_EQ(b_settings.get_max(), 5);

	auto& a_settings = root.get<TestStruct>().get_member(instance, instance.b);
	EXPECT_EQ(a_settings.get_min(), 0);
	EXPECT_EQ(a_settings.get_max(), 10);
}

template<class T, class M>
void do_something(const svh::scope<type_settings>& s, const T& instance, const M& member) {
	auto& settings = s.get<TestStruct>().get_member(instance, member);
	EXPECT_EQ(settings.get_min(), 0);
	EXPECT_EQ(settings.get_max(), 10);
}

TEST(Default, member_variable_func) {
	svh::scope<type_settings> root;
	root.push<TestStruct>()
		____.push_member<&TestStruct::a>()
		________.min(0)
		________.max(10)
		____.pop()
		.pop();
	TestStruct instance{ 1, 2 };
	do_something(root, instance, instance.a);
}

TEST(Default, member_push) {
	svh::scope<type_settings> root;
	root.push<TestStruct>()
		____.push_member<&TestStruct::a>()
		________.min(0)
		________.max(10)
		________.push<int>()
		____________.min(-5)
		________.pop()
		____.pop()
		.pop();

	TestStruct instance{ 1, 2 };

	auto& a_settings = root.get<TestStruct>().get_member(instance, instance.a);
	EXPECT_EQ(a_settings.get_min(), 0);
	EXPECT_EQ(a_settings.get_max(), 10);

	auto& other = root.get<TestStruct>().get_member(instance, instance.a).get<int>();
	/* Should fallback to member A settings */
	EXPECT_EQ(other.get_min(), -5);
	EXPECT_EQ(other.get_max(), 10);
}

TEST(Default, test_app) {
	auto root = svh::scope<type_settings>();
	root.push_member<&TestStruct::a>()
		____.min(0)
		____.max(50)
		.pop()
		.push_member<&TestStruct::b>()
		____.min(100)
		____.max(200)
		.pop();

	auto& member_a_settings = root.get_member<&TestStruct::a>();
	auto& int_settings = member_a_settings.get<int>();
	EXPECT_EQ(int_settings.get_min(), 0);
	EXPECT_EQ(int_settings.get_max(), 50);

	auto& member_b_settings = root.get_member<&TestStruct::b>();
	auto& int_settings_b = member_b_settings.get<int>();
	EXPECT_EQ(int_settings_b.get_min(), 100);
	EXPECT_EQ(int_settings_b.get_max(), 200);
}

/* Other generic builders */
template<class T, class Enable = void>
struct other_settings : svh::scope<other_settings> {};

/*other_settings for int*/
template<>
struct other_settings<int> : svh::scope<other_settings> {
	int _value = 0;
	other_settings& value(const int& v) { _value = v; return *this; }
	const int& get_value() const { return _value; }
};

TEST(Default, other_settings) {
	svh::scope<type_settings> root;
	root.push<int>()
		____.min(-50)
		____.max(50)
		.pop();

	auto& int_settings = root.get<int>();
	
	EXPECT_EQ(int_settings.get_min(), -50);
	EXPECT_EQ(int_settings.get_max(), 50);
	
	auto other_root = svh::scope<other_settings>();
	other_root.push<int>()
		____.value(123)
		.pop();
	
	auto& other_int_settings = other_root.get<int>();
	EXPECT_EQ(other_int_settings.get_value(), 123);
}