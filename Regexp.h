#pragma once
#include <string>
#include <vector>
#include <re2/re2.h>
struct Regexp {
	
	RE2 separators = RE2(R"((\s+|".*"|xor_eq|not_eq|xor|not|bitand|bitor|compl|and_eq|or_eq|or|and|>>=|<<=|>>|<<|&&|\|\||\+\+|--|<=>|>=|<=|==|\+=|-=|\*=|/=|%=|^=|&=|\|=|!=|<%|%>|<:|:>|\.\.\.|::|\.\*|->|->\*|\^\^|[][(){};:?~!+\-*/%\^&|=<>,.]))");
	void mySplit(const std::string& text, std::vector<std::string>& nonmatches, std::vector<std::string>& matches);
};