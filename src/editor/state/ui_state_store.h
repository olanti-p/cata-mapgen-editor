#ifndef CATA_SRC_EDITOR_UI_STATE_STORE_H
#define CATA_SRC_EDITOR_UI_STATE_STORE_H

#include <string>

namespace editor
{
struct UiState;

UiState &get_uistate_for_project( const std::string &project_uuid );

void initialize_settings_export();

} // namespace editor

#endif // CATA_SRC_EDITOR_UI_STATE_STORE_H
