#ifndef CATA_SRC_EDITOR_EDITABLE_ID_H
#define CATA_SRC_EDITOR_EDITABLE_ID_H

#include "defs.h"

#include "type_id.h"

#include <string>
#include <vector>

template<typename T> struct enum_traits;

class npc_template;

namespace editor
{

namespace detail
{
void serialize_eid( JsonOut &jsout, const std::string &data );
void deserialize_eid( const TextJsonValue &jsin, std::string &data );
} // namespace detail

template<typename T>
struct EditableID {
    public:
        std::string data;

        EditableID() = default;
        EditableID( const EditableID<T> & ) = default;
        EditableID( EditableID<T> && ) = default;
        EditableID( const std::string &s ) : data( s ) {}
        EditableID( const string_id<T> &id ) : data( id.str() ) {}
        ~EditableID() = default;

        EditableID &operator= ( const EditableID<T> & ) = default;
        EditableID &operator= ( EditableID<T> && ) = default;

        inline bool operator==( const EditableID<T> &rhs ) const {
            return data == rhs.data;
        }
        inline bool operator!=(const EditableID<T>& rhs) const {
            return data != rhs.data;
        }

        bool is_valid() const {
            return string_id<T>( data ).is_valid();
        }

        bool is_null() const {
            return string_id<T>( data ).is_null();
        }

        bool is_empty() const {
            return data.empty();
        }

        const T &obj() const {
            return string_id<T>( data ).obj();
        }

        static const EditableID<T> NULL_ID() {
            return string_id<T>::NULL_ID();
        }

        static const std::vector<std::string> &get_all_opts();

        void serialize( JsonOut &jsout ) const {
            detail::serialize_eid( jsout, data );
        }
        void deserialize( const TextJsonValue &jsin ) {
            detail::deserialize_eid( jsin, data );
        }

    private:
        // TODO: invalidate on data change
        static std::vector<std::string> all_opts;
};

template<typename T>
std::vector<std::string> EditableID<T>::all_opts;

struct snippet_category_tag {};
struct liquid_item_tag {};
struct item_group_tag {};
struct temp_palette_tag {};
struct import_mapgen_tag {};
struct signage_furn_tag {};

// EditableID specified for types
namespace EID
{

using Field = EditableID<field_type>;
using Furn = EditableID<furn_t>;
using IGroup = EditableID<item_group_tag>;
using ImportMapgen = EditableID<import_mapgen_tag>;
using Item = EditableID<itype>;
using Liquid = EditableID<liquid_item_tag>;
using MGroup = EditableID<MonsterGroup>;
using Monster = EditableID<mtype>;
using Nest = EditableID<nested_mapgen>;
using NPCTemplate = EditableID<npc_template>;
using Oter = EditableID<oter_t>;
using OterType = EditableID<oter_type_t>;
using Palette = EditableID<mapgen_palette>;
using SignageFurn = EditableID<signage_furn_tag>;
using SnippetCategory = EditableID<snippet_category_tag>;
using TempPalette = EditableID<temp_palette_tag>;
using Ter = EditableID<ter_t>;
using TerFurnTransform = EditableID<ter_furn_transform>;
using Trait = EditableID<mutation_branch>;
using Trap = EditableID<trap>;
using Update = EditableID<update_mapgen>;
using VGroup = EditableID<VehicleGroup>;
using Zone = EditableID<zone_type>;

} // namespace EID

} // namespace editor

#endif // CATA_SRC_EDITOR_EDITABLE_ID_H
