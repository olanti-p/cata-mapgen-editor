#include "loot_designer.h"

#include "state/ui_state.h"
#include "widget/editable_id.h"
#include "mapgen/palette.h"
#include "mapgen/palette_view.h"
#include "mapgen/piece_impl.h"
#include "state/state.h"
#include "project/project.h"
#include "widget/widgets.h"

#include "cata_imgui.h"
#include "output.h"

namespace editor
{

static void roll_one( std::vector<item>& result, const Piece* piece, const MapObject* object_info ) {
    // TODO: custom spawn rates (general and per category)
    const float spawnrate = 1.0f;
    // TODO: custom spawn time point
    time_point turn = calendar::start_of_game;

    IntRange repeat_from_piece = piece->get_repeat();
    IntRange repeat_from_object = object_info ? object_info->repeat : IntRange(1, 1);
    int repeats_final = std::max(repeat_from_piece.roll(), repeat_from_object.roll());

    for (int i = 0; i < repeats_final; i++) {
        piece->roll_loot(result, turn, spawnrate);
    }
}

static std::string roll_all( std::vector<const Piece*> pieces, std::vector<const MapObject*> objects, int n_rolls )
{
    std::vector<item> result;
    for (const auto& piece : pieces) {
        for (int roll = 0; roll < n_rolls; roll++) {
            roll_one(result, piece, nullptr);
        }
    }
    for (const auto& object : objects) {
        for (int roll = 0; roll < n_rolls; roll++) {
            roll_one(result, object->piece.get(), object);
        }
    }

    std::unordered_map<std::string, int> mapped;
    for (const item& itm : result) {
        //std::string disp = remove_color_tags( itm.display_name() );
        std::string disp = itm.display_name();
        auto it = mapped.find(disp);
        if (it == mapped.end()) {
            mapped[disp] = 1;
        }
        else {
            it->second++;
        }
    }

    std::vector<std::pair<std::string, int>> listed;
    listed.insert(listed.begin(), mapped.begin(), mapped.end());
    std::stable_sort(listed.begin(), listed.end(), [](const auto& a, const auto& b) {
        if (a.second > b.second) {
            return true;
        }
        else if (a.second == b.second) {
            return a.first > b.first;
        }
        else {
            return false;
        }
    });

    std::string ret;
    //ret = string_format("%d rolls of: %d pieces, %d objects\n\n", n_rolls, pieces.size(), objects.size());
    for (const auto& it : listed) {
        ret += string_format("%d %s\n", it.second, it.first );
    }
    return ret;
}

void show_loot_designer(State& state, detail::OpenLootDesigner& instance)
{
    if (!instance.open) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800.0f, 800.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

    std::string window_title = "Loot Designer##" + instance.make_window_id();
    if (!ImGui::Begin(window_title.c_str(), &instance.open)) {
        ImGui::End();
        return;
    }

    Palette* palette = nullptr; 
    PaletteEntry* entry = nullptr;  
    ViewPalette view_palette(state.project());
    ViewEntry* view_entry = nullptr;
    Mapgen* mapgen = nullptr;
    MapObject* mapobject = nullptr;

    if (instance.is_mapping_mode) {
        palette = state.project().get_palette(instance.palette);
        if (!palette) {
            instance.open = false;
        }
        else {
            if (instance.resolved) {
                // Resolved palette mode
                view_palette.add_palette_recursive(*palette, state.ui->view_palette_tree_states[palette->uuid]);
                // FIXME: optimize repeated view creation
                view_palette.finalize();
                view_entry = view_palette.find_entry(instance.map_key);
                if (!view_entry) {
                    instance.open = false;
                }
                else {
                    ImGui::Text("Resolved mappings for key \"%s\" in palette \"%s\"", instance.map_key.str().c_str(), palette->display_name().c_str());
                }
            }
            else {
                // Source palette mode
                entry = palette->find_entry(instance.map_key);
                if (!entry) {
                    instance.open = false;
                }
                else {
                    ImGui::Text("Source mappings for key \"%s\" in palette \"%s\"", instance.map_key.str().c_str(), palette->display_name().c_str());
                }
            }
        }
    }
    else {
        // Mapgen object mode
        mapgen = state.project().get_mapgen(instance.mapgen);
        if (!mapgen) {
            instance.open = false;
        }
        else {
            mapobject = mapgen->get_object(instance.mapgen_object);
            if (!mapobject) {
                instance.open = false;
            }
            else {
                ImGui::Text("Placement '%s' in mapgen %s", mapobject->piece->fmt_summary().c_str(), mapgen->display_name().c_str());
            }
        }
    }

    if (!instance.open) {
        // Couldn't find data
        ImGui::End();
        return;
    }

    ImGuiChildFlags child_flags = ImGuiChildFlags_Borders;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    {
        ImGui::BeginChild("ChildLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0.0f), child_flags | ImGuiChildFlags_ResizeX, window_flags);

        int num_entries_visible = 0;

        if (mapobject) {
            ImGui::PushID("the-object");
            // FIXME: this dupes object entry rendering, undupe it
            ImGui::Text("%s", mapobject->piece->fmt_summary().c_str());
            if (ImGui::InputIntRange("x", mapobject->x)) {
                state.mark_changed("me-mapobject-x-input");
            }
            ImGui::SameLine();
            if (ImGui::InputIntRange("y", mapobject->y)) {
                state.mark_changed("me-mapobject-y-input");
            }
            ImGui::SameLine();
            if (ImGui::InputIntRange("repeat", mapobject->repeat)) {
                state.mark_changed("me-mapobject-repeat-input");
            }
            ImGui::PushID("piece");
            mapobject->piece->show_ui(state);
            ImGui::PopID();
            ImGui::Separator();
            ImGui::PopID();
            num_entries_visible++;
        }

        if (entry) {
            for (size_t idx = 0; idx < entry->pieces.size(); idx++) {
                // FIXME: this dupes piece rendering, undupe it
                Piece& piece = *entry->pieces[idx].get();
                if (!is_used_by_loot_designer(piece.get_type())) {
                    continue;
                }
                ImGui::PushID(idx);

                if (!instance.enabled_pieces.count(piece.uuid)) {
                    instance.enabled_pieces[piece.uuid] = true;
                }

                ImVec2 cp = ImGui::GetCursorPos();
                ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 20);
                ImGui::Checkbox("##toggle-loot-designer", &instance.enabled_pieces[piece.uuid]);

                ImGui::HelpPopup("Toggle this piece for participating in loot designer rolls.");
                ImGui::SetCursorPos(cp);

                ImGui::BeginDisabled(!instance.enabled_pieces[piece.uuid]);
                ImGui::Text("%s", piece.fmt_summary().c_str());
                piece.show_ui(state);
                ImGui::EndDisabled();
                ImGui::PopID();
                ImGui::Separator();
                num_entries_visible++;
            }

            // TODO: Quick add buttons
        }

        if (view_entry) {
            for (size_t idx = 0; idx < view_entry->pieces.size(); idx++) {
                // FIXME: this dupes piece rendering, undupe it
                Piece& piece = *view_entry->pieces[idx].piece;
                if (!is_used_by_loot_designer(piece.get_type())) {
                    continue;
                }
                ImGui::PushID(idx);

                if (!instance.enabled_pieces.count(piece.uuid)) {
                    instance.enabled_pieces[piece.uuid] = true;
                }

                ImVec2 cp = ImGui::GetCursorPos();
                ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 20);
                ImGui::Checkbox("##toggle-loot-designer", &instance.enabled_pieces[piece.uuid]);
                ImGui::HelpPopup("Toggle this piece for participating in loot designer rolls.");
                ImGui::SetCursorPos(cp);

                ImGui::BeginDisabled(!instance.enabled_pieces[piece.uuid]);
                ImGui::Text("%s", piece.fmt_summary().c_str());
                piece.show_ui(state);
                ImGui::EndDisabled();
                ImGui::PopID();
                ImGui::Separator();
                num_entries_visible++;
            }
        }

        if (num_entries_visible == 0) {
            ImGui::Text( "No loot-generating data found." );
        }

        ImGui::EndChild();
    }
    ImGui::SameLine();
    {
        ImGui::BeginChild("ChildRight", ImVec2(0.0f, 0.0f), child_flags, window_flags);
        int n_rolls = 0;
        if (ImGui::Button("Roll 1")) {
            n_rolls = 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Roll 10")) {
            n_rolls = 10;
        }
        ImGui::SameLine();
        if (ImGui::Button("Roll 100")) {
            n_rolls = 100;
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy to clipboard")) {
            ImGui::SetClipboardText(remove_color_tags( instance.get_display_cache() ).c_str());
            instance.has_been_copied = true;
        }
        if (instance.has_been_copied) {
            ImGui::SameLine();
            ImGui::Text("Copied!");
        }
        if (n_rolls != 0) {
            std::vector<const Piece*> pieces;
            std::vector<const MapObject*> objects;
            if (entry) {
                for (const auto& it : entry->pieces) {
                    if (
                        is_used_by_loot_designer(it->get_type()) &&
                        instance.enabled_pieces[it->uuid]
                        )
                    {
                        pieces.push_back(it.get());
                    }
                }
            }
            if (view_entry) {
                for (const auto& it : view_entry->pieces) {
                    if (
                        is_used_by_loot_designer(it.piece->get_type()) &&
                        instance.enabled_pieces[it.piece->uuid]
                        )
                    {
                        pieces.push_back(it.piece);
                    }
                }
            }
            if (mapobject) {
                objects.push_back(mapobject);
            }
            instance.set_display_cache(roll_all(pieces, objects, n_rolls));
        }

        const std::string& display_cache = instance.get_display_cache();
        if (display_cache.empty()) {
            ImGui::BeginDisabled();
            ImGui::Text("<empty>");
            ImGui::EndDisabled();
        }
        else {
            cataimgui::draw_colored_text(display_cache);
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

} // namespace editor
