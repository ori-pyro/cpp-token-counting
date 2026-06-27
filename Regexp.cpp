#include <iostream>
#include <re2/re2.h>
#include "Regexp.h"
using namespace std;

void Regexp::mySplit(const string& text, vector<string>& nonmatches, vector<string>& matches) {
	re2::StringPiece rest(text);
	const char* start = text.data();
	const char* current = start;
	re2::StringPiece match;
	while (RE2::FindAndConsume(&rest, separators, &match)) {
		const char* start = match.data();
		if (start > current) {
			cout<<"100\n";
			nonmatches.emplace_back(current,start-current);
		}
		matches.emplace_back(match.data(),match.size());
		current = match.data()+match.size();
	}
	const char* end = text.data() + text.size();
	if (end > current) {
		nonmatches.emplace_back(current, end - current);
	}
}

int main() {
	cout<<"Titata";
	string code = "int main() { int x = 5; x += 2; }";
	Regexp reps;
	vector<string> matches;
	vector<string> nonmatches;
	reps.mySplit(code, nonmatches, matches);
	for (const auto& s : matches) cout << "'" << s << "' ";
    cout <<"\ntitatu\n";
    for (const auto& s : nonmatches) cout << "'" << s << "' ";
}