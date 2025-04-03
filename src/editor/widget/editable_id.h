#ifndef CATA_SRC_EDITOR_EDITABLE_ID_H
#define CATA_SRC_EDITOR_EDITABLE_ID_H

#include "type_id.h"

#include <string>
#include <vector>

class JsonOut;
class JsonIn;
template<typename T> struct enum_traits;

class npc_template;

namespace editor
{

namespace detail
{
void serialize_eid( JsonOut &jsout, const std::string &data );
void deserialize_eid( JsonIn &jsin, std::string &data );
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

        bool is_valid() const {
            return string_id<T>( data ).is_valid();
        }

        bool is_null() const {
            return string_id<T>( data ).is_null();
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
        void deserialize( JsonIn &jsin ) {
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

// EditableID specified for types
namespace EID
{

using Field = EditableID<field_type>;
using Furn = EditableID<furn_t>;
using IGroup = EditableID<item_group_tag>;
using Item = EditableID<itype>;
using Liquid = EditableID<liquid_item_tag>;
using MGroup = EditableID<MonsterGroup>;
using NPCTemplate = EditableID<npc_template>;
using Oter = EditableID<oter_t>;
using OterType = EditableID<oter_type_t>;
using Palette = EditableID<mapgen_palette>;
using SnippetCategory = EditableID<snippet_category_tag>;
using Ter = EditableID<ter_t>;
using Trait = EditableID<mutation_branch>;
using Trap = EditableID<trap>;
using VGroup = EditableID<VehicleGroup>;

} // namespace EID

} // namespace editor

#endif // CATA_SRC_EDITOR_EDITABLE_ID_H
