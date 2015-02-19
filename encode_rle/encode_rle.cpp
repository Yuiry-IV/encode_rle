// encode_rle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>

template <typename container> std::string dump_rle(const container &in)
{
	std::stringstream ss;
	container::const_iterator i = in.begin();

	while (i != in.end())
	{
		ss << "[";
		if ((*i & 0x80) == 0x80)
			ss << 's';
		else
			ss << 'r';
		ss << (*i & 0x7F) << "]";

		if ((*i & 0x80) == 0x80)
		{
			for (unsigned l = *i & 0x7F; l > 0; --l)
			{
				++i;
				ss << char(*i);
			}
		}
		else
		{
			++i;
			ss << char(*i);
		}
		++i;
	}

	std::string s = ss.str();
	return s;
}

template <typename container> bool encode_rle(const container &in, container & out)
{
	if (in.size()<2)
		return false;

	out.clear();

	container::const_iterator seq_begin = in.begin();
	container::const_iterator i = in.begin() + 1;
	bool prev_equal = *(i - 1) == *i;

	while (i != in.end())
	{
		bool equal = *(i - 1) == *i;
		if (equal && !prev_equal)
		{
			if ((i - 1 - seq_begin) > 0)
			{
				out.push_back(char(0x80 | (i - 1 - seq_begin)));
				for (unsigned k = 0; k < (i - 1 - seq_begin); ++k)
					out.push_back(*(seq_begin + k));
				seq_begin = i - 1;
			}
		}

		if (!equal && prev_equal)
		{
			if (i - seq_begin > 0)
			{
				out.push_back(char(i - seq_begin));
				out.push_back(*seq_begin);
				seq_begin = i;
			}
		}
		prev_equal = equal;
		++i;
	}

	if (prev_equal)
	{
		if (in.end() - seq_begin > 0)
		{
			out.push_back(char(in.end() - seq_begin));
			out.push_back(*seq_begin);
		}
	}
	if (!prev_equal)
	{
		if (in.end() - seq_begin > 0)
		{
			out.push_back(char(0x80 | (in.end() - seq_begin)));
			for (unsigned k = 0; k < (in.end() - seq_begin); ++k)
				out.push_back(*(seq_begin + k));
			seq_begin = i - 1;
		}
	}
	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<std::string> test_results;
	std::vector<std::string> test_data;

	std::cout << dump_rle(std::string());

	//test_data.push_back("aaaabcbcbcdddd");  test_results.push_back("[r4]a[s6]bcbcbc[r4]d");
	//test_data.push_back("aaaqqqssrtpp");	test_results.push_back("[r3]a[r3]q[r2]s[s2]rt[r2]p");
	//test_data.push_back("");				test_results.push_back("");
	//test_data.push_back("a");				test_results.push_back("");
	test_data.push_back("aa");				test_results.push_back("[r2]a");
	test_data.push_back("ba");				test_results.push_back("[s2]ba");
	//test_data.push_back("abc");				test_results.push_back("[s3]abc");

	for (unsigned int i = 0; i < test_data.size(); ++i)
	{
		std::string r;
		const std::string &s = test_data[i];
		bool result = encode_rle(s, r);
		std::cout << "string" << i << ": encode is " << std::boolalpha << result << "; input: " << s << " output: " << dump_rle(r) << "\n";
		assert(test_results[i] == dump_rle(r));
	}

	std::vector<char> in;
	std::vector<char> out;

	in.push_back('a'); in.push_back('a'); in.push_back('a'); in.push_back('a');
	// param. calculation side effect...
	// std::cout << "vector result is " << encode_rle(in, out) << " input: aaaa output:" << dump_rle(out) << "\n";
	bool result = encode_rle(in, out);
	std::cout << "vector result is " << std::boolalpha << result << " input: aaaa output:" << dump_rle(out) << "\n";
	assert(dump_rle(out) == "[r4]a");
	std::cout << "\n";
	return 0;
}

