#ifndef CATA_SRC_EDITOR_SPRITE_REF_H
#define CATA_SRC_EDITOR_SPRITE_REF_H

#include <string>
#include <utility>

#include "imgui.h"

struct SpriteRef {
    int tile_idx = -1;

    SpriteRef() = default;
    SpriteRef( SpriteRef && ) = default;
    SpriteRef( const SpriteRef & ) = default;

    SpriteRef( int tile_idx ) : tile_idx( tile_idx ) {};
    SpriteRef( const std::string &id );

    SpriteRef &operator=( const SpriteRef & ) = default;
    SpriteRef &operator=( SpriteRef && ) = default;

    ~SpriteRef() = default;

    ImTextureID get_tex_id() const;
    std::pair<ImVec2, ImVec2> make_uvs() const;
};

#endif // CATA_SRC_EDITOR_SPRITE_REF_H
