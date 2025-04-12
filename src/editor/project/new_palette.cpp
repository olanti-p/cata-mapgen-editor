#include "new_palette.h"

#include "common/uuid.h"
#include <imgui/imgui.h>
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

    ImGui::Checkbox("Use temp mapgen palettes", &palette.import_temp);

    if (palette.import_temp) {
        ImGui::InputId( "Source", palette.import_temp_from );
    }
    else {
        ImGui::InputId( "Source", palette.import_from );
    }

    bool input_ok = palette.import_temp ? palette.import_temp_from.is_valid() : palette.import_from.is_valid();

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
        if (palette.import_temp) {
            quick_import_temp_palette( state, palette.import_temp_from );
        }
        else {
            quick_import_palette( state, palette.import_from );
        }
        state.mark_changed();
        return false;
    }
    return true;
}

} // namespace editor
