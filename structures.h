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

    int cpp_cnt = 0;
    int cc_cnt = 0;
    int cxx_cnt = 0;
    int h_cnt = 0;
    int hpp_cnt = 0;
    int hh_cnt = 0;
    int hxx_cnt = 0;


    bool check(const std::string& extension) {
        if ((extension == ".cpp") && cpp)      { return true; }
        else if ((extension == ".cc") && cc)   { return true; }
        else if ((extension == ".cxx") && cxx) { return true; }
        else if ((extension == ".h") && h)     { return true; }
        else if ((extension == ".hpp") && hpp) { return true; }
        else if ((extension == ".hh") && hh)   { return true; }
        else if ((extension == ".hxx") && hxx) { return true; }
        else { return true; }
    }

    bool check_and_count(const std::string& extension) {
        if ((extension == ".cpp") && cpp)      { cpp_cnt++; return true; }
        else if ((extension == ".cc") && cc)   { cc_cnt++; return true; }
        else if ((extension == ".cxx") && cxx) { cxx_cnt++; return true; }
        else if ((extension == ".h") && h)     { h_cnt++; return true; }
        else if ((extension == ".hpp") && hpp) { hpp_cnt++; return true; }
        else if ((extension == ".hh") && hh)   { hh_cnt++; return true; }
        else if ((extension == ".hxx") && hxx) { hxx_cnt++; return true; }
        else { return true; }
    }
};
