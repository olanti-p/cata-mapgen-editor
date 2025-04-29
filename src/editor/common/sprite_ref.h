#ifndef CATA_SRC_EDITOR_SPRITE_REF_H
#define CATA_SRC_EDITOR_SPRITE_REF_H

#include <string>
#include <utility>

#include <imgui/imgui.h>

struct SpriteRef {
    int tile_idx = -1;

    SpriteRef() = default;
    SpriteRef( SpriteRef && ) = default;
    SpriteRef( const SpriteRef & ) = default;

    SpriteRef( int tile_idx ) : tile_idx( tile_idx ) {};
    SpriteRef( const std::string &id );

    SpriteRef &operator=( const SpriteRef & ) = default;
    SpriteRef &operator=( SpriteRef && ) = default;

    constexpr operator bool() const {
        return tile_idx != -1;
    }

    constexpr bool operator==(const SpriteRef& rhs) const {
        return tile_idx == rhs.tile_idx;
    }
    constexpr bool operator!=(const SpriteRef& rhs) const {
        return tile_idx != rhs.tile_idx;
    }
    constexpr bool operator<(const SpriteRef& rhs) const {
        return tile_idx < rhs.tile_idx;
    }

    ~SpriteRef() = default;

    ImTextureID get_tex_id() const;
    std::pair<ImVec2, ImVec2> make_uvs() const;
};

#endif // CATA_SRC_EDITOR_SPRITE_REF_H
