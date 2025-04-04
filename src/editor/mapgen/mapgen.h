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
    MapgenBase() : canvas( point( SEEX * 2, SEEY * 2 ), UUID_INVALID ) { }
    ~MapgenBase();

    Canvas2D<UUID> canvas;
    UUID palette = UUID_INVALID;

    void remove_usages( const UUID &uuid );

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

enum class OterMapgenBase {
    FillTer,
    PredecessorMapgen,
    Rows,
    _Num,
};

struct MapgenOter {
    bool matrix_mode = true;
    std::vector<EID::OterType> om_terrain;
    Canvas2D<EID::OterType> om_terrain_matrix =
        Canvas2D<EID::OterType>( point( 1, 1 ), EID::OterType::NULL_ID() );
    int weight = 100;
    OterMapgenBase mapgen_base = OterMapgenBase::FillTer;
    EID::Ter fill_ter = EID::Ter::NULL_ID();
    EID::OterType predecessor_mapgen;
    IntRange rotation;

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

struct MapgenUpdate {
    std::string update_mapgen_id;
    EID::Ter fill_ter = EID::Ter::NULL_ID();

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

struct MapgenNested {
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

        std::string name;

        std::string display_name() const;

        void serialize( JsonOut &jsout ) const;
        void deserialize( const TextJsonValue &jsin );

        inline bool uses_rows() const {
            return mtype == editor::MapgenType::Nested ||
                   (
                       mtype == editor::MapgenType::Oter &&
                       oter.mapgen_base == editor::OterMapgenBase::Rows
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

    private:
        SelectionMask selection_mask;
};

/**
 * =============== Windows ===============
 */
void show_mapgen_info( State &state, Mapgen &mapgen, bool &show );

} // namespace editor

template<>
struct enum_traits<editor::OterMapgenBase> {
    static constexpr editor::OterMapgenBase last = editor::OterMapgenBase::_Num;
};

template<>
struct enum_traits<editor::MapgenType> {
    static constexpr editor::MapgenType last = editor::MapgenType::_Num;
};

#endif // CATA_SRC_EDITOR_MAPGEN_H
