#ifndef CATA_SRC_EDITOR_EDITOR_ENGINE_H
#define CATA_SRC_EDITOR_EDITOR_ENGINE_H

#include <SDL2/SDL.h>

#include <string>

namespace editor
{
void set_default_ini_path( bool flush = true );
void set_project_ini_path( const std::string &project_uuid, bool flush = true );
void flush_ini_to_disk();
bool init_ui( SDL_Window &window_ref, SDL_Renderer &renderer_ref );
void shutdown_ui();
void render_ui();
bool process_event( SDL_Event &event );
bool show_cata_ui();
bool ui_exists();

void bnmt_entry_point();
}

#endif // CATA_SRC_EDITOR_EDITOR_ENGINE_H
