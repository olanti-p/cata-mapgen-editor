#include "new_palette.h"

#include "common/uuid.h"
#include "imgui.h"
#include "mapgen/palette_making.h"
#include "mapgen/palette.h"
#include "project.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"

#include <chrono>
#include <cstddef>

namespace editor
{
bool show_new_palette_window( State &state, NewPaletteState &palette )
{
    bool keep_open = true;
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Appearing);
    ImGui::Begin( "Import Palette", &keep_open );
    if( !keep_open ) {
        palette.cancelled = true;
    }

    ImGui::InputId( "Source", palette.import_from );

    bool input_ok = palette.import_from.is_valid();

    ImGui::BeginDisabled( !input_ok );
    if( ImGui::Button( "Confirm" ) ) {
        palette.confirmed = true;
    }
    ImGui::EndDisabled();

    ImGui::End();

    if( palette.cancelled ) {
        return false;
    }
    if( palette.confirmed ) {
        quick_import_palette( state, palette.import_from );
        state.mark_changed();
        return false;
    }
    return true;
}

} // namespace editor
