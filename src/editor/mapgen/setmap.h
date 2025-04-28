#ifndef CATA_SRC_EDITOR_SETMAP_H
#define CATA_SRC_EDITOR_SETMAP_H

#include "common/uuid.h"
#include "common/int_range.h"
#include "enum_traits.h"

#include "cuboid_rectangle.h"

#include <imgui/imgui.h>

class JsonOut;
class TextJsonObject;
struct jmapgen_setmap;

#define IMPLEMENT_SETMAP(the_class, setmap_type)                        \
    the_class() = default;                                              \
    the_class( const the_class& ) = default;                            \
    the_class( the_class&&) = default;                                  \
    ~the_class() = default;                                             \
    SetMapType get_type() const override {                              \
        return setmap_type;                                             \
    }                                                                   \
    std::unique_ptr<SetMapData> clone() const override {                \
        return std::make_unique<the_class>( *this );                    \
    };                                                                  \
    void serialize( JsonOut &jsout ) const override;                    \
    void deserialize( const TextJsonObject &jsin ) override;            \
    void export_func( JsonOut& jo ) const override;                     \
    bool try_import( const jmapgen_setmap& piece, PaletteImportReport& report ) override; \
    void show_ui( State& state ) override;                              \
    std::string fmt_data_summary() const override;

namespace editor
{
struct Mapgen;
struct State;
struct PaletteImportReport;

enum class SetMapType {
    Ter,
    Furn,
    Trap,
    Variable,
    Bash,
    Burn,
    Radiation,
    RemoveTrap,
    RemoveCreature,
    RemoveItem,
    RemoveField,

    _Num,
};

enum class SetMapMode {
    Point,
    Line,
    Square,

    _Num,
};

struct SetMapData {
    virtual SetMapType get_type() const = 0;
    virtual std::unique_ptr<SetMapData> clone() const = 0;

    virtual void serialize(JsonOut& jsout) const = 0;
    virtual void deserialize(const TextJsonObject& jsin) = 0;

    virtual void export_func(JsonOut& jo) const = 0;
    virtual bool try_import(const jmapgen_setmap& piece, PaletteImportReport& report) = 0;
    virtual void show_ui(State& state) = 0;
    virtual std::string fmt_data_summary() const = 0;
};

struct SetMap {
    SetMap();
    SetMap( const SetMap &rhs );
    SetMap( SetMap && );
    ~SetMap();

    SetMap &operator=( const SetMap &rhs );
    SetMap &operator=( SetMap && );

    UUID uuid = UUID_INVALID;
    IntRange x;
    IntRange y;
    IntRange x2;
    IntRange y2;
    IntRange z;
    int chance = 1;
    IntRange repeat = IntRange(1,1);
    ImVec4 color;
    bool visible = true;
    SetMapMode mode = SetMapMode::Point;
    std::unique_ptr<SetMapData> data;

    void show_ui(State& state);
    std::string fmt_summary() const;
    inclusive_rectangle<point> get_bounding_box() const;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonObject& jsin);
};

const std::vector<std::unique_ptr<SetMapData>>& get_setmap_data_templates();
std::unique_ptr<SetMapData> make_new_setmap_data(SetMapType pt);

/**
 * =============== Windows ===============
 */
void show_setmaps( State &state, Mapgen &f, bool is_active, bool &show );

} // namespace editor

template<>
struct enum_traits<editor::SetMapType> {
    static constexpr editor::SetMapType last = editor::SetMapType::_Num;
};

template<>
struct enum_traits<editor::SetMapMode> {
    static constexpr editor::SetMapMode last = editor::SetMapMode::_Num;
};

#endif // CATA_SRC_EDITOR_SETMAP_H
