#ifndef CATA_SRC_EDITOR_TITLE_SCREEN_H
#define CATA_SRC_EDITOR_TITLE_SCREEN_H

#include "options.h"

#include <imgui/imgui.h>

namespace editor
{

struct TitleScreenReturn {
    bool exit = false;
    bool exit_to_desktop = false;
    bool make_new = false;
    bool load_existing = false;
    std::string load_path;
};

struct TitleScreen {
    TitleScreen() = default;
    TitleScreen( const TitleScreen & ) = delete;
    TitleScreen( TitleScreen && ) = default;
    ~TitleScreen() = default;

    TitleScreen &operator=( const TitleScreen & ) = delete;
    TitleScreen &operator=( TitleScreen && ) = default;

    bool open_project_dialog = false;
    std::optional<TitleScreenReturn> ret;
    std::optional<std::string> popup_prompt;
};

/**
 * ============= Entry point =============
 */
void show_title_screen( TitleScreen &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_TITLE_SCREEN_H
