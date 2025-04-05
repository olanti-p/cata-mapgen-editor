#pragma once
#ifndef CATA_SRC_MAPGEN_MAP_KEY_H
#define CATA_SRC_MAPGEN_MAP_KEY_H

#include "defs.h"

#include <string>
#include <utility>

class JsonMember;
class TextJsonMember;

// Strong typedef for strings used as map/palette keys
// Each key should be a UTF-8 string displayed in only one column (i.e.
// utf8_width of 1) but can contain multiple Unicode code points.
class map_key
{
    public:
        map_key() = default;
        map_key( const map_key & ) = default;
        map_key( map_key && ) = default;
        explicit map_key( const std::string & );
        explicit map_key( const JsonMember & );
		explicit map_key(const TextJsonMember&);
        ~map_key() = default;

        map_key &operator= ( const map_key & ) = default;
        map_key &operator= ( map_key && ) = default;

        friend bool operator==( const map_key &l, const map_key &r ) {
            return l.str == r.str;
        }
        friend bool operator!=(const map_key& l, const map_key& r) {
            return l.str != r.str;
        }

        std::string str;
};

const map_key default_map_key( "." );

namespace std
{
template<>
struct hash<map_key> {
    size_t operator()( const map_key &k ) const noexcept {
        return hash<std::string> {}( k.str );
    }
};
} // namespace std

#endif // CATA_SRC_MAPGEN_MAP_KEY_H
