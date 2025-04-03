#ifndef CATA_SRC_EDITOR_VIEW_CANVAS_H
#define CATA_SRC_EDITOR_VIEW_CANVAS_H

namespace editor
{
struct State;
struct Mapgen;

/**
 * =============== Windows ===============
 */
void show_editor_view( State &state, Mapgen *mapgen_ptr );
void handle_view_change( State &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_VIEW_CANVAS_H