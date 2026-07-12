#include <string>

#pragma once

struct ChosenExtensions {
    bool cpp = false;
    bool cc = false;
    bool cxx = false;
    bool h = false;
    bool hpp = false;
    bool hh = false;
    bool hxx = false;

    bool check(const std::string&);
};
