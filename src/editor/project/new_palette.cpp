#include "new_palette.h"

#include "common/uuid.h"
#include "imgui.h"
#include "mapgen/palette_making.h"
#include "mapgen/palette.h"
#include "project.h"
#include "state/control_state.h"
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
    ImGui::Begin( "New Palette", &keep_open );
    if( !keep_open ) {
        palette.cancelled = true;
    }

    ImGui::InputText( "Name", &palette.name );
    ImGui::HelpPopup( "Display name.  Has no effect, just for convenience." );

    if( ImGui::RadioButton( "Create New", palette.kind == NewPaletteKind::BrandNew ) ) {
        palette.kind = NewPaletteKind::BrandNew;
    }
    if( ImGui::RadioButton( "Import", palette.kind == NewPaletteKind::Imported ) ) {
        palette.kind = NewPaletteKind::Imported;
    }

    if( palette.kind == NewPaletteKind::Imported ) {
        ImGui::InputId( "Source", palette.import_from );
    } else {
        ImGui::Text( "-- TODO: inheriting palettes --" );
        /*
        ImGui::Checkbox( "Create inheriting palette", &palette.inherits );
        ImGui::BeginDisabled( !palette.inherits );
        ImGui::PaletteSelector( "Inherit from", palette.inherits_from, state.project().palettes );
        ImGui::EndDisabled();
        */
    }

    bool input_ok = true;
    if( palette.kind == NewPaletteKind::BrandNew && palette.inherits &&
        !state.project().get_palette( palette.inherits_from ) ) {
        input_ok = false;
    }
    if( palette.kind == NewPaletteKind::Imported && !palette.import_from.is_valid() ) {
        input_ok = false;
    }

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
        add_palette( state, palette );
        state.mark_changed();
        return false;
    }
    return true;
}

void add_palette( State &state, NewPaletteState &palette )
{
    Project &project = state.project();
    UUID new_palette_uuid = project.uuid_generator();
    project.palettes.emplace_back();
    Palette &new_palette = project.palettes.back();
    new_palette.uuid = new_palette_uuid;
    new_palette.name = palette.name;

    if( palette.inherits ) {
        new_palette.inherits_from = palette.inherits_from;
    }
    if( palette.kind == NewPaletteKind::Imported ) {
        PaletteImportReport rep = import_palette_data( state.project(), new_palette, palette.import_from );
        if( rep.num_failed != 0 ) {
            std::string text = string_format(
                                   "Palette has been imported only partially.\n\n"
                                   "%d out of %d mappings have been skipped.\n\n"
                                   "-- TODO: Implement import of all mappings. --",
                                   rep.num_failed, rep.num_total
                               );
            state.control->show_warning_popup( text );
        }
    }
}

} // namespace editor
