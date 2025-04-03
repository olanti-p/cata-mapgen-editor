#ifndef CATA_SRC_EDITOR_APP_H
#define CATA_SRC_EDITOR_APP_H

#include <memory>

namespace editor
{
struct TitleScreen;
struct State;

struct AppRunState {
    bool do_exit_to_game = false;
    bool do_exit_to_dektop = false;
};

struct App {
    App();
    App( const App & ) = delete;
    App( App && );
    ~App();

    App &operator=( const App & ) = delete;
    App &operator=( App && );

    std::unique_ptr<TitleScreen> title_state;
    std::unique_ptr<State> editor_state;

    AppRunState run_state;
};

/**
 * =============== Helpers ===============
 */
void init_app( App &app );
void update_app_state( App &app );

/**
 * =============== Windows ===============
 */
void show_app( App &app );


} // namespace editor

#endif // CATA_SRC_EDITOR_APP_H
