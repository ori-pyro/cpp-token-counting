#include <memory>
#include <map>

namespace pfd { class save_file; class select_folder; }
extern std::unique_ptr<pfd::save_file> save_dialog;
extern std::unique_ptr<pfd::select_folder> select_dialog;


void open_save_dialog();
void save_dialog_update(const std::string project_path,
                        const std::map<std::string, int>& types,
                        const std::map<std::string, std::map<std::string, int>>& sub_types);
void open_select_dialog();
void select_dialog_update(std::string& dir_path_str);
