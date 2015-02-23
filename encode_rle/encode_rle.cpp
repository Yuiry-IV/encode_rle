// encode_rle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>

const char control_bit = '\x80';

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

template <typename iterator,  typename container>
iterator insert_bytes(const iterator begin, const iterator end, container &out)
{
	if (end == begin)
		return begin;
	out.push_back(control_bit | (end - begin) );
	for (iterator i = begin; i != end; ++i)
		out.push_back( *i );	
	return end;
}

template <typename iterator, typename container>
iterator insert_byte(const iterator begin, const iterator end, container &out)
{
	if ( end == begin )
		return begin;
	out.push_back( end - begin );
	out.push_back(*begin);
	return end;
}


enum encoder_state
{
	sequence_of_values,
	repeated_values,
};

void write_out(char const *const in, unsigned size, encoder_state state, std::ostream &os)
{
	if (!size)
		return;
	os << "[";
	if (repeated_values == state)
		os << "r";
	else
		os << "s";

	os << size;
	os << "]";
	if (repeated_values == state)
		os << *in;
	else
		os << std::string(in, in + size);
}

void encode_rle(char const *const in, unsigned size, std::ostream &os)
{
	if (size < 1)
		return;
	encoder_state state = sequence_of_values;	
	unsigned j = 0;
	unsigned i = 1;
	if (in[j] == in[i])
		state = repeated_values;	
	for (; i < size; ++i)
	{		
		if ( sequence_of_values == state && in[i - 1] == in[i] || i - 1 - j == 127 )
		{
			write_out(in + j, i - 1 - j, state, os);
			j = i-1;
			state = repeated_values;			
		}
		if ( repeated_values == state && in[i - 1] != in[i] || i - j == 127 )
		{
			write_out(in + j, i - j, state, os);
			j = i;
			state = sequence_of_values;
		}
	}
	write_out(in + j, size-j, state, os);	
}

void encode_rle(std::istream &is, std::ostream &os)
{
	std::string buf;
	char prev_prev_ch = 0;
	char prev_ch =0;
	char ch = 0;
	while ( true )
	{		
		char tmp = is.get();		
		if (!is) break;
		ch = tmp;
		if ( prev_prev_ch != 0 )
		{
			if (prev_prev_ch == prev_ch && prev_ch != ch || buf.size() == 127)
			{
				os << "[r" << buf.size() << "]" << buf[0];
				buf.clear();
			}
			else if (prev_prev_ch != prev_ch && prev_ch == ch || buf.size() == 127)
			{
				(void)buf.pop_back();
				if (!buf.empty())
				{
					os << "[s" << buf.size() << "]" << buf;
					buf.clear();
				}
				buf.push_back(ch);
			}
		}

		buf.push_back(ch);
		prev_prev_ch = prev_ch;
		prev_ch = ch;
	}

	if (!buf.empty())
	{
		if (prev_prev_ch == prev_ch && prev_ch == ch)
			os << "[r" << buf.size() << "]" << buf[0];
		else
			os << "[s" << buf.size() << "]" << buf;
	}
}

template <typename in_iterator, typename out_iterator> 
inline out_iterator encode_rle_i(in_iterator begin, in_iterator end, out_iterator out)
{
	if (begin == end)
		return out;

	if (begin+1 == end)
		return out;		
	
	in_iterator i = begin+1;
	bool prev_equal = *(i - 1) == *i;

	while ( i != end )
	{
		bool equal = *(i-1) == *i;
		
		if( ( equal ^ prev_equal ) || (i - begin == 127) )
		{
			if (!prev_equal)
				i--;
			*out = (prev_equal?0:0x80) | (i - begin);
			out++;
			if (!prev_equal)
			{
				out = std::copy(begin, i, out);				
			}
			else
			{
				*out++ = *begin;
			}
			begin = i;
		}
		prev_equal = equal;
		++i;
	}

	*out = (prev_equal ? 0 : 0x80) | (end - begin);
	out++;
	if (!prev_equal)
	{
		out = std::copy(begin, end, out);
	}
	else
	{
		*out++ = *begin;
	}
	return out;
}

template <typename container> bool encode_rle_c(const container &in, container & out)
{
	if( in.size()<2 )
		return false;
	out.clear();

	container::const_iterator seq_begin = in.begin();
	container::const_iterator i = in.begin() + 1;
	bool prev_equal = *(i - 1) == *i;	
	while( i != in.end() )
	{
		bool equal = *(i - 1) == *i;
		container::const_iterator seq_end = (!prev_equal) ? i - 1 : i;
		
		if ((equal && !prev_equal) || (!equal &&  prev_equal) || (seq_end - seq_begin == 127) )
		{
			if (prev_equal)
				seq_begin = insert_byte(seq_begin, seq_end, out);
			else
				seq_begin = insert_bytes(seq_begin, seq_end, out);
		}
		prev_equal = equal;
		++i;
	}

	if (!prev_equal)
		insert_bytes(seq_begin, in.end(), out);
	if (prev_equal)
		insert_byte(seq_begin, in.end(), out);

	return true;	
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<std::string> test_results;
	std::vector<std::string> test_data;
	
	test_data.push_back("aaaqqqssrtpp");	test_results.push_back("[r3]a[r3]q[r2]s[s2]rt[r2]p");
	test_data.push_back("aaaabcebcebceffff");  test_results.push_back("[r4]a[s9]bcebcebce[r4]f");
	test_data.push_back("");				test_results.push_back("");
	test_data.push_back("a");				test_results.push_back("[s1]a");
	test_data.push_back("aa");				test_results.push_back("[r2]a");
	test_data.push_back("ba");				test_results.push_back("[s2]ba");
	test_data.push_back("abc");				test_results.push_back("[s3]abc");
	test_data.push_back("aaaabcebcebceffff");  test_results.push_back("[r4]a[s9]bcebcebce[r4]f");	
	test_data.push_back(std::string(128, 'a')); test_results.push_back("[r127]a[s1]a");

	for (unsigned int i = 0; i < test_data.size(); ++i)
	{
		std::stringstream out_s;				
		const std::string &s = test_data[i];
		encode_rle(s.c_str(), s.size(), out_s);		
		const std::string r = out_s.str();
		std::cout << "string[" << i << "] input: " << test_data[i] << " output: " << r << "\n";
		assert(test_results[i] == r);
	}	

	/*for (unsigned int i = 0; i < test_data.size(); ++i)
	{
		std::stringstream in_s;
		in_s.str(test_data[i]);
		std::stringstream out_s;
		out_s.str("");
		encode_rle(in_s, out_s);
		std::string r = out_s.str();

		std::cout << "string[" << i << "] input: " << test_data[i] << " output: " << r << "\n";

		assert( test_results[i] == r );
	}*/

	//std::string s( test_data[0].size(), '\0' );
	//std::string::iterator i = encode_rle<std::string::iterator, std::string::iterator>(test_data[0].begin(), test_data[0].end(), s.begin());
	//s.erase( i, s.end() );

	//std::cout << dump_rle(s);

	//for (unsigned int i = 0; i < test_data.size(); ++i)
	//{
	//	std::string r;
	//	const std::string &s = test_data[i];
	//	bool result = encode_rle<std::string>(s, r);
	//	std::cout << "string[" << i << "]: encode is " << std::boolalpha << result << "; input: " << s << " output: " << dump_rle(r) << "\n";
	//	assert(test_results[i] == dump_rle(r));
	//}
	//
	//std::vector<char> in;
	//std::vector<char> out;

	//in.push_back('a'); in.push_back('a'); in.push_back('a'); in.push_back('a');
	//// param. calculation side effect...
	//// std::cout << "vector result is " << encode_rle(in, out) << " input: aaaa output:" << dump_rle(out) << "\n";
	//bool result = encode_rle<std::vector>(in, out);
	//std::cout << "vector[0] result is " << std::boolalpha << result << " input: aaaa output:" << dump_rle(out) << "\n";
	//assert(dump_rle(out) == "[r4]a");
	//std::cout << "\n";
	
	return 0;
}

