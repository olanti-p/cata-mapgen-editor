#include "welcome_screen.h"

#include "color.h"
#include "game.h"
#include "runtime/editor_engine.h"
#include "translations.h"
#include "uilist.h"
#include "options.h"
#include "ui_manager.h"

namespace editor
{

enum Command : int {
    OpenEditor,
    EditMods,
    GameOptions,
    QuitToGame,
    QuitToDesktop,
};

WelcomeResult show_welcome_screen()
{
    if( g->enter_editor_on_start ) {
        WelcomeResult res;
        res.open_editor = true;
        return res;
    }

    background_pane bg_pane;

    while( true ) {
        uilist menu;
        menu.border_color = c_blue;
        menu.hotkey_color = c_dark_gray;
        menu.text_color = c_white;
        menu.settext( _(
                          "+--=== B N M T ===--+\n"
                          "|   Bright Nights   |\n"
                          "|   Mapping  Tool   |\n"
                          "+-------------------+\n"
                      ) );
        menu.addentry( Command::OpenEditor, true, 'e', _( "Open Editor" ) );
        menu.addentry( Command::EditMods, true, 'm', _( "Specify Mod List" ) );
        menu.addentry( Command::GameOptions, true, 'o', _( "Game Options" ) );
        menu.addentry( Command::QuitToGame, true, 'g', _( "Quit To Game" ) );
        menu.addentry( Command::QuitToDesktop, true, 'q', _( "Quit To Desktop" ) );
        menu.query();

        switch( menu.ret ) {
            case Command::OpenEditor: {
                WelcomeResult res;
                res.open_editor = true;
                return res;
            }
            case Command::EditMods: {
                WelcomeResult res;
                res.edit_mods = true;
                return res;
            }
            case Command::GameOptions: {
                get_options().show( false );
                break;
            }
            case Command::QuitToGame: {
                WelcomeResult res;
                res.quit_to_game = true;
                return res;
            }
            case Command::QuitToDesktop: {
                WelcomeResult res;
                res.quit_to_desktop = true;
                return res;
            }
        }
    }

    WelcomeResult res;
    res.quit_to_desktop = true;
    return res;
}

} // namespace editor
