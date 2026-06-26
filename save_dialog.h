#include <memory>
#include <vector>

namespace pfd { class save_file; }
extern std::unique_ptr<pfd::save_file> save_dialog;

void open_save_dialog();
void save_dialog_update(const std::vector<std::string>&, const std::vector<int>&);
