#include <iostream>
#include <fstream>
#include <filesystem>
#include "parser.h"
#include "tree_sitter/api.h"
#include <vector>
#include <set>

namespace fs = std::filesystem;

// создаём объект языка из tree-sitter-cpp, чтобы потом подключить к tree-sitter парсеру
extern "C" TSLanguage *tree_sitter_cpp();

// типы токенов из tree-sitter-cpp, которые не нужны
std::set<std::string> exseption = { "" };

void DFS(TSNode node, std::vector<TSNode>& visited_nodes) {
    if (ts_node_is_null(node)) {
        return;
    }
    visited_nodes.push_back(node);
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        DFS(child, visited_nodes);
    }
}


int main() {
    std::ifstream file("test/test_file.cpp");

    if (!file.is_open()) {
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str(); // Весь код в одну строку
    // parser.parser(code);

    TSParser* PARSER = ts_parser_new();
    ts_parser_set_language(PARSER, tree_sitter_cpp());

    TSTree* TREE = ts_parser_parse_string(
        PARSER,         // Указатель на парсер
        nullptr,        // указатель на прошлое дерево (нам не надо)
        code.c_str(),   // начало строки
        code.length()   // длина строки
    );

    TSNode ROOT = ts_tree_root_node(TREE);

    std::vector<TSNode> nodes;
    DFS(ROOT, nodes);

    for (auto& node : nodes) {
        if (ts_node_child_count(node) == 0) { // Только листья дерева (в них лежат лексемы)
            uint32_t start = ts_node_start_byte(node);
            uint32_t length = ts_node_end_byte(node) - start;
            std::cout << code.substr(start, length) << "\033[20G" << ts_node_type(node) << '\n';
        }
    }
    // ИДЕЯ (хуйня)
    // сделать вывод через наш парсер в том же формате что и выше, а потом сравнить с тем что вывел tree-sitter
    // Надо будет добавить исключения для tree-sitter, которые не может обрабатывать наш парсер и добавить в tree-sitter
    // то, что наш парсер умеет (floating-point-litaral, ...)

    ts_tree_delete(TREE);
    ts_parser_delete(PARSER);
}
