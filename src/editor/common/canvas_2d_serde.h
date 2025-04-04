#ifndef CATA_SRC_EDITOR_2D_CANVAS_SERDE_H
#define CATA_SRC_EDITOR_2D_CANVAS_SERDE_H

#include "canvas_2d.h"
#include "json.h"

namespace editor
{

template<typename T>
void serialize( const editor::Canvas2D<T> &canvas, JsonOut &jsout )
{
    jsout.start_object();
    jsout.member( "size", canvas.get_size() );
    jsout.member( "data", canvas.get_data() );
    jsout.end_object();
}

template<typename T>
void deserialize( editor::Canvas2D<T> &list, const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    point size;
    jo.read( "size", size );
    std::vector<T> data;
    jo.read( "data", data );

    list.set_size( size );
    list.get_data() = std::move( data );
}

} // namespace editor

#endif // CATA_SRC_EDITOR_2D_CANVAS_SERDE_H
