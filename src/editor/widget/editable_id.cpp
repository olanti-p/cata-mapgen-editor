#include "editable_id.h"

#include "faction.h"
#include "field_type.h"
#include "game.h"
#include "item_factory.h"
#include "mapdata.h"
#include "mapgen.h"
#include "mongroup.h"
#include "mutation.h"
#include "npc.h"
#include "omdata.h"
#include "text_snippets.h"
#include "trap.h"
#include "vehicle_group.h"

namespace editor
{

template<>
const std::vector<std::string> &EditableID<field_type>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( field_types::get_all().size() );
        for( const field_type &it : field_types::get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<furn_t>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( furn_t::get_all().size() );
        for( const furn_t &it : furn_t::get_all().get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<item_group_tag>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( item_controller->get_all_group_names().size() );
        for( const item_group_id &it : item_controller->get_all_group_names() ) {
            all_opts.push_back( it.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string>& EditableID<import_mapgen_tag>::get_all_opts()
{
    if (all_opts.empty()) {
        all_opts.reserve(editor_mapgen_refs.size());
        for (const auto& it : editor_mapgen_refs) {
            all_opts.push_back(it.first);
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<itype>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( item_controller->all().size() );
        for( const itype *it : item_controller->all() ) {
            all_opts.push_back( it->id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<liquid_item_tag>::get_all_opts()
{
    if( all_opts.empty() ) {
        for( const itype *it : item_controller->all() ) {
            if( it->phase != phase_id::LIQUID ) {
                continue;
            }
            all_opts.push_back( it->id.str() );
        }
        all_opts.shrink_to_fit();
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<MonsterGroup>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( MonsterGroupManager::Get_all_Groups().size() );
        for( const auto &it : MonsterGroupManager::Get_all_Groups() ) {
            all_opts.push_back( it.first.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<npc_template>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( get_all_npc_templates().size() );
        for( const auto &it : get_all_npc_templates() ) {
            all_opts.push_back( it.first.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string>& EditableID<nested_mapgen>::get_all_opts()
{
    if (all_opts.empty()) {
        all_opts.reserve(nested_mapgens.size());
        for (const auto& it : nested_mapgens) {
            all_opts.push_back(it.first.str());
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<oter_t>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( overmap_terrains::get_all().size() );
        for( const oter_t &it : overmap_terrains::get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<oter_type_t>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( overmap_terrains::get_all_types().size() );
        for( const oter_type_t &it : overmap_terrains::get_all_types() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<mapgen_palette>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( mapgen_palette::get_all().size() );
        for( const auto &it : mapgen_palette::get_all() ) {
            all_opts.push_back( it.first.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string>& EditableID<temp_palette_tag>::get_all_opts()
{
    if (all_opts.empty()) {
        all_opts.reserve(get_temp_mapgen_palettes().size());
        for (const auto& it : get_temp_mapgen_palettes()) {
            all_opts.push_back(it.first);
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<snippet_category_tag>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( SNIPPET.snippets_by_category.size() );
        for( const auto &it : SNIPPET.snippets_by_category ) {
            all_opts.push_back( it.first );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<ter_t>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( ter_t::get_all().size() );
        for( const ter_t &it : ter_t::get_all().get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<mutation_branch>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( mutation_branch::get_all().size() );
        for( const mutation_branch &it : mutation_branch::get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<trap>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( trap::get_all().size() );
        for( const trap &it : trap::get_all() ) {
            all_opts.push_back( it.id.str() );
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string>& EditableID<update_mapgen>::get_all_opts()
{
    if (all_opts.empty()) {
        all_opts.reserve(update_mapgens.size());
        for (const auto& it : update_mapgens) {
            all_opts.push_back(it.first.str());
        }
    }
    return all_opts;
}

template<>
const std::vector<std::string> &EditableID<VehicleGroup>::get_all_opts()
{
    if( all_opts.empty() ) {
        all_opts.reserve( vgroups.size() );
        for( const auto &it : vgroups ) {
            all_opts.push_back( it.first.str() );
        }
    }
    return all_opts;
}

} // namespace editor

template<>
bool string_id<editor::snippet_category_tag>::is_valid() const
{
    return SNIPPET.snippets_by_category.count( this->str() ) > 0;
}

template<>
bool string_id<editor::liquid_item_tag>::is_valid() const
{
    return itype_id( this->str() ).is_valid();
}

template<>
bool string_id<editor::item_group_tag>::is_valid() const
{
    return item_controller->get_group( item_group_id( this->str() ) );
}

template<>
bool string_id<editor::temp_palette_tag>::is_valid() const
{
    return get_temp_mapgen_palettes().count(this->str()) > 0;
}

template<>
bool string_id<editor::import_mapgen_tag>::is_valid() const
{
    return editor_mapgen_refs.count(this->str()) > 0;
}
