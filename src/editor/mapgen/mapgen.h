#ifndef CATA_SRC_EDITOR_MAPGEN_H
#define CATA_SRC_EDITOR_MAPGEN_H

#include "defs.h"

#include "coordinates.h"
#include "cuboid_rectangle.h"
#include "game_constants.h"
#include "point.h"

#include "canvas_snippet.h"
#include "common/canvas_2d.h"
#include "common/uuid.h"
#include "mapgen/mapobject.h"
#include "mapgen/setmap.h"
#include "mapgen/palette.h"
#include "selection_mask.h"
#include "widget/editable_id.h"

struct ImVec4;
class JsonOut;
class JSON_IN;
template<typename T> struct enum_traits;

namespace editor
{
struct State;

struct MapgenBase {
    MapgenBase() : canvas( point( SEEX * 2, SEEY * 2 ), MapKey() ) { }
    ~MapgenBase();

    Canvas2D<MapKey> canvas;
    UUID palette = UUID_INVALID;

    void remove_usages( const MapKey &uuid );

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

enum class OterMapgenFill {
    None,
    FillTer,
    PredecessorMapgen,
    FallbackPredecessorMapgen,
    _Num,
};

enum class LayeringRuleTrapFurn {
    Unspecified,
    Allow,
    Dismantle,
    Erase,
    _Num,
};

enum class LayeringRuleItems {
    Unspecified,
    Allow,
    Erase,
    _Num,
};

constexpr int DEFAULT_OTER_MAPGEN_WEIGHT = 1000;

struct MapgenOter {
    bool matrix_mode = true;
    std::vector<EID::OterType> om_terrain;
    Canvas2D<EID::OterType> om_terrain_matrix =
        Canvas2D<EID::OterType>( point( 1, 1 ), EID::OterType::NULL_ID() );
    int weight = DEFAULT_OTER_MAPGEN_WEIGHT;
    OterMapgenFill mapgen_base = OterMapgenFill::None;
    bool uses_rows = true;
    EID::Ter fill_ter = EID::Ter::NULL_ID();
    EID::OterType predecessor_mapgen;
    IntRange rotation;

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

struct MapgenUpdate {
    std::string update_mapgen_id;
    EID::Ter fill_ter = EID::Ter::NULL_ID(); // FIXME: is this a thing? Does it work?
    bool uses_rows = true;

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

struct MapgenNested {
    std::string imported_mapgen_id;
    std::string nested_mapgen_id;
    point size = point( 24, 24 );
    IntRange rotation;

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

enum class MapgenType {
    Oter,
    Update,
    Nested,
    _Num,
};

struct MapgenFlags {
    bool avoid_creatures = false;
    bool no_underlying_rotate = false;
    LayeringRuleItems rule_items = LayeringRuleItems::Unspecified;
    LayeringRuleTrapFurn rule_furn = LayeringRuleTrapFurn::Unspecified;
    LayeringRuleTrapFurn rule_traps = LayeringRuleTrapFurn::Unspecified;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonObject& jsin);
};

struct Mapgen {
    public:
        Mapgen() = default;
        Mapgen( const Mapgen & ) = default;
        Mapgen( Mapgen && ) = default;
        Mapgen &operator=( const Mapgen & ) = default;
        Mapgen &operator=( Mapgen && ) = default;

        UUID uuid = UUID_INVALID;

        MapgenType mtype = MapgenType::Oter;
        MapgenBase base;
        MapgenOter oter;
        MapgenUpdate update;
        MapgenNested nested;

        std::vector<MapObject> objects;
        std::vector<SetMap> setmaps;
        MapgenFlags flags;

        std::string name;

        std::string display_name() const;

        void serialize( JsonOut &jsout ) const;
        void deserialize( const TextJsonValue &jsin );

        inline bool uses_rows() const {
            return mtype == editor::MapgenType::Nested ||
                   (
                       mtype == editor::MapgenType::Oter &&
                       oter.uses_rows
                   ) ||
                    (
                        mtype == editor::MapgenType::Update &&
                        update.uses_rows
                    );
        }

        point_rel_etile mapgensize() const;

        inline half_open_rectangle<point_abs_etile> get_bounds() const {
            return {
                point_abs_etile( 0, 0 ),
                point_abs_etile( mapgensize().raw() )
            };
        }

        void set_canvas_size( point new_size );

        SelectionMask *get_selection_mask();
        void erase_selected( const SelectionMask &mask );
        void apply_snippet( const CanvasSnippet &snippet );
        void select_from_snippet( const CanvasSnippet &snippet );

        const MapObject* get_object(UUID uuid) const;
        MapObject* get_object(UUID uuid) {
            const Mapgen* this_c = this;
            return const_cast<MapObject*>(this_c->get_object(uuid));
        }

    private:
        SelectionMask selection_mask;
};

/**
 * =============== Windows ===============
 */
void show_mapgen_info( State &state, Mapgen &mapgen, bool &show );

} // namespace editor

template<>
struct enum_traits<editor::OterMapgenFill> {
    static constexpr editor::OterMapgenFill last = editor::OterMapgenFill::_Num;
};

template<>
struct enum_traits<editor::LayeringRuleItems> {
    static constexpr editor::LayeringRuleItems last = editor::LayeringRuleItems::_Num;
};

template<>
struct enum_traits<editor::LayeringRuleTrapFurn> {
    static constexpr editor::LayeringRuleTrapFurn last = editor::LayeringRuleTrapFurn::_Num;
};

template<>
struct enum_traits<editor::MapgenType> {
    static constexpr editor::MapgenType last = editor::MapgenType::_Num;
};

#endif // CATA_SRC_EDITOR_MAPGEN_H
