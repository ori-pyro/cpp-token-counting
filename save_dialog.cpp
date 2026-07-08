#include <portable-file-dialogs.h>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include "save_dialog.h"
#include <fstream>      // Чтение файлов

std::unique_ptr<pfd::save_file> save_dialog = nullptr;
std::unique_ptr<pfd::select_folder> select_dialog = nullptr;


void open_save_dialog() {
    save_dialog = std::make_unique<pfd::save_file>(
        "Заголовок",
        "token_count.txt",
        std::vector<std::string>{ "Text Files (*.txt)", "*.txt" },
        pfd::opt::force_overwrite
    );
}

void save_dialog_update(
    const std::string project_path,
    const std::map<std::string, int>& types,
    const std::map<std::string, std::map<std::string, int>>& sub_types)
{
    if (save_dialog->ready()) {
        std::filesystem::path save_path = save_dialog->result();
        if (!save_path.empty()) {
            std::ofstream file(save_path);
            file << "\nProject: " << project_path << "\n\n";
            for (auto& [type, cnt] : types) {
                file << std::left << std::setw(30) << type
                    << std::right << std::setw(10) << cnt << "\n";
                if (sub_types.contains(type)) {
                    for (auto& [sub_type, sub_cnt] : sub_types.at(type)) {
                        file << '\t' << std::left << std::setw(30) << sub_type
                            << std::right << std::setw(10) << sub_cnt << "\n";
                    }
                }
            }

        }
        save_dialog.reset();
    }
}

void open_select_dialog() {
    select_dialog = std::make_unique<pfd::select_folder>("Выберите директорию");
}

void select_dialog_update(std::string& dir_path_str) {
    if (select_dialog->ready()) {
        std::filesystem::path dir_path = select_dialog->result();
        dir_path_str = dir_path.string();
        select_dialog.reset();
    }
}
