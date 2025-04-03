#ifndef CATA_SRC_EDITOR_WELCOME_SCREEN_H
#define CATA_SRC_EDITOR_WELCOME_SCREEN_H

namespace editor
{

struct WelcomeResult {
    bool open_editor = false;
    bool quit_to_desktop = false;
    bool quit_to_game = false;
    bool edit_mods = false;
};

WelcomeResult show_welcome_screen();

} // namespace editor

#endif // CATA_SRC_EDITOR_WELCOME_SCREEN_H
