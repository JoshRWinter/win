// my sicknasty testing framework
// emphasis on the nasty

#include <vector>
#include <iostream>

struct Test
{
	Test(void (*fn)(), int lineno)
		: fn(fn)
		, lineno(lineno)
	{}

	void (*fn)();
	int lineno;
};

inline std::vector<Test> tests;

inline void add_test(const Test &test)
{
	tests.push_back(test);
}

inline bool run_test(const Test &test)
{
	try
	{
		test.fn();
		return true;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
}

inline int run_tests(int line = -1)
{
	if (line < 0)
	{
		for (const Test &test : tests)
		{
			if (!run_test(test))
				return 1;
		}

		std::cout << "All " << tests.size() << " tests passed" << std::endl;
		return 0;
	}
	else
	{
		int index = -1;
		for (int i = 0; i < tests.size(); ++i)
		{
			if (tests.at(i).lineno == line)
			{
				index = i;
				break;
			}
		}

		if (index == -1)
		{
			std::cerr << "No test at line number " << line << std::endl;
			return 1;
		}

		if(!run_test(tests.at(index)))
			return false;

		std::cout << "Test on line " << line << " passed" << std::endl;
		return 0;
	}
}

inline std::string stringify(const std::string &str) { return str; }
inline std::string stringify(const char *str) { return std::string(str); }
template <typename T> std::string stringify(const T &t) { return std::to_string(t); }

// preprocessor jankery
#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)

#define TEST() void CONCAT(test_fn_, __LINE__)(); bool CONCAT(test_dummy_, __LINE__) = (add_test(Test(CONCAT(test_fn_, __LINE__), __LINE__)), true); void CONCAT(test_fn_, __LINE__)()

#define is_true(exp) if (!(exp)) throw std::runtime_error(std::to_string(__LINE__) + ": Assertion failed");
#define are_equal(expected, actual) if (stringify(expected) != stringify(actual)) throw std::runtime_error(std::to_string(__LINE__) + ": expected '" + stringify(expected) + "', actual '" + stringify(actual) + "'")
