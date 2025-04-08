#include "piece.h"
#include "piece_impl.h"

#define REG_PIECE( piece_class ) ret.push_back( std::make_unique<piece_class>() )

namespace editor
{

std::string Piece::fmt_summary() const
{
    return string_format(
        "%s%s: %s",
        constraint.has_value() ? "$ ": "",
        io::enum_to_string<PieceType>( get_type() ),
        fmt_data_summary()
    );
}

const std::vector<std::unique_ptr<Piece>> &get_piece_templates()
{
    static std::vector<std::unique_ptr<Piece>> ret;
    if( ret.empty() ) {
        ret.reserve( static_cast<int>( PieceType::NumJmTypes ) );
        REG_PIECE( PieceField );
        REG_PIECE( PieceNPC );
        REG_PIECE( PieceFaction );
        REG_PIECE( PieceSign );
        REG_PIECE( PieceGraffiti );
        REG_PIECE( PieceVendingMachine );
        REG_PIECE( PieceToilet );
        REG_PIECE( PieceGaspump );
        REG_PIECE( PieceLiquid );
        REG_PIECE( PieceIGroup );
        REG_PIECE( PieceLoot );
        REG_PIECE( PieceMGroup );
        REG_PIECE( PieceMonster );
        REG_PIECE( PieceVehicle );
        REG_PIECE( PieceItem );
        REG_PIECE( PieceTrap );
        REG_PIECE( PieceFurniture );
        REG_PIECE( PieceTerrain );
        REG_PIECE( PieceTerFurnTransform );
        REG_PIECE( PieceMakeRubble );
        REG_PIECE( PieceComputer );
        REG_PIECE( PieceSealeditem );
        REG_PIECE( PieceTranslate );
        REG_PIECE( PieceZone );
        REG_PIECE( PieceNested );
        REG_PIECE( PieceAltTrap );
        REG_PIECE( PieceAltFurniture );
        REG_PIECE( PieceAltTerrain );
        REG_PIECE( PieceRemoveAll );
        REG_PIECE( PieceUnknown );
    }
    return ret;
}

std::unique_ptr<Piece> make_new_piece( PieceType pt )
{
    for( const std::unique_ptr<Piece> &it : get_piece_templates() ) {
        if( it->get_type() == pt ) {
            return it->clone();
        }
    }
    std::cerr << "Failed to generate piece with type " << static_cast<int>( pt ) << std::endl;
    std::abort();
}

bool is_alt_piece( PieceType pt )
{
    return pt == editor::PieceType::AltTerrain ||
           pt == editor::PieceType::AltFurniture ||
           pt == editor::PieceType::AltTrap;
}

bool is_piece_exclusive( PieceType pt )
{
    return is_alt_piece( pt );
}

bool is_available_as_mapping( PieceType pt )
{
    return !(
               pt == editor::PieceType::Unknown ||
               pt == editor::PieceType::Terrain ||
               pt == editor::PieceType::Furniture ||
               pt == editor::PieceType::Trap ||
               pt == editor::PieceType::Loot
           );
}

bool is_available_as_mapobject( PieceType pt )
{
    return !(
               pt == editor::PieceType::RemoveAll ||
               pt == editor::PieceType::Unknown ||
               pt == editor::PieceType::AltTerrain ||
               pt == editor::PieceType::AltFurniture ||
               pt == editor::PieceType::AltTrap ||
               pt == editor::PieceType::SealedItem
           );
}

} // namespace editor
