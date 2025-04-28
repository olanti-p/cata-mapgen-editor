#include "setmap_impl.h"

#include "widget/widgets.h"
#include "state/state.h"

namespace editor
{

void SetMapTer::show_ui( State& state) {
    ImGui::HelpMarkerInline("Terrain type to spawn.");
    if (ImGui::InputId("terrain", id)) {
        state.mark_changed("id");
    }
}

std::string SetMapTer::fmt_data_summary() const {
    return id.data;
}

void SetMapFurn::show_ui(State& state) {
    ImGui::HelpMarkerInline("Furniture type to spawn.");
    if (ImGui::InputId("furniture", id)) {
        state.mark_changed("id");
    }
}

std::string SetMapFurn::fmt_data_summary() const {
    return id.data;
}

void SetMapTrap::show_ui(State& state) {
    ImGui::HelpMarkerInline("Trap type to spawn.");
    if (ImGui::InputId("trap", id)) {
        state.mark_changed("id");
    }
}

std::string SetMapTrap::fmt_data_summary() const {
    return id.data;
}

void SetMapVariable::show_ui(State& state) {
    ImGui::HelpMarkerInline("Variable id.");
    if (ImGui::InputText("id", &id)) {
        state.mark_changed("id");
    }
}

std::string SetMapVariable::fmt_data_summary() const {
    return id;
}

void SetMapBash::show_ui(State& state) {
    // No data
}

std::string SetMapBash::fmt_data_summary() const {
    return "<bash>";
}

void SetMapBurn::show_ui(State& state) {
    // No data
}

std::string SetMapBurn::fmt_data_summary() const {
    return "<burn>";
}

void SetMapRadiation::show_ui(State& state) {
    ImGui::HelpMarkerInline("Amount of radiation to spawn.");
    if (ImGui::InputIntRange("amount", amount)) {
        state.mark_changed("amount");
    }
}

std::string SetMapRadiation::fmt_data_summary() const {
    if (amount.min == amount.max) {
        return string_format("%d", amount.min);
    }
    else {
        return string_format("[%d,%d]", amount.min, amount.max);
    }
}

void SetMapRemoveTrap::show_ui(State& state) {
    ImGui::HelpMarkerInline("Trap type to remove.");
    if (ImGui::InputId("furniture", id)) {
        state.mark_changed("id");
    }
}

std::string SetMapRemoveTrap::fmt_data_summary() const {
    return id.data;
}

void SetMapRemoveCreature::show_ui(State& state) {
    // No data
}

std::string SetMapRemoveCreature::fmt_data_summary() const {
    return "<any>";
}

void SetMapRemoveItem::show_ui(State& state) {
    // No data
}

std::string SetMapRemoveItem::fmt_data_summary() const {
    return "<any>";
}

void SetMapRemoveField::show_ui(State& state) {
    // No data
}

std::string SetMapRemoveField::fmt_data_summary() const {
    return "<any>";
}

} // namespace editor
