#include <portable-file-dialogs.h>
#include <filesystem>
#include <memory>
#include "save_dialog.h"
#include <fstream>      // Чтение файлов

std::unique_ptr<pfd::save_file> save_dialog = nullptr;

void open_save_dialog() {
    save_dialog = std::make_unique<pfd::save_file>(
        "Заголовок",
        "token_count.txt",
        std::vector<std::string>{ "Text Files (*.txt)", "*.txt" },
        pfd::opt::force_overwrite
    );
}

void save_dialog_update(const std::vector<std::string>& string_column, const std::vector<int>& int_column) {
    if (save_dialog->ready()) {
        std::filesystem::path save_path = save_dialog->result();
        if (!save_path.empty()) {
            std::ofstream file(save_path);
            for (int i = 0; i < string_column.size(); i++) {
                file << string_column[i] << "; " << int_column[i] << '\n';
            }
        }
        save_dialog.reset();
    }
}
