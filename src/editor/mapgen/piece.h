#ifndef CATA_SRC_EDITOR_PIECE_H
#define CATA_SRC_EDITOR_PIECE_H

#include "defs.h"

#include "calendar.h"

// FIXME: conflicts in include paths
#include "editor/mapgen/piece_type.h"
#include "editor/widget/editable_id.h"
#include "editor/common/int_range.h"
#include "editor/common/uuid.h"

#include <memory>
#include <string>

class JsonOut;
class JSON_OBJECT;
class jmapgen_piece;

#define IMPLEMENT_ME_PIECE(piece_class, piece_type)                     \
    piece_class() = default;                                            \
    piece_class( const piece_class& ) = default;                        \
    piece_class( piece_class&&) = default;                              \
    ~piece_class() = default;                                           \
    PieceType get_type() const override {                               \
        return piece_type;                                              \
    }                                                                   \
    std::unique_ptr<Piece> clone() const override {                     \
        return std::make_unique<piece_class>( *this );                  \
    };                                                                  \
    void serialize( JsonOut &jsout ) const override;                    \
    void deserialize( const TextJsonObject &jsin ) override;            \
    void export_func( JsonOut& jo ) const override;                     \
    bool try_import( const jmapgen_piece& piece, PaletteImportReport& report ) override; \
    void show_ui( State& state ) override;                              \
    std::string fmt_data_summary() const override;

namespace editor
{
struct State;
struct PaletteImportReport;

struct Piece {
    Piece() = default;
    virtual ~Piece() = default;

    virtual PieceType get_type() const = 0;

    UUID uuid = UUID_INVALID;

    virtual std::unique_ptr<Piece> clone() const = 0;

    virtual void serialize( JsonOut &jsout ) const = 0;
    virtual void deserialize( const TextJsonObject &jsin ) = 0;
    virtual void export_func( JsonOut &jo ) const = 0;
    virtual bool try_import( const jmapgen_piece &piece, PaletteImportReport& report) = 0;

    virtual void show_ui( State &state ) = 0;

    virtual void init_new() {};

    /**
     * Returns "Type: data" summary string
    */
    std::string fmt_summary() const;

    /**
     * Returns data summary string;
    */
    virtual std::string fmt_data_summary() const = 0;
};

const std::vector<std::unique_ptr<Piece>> &get_piece_templates();
std::unique_ptr<Piece> make_new_piece( PieceType pt );

bool is_alt_piece( PieceType pt );
bool is_piece_exclusive( PieceType pt );
bool is_available_as_mapping( PieceType pt );
bool is_available_as_mapobject( PieceType pt );

} // namespace editor

#endif // CATA_SRC_EDITOR_PIECE_H
