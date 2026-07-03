#include <memory>
#include <unordered_map>

namespace pfd { class save_file; }
extern std::unique_ptr<pfd::save_file> save_dialog;

void open_save_dialog();
void save_dialog_update(
    const std::unordered_map<std::string, int>& types,
    const std::unordered_map<std::string, std::unordered_map<std::string, int>>& sub_types);
