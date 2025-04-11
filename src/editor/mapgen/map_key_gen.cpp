#include "map_key_gen.h"

#include "text_snippets.h"
#include "catacharset.h"

#include "mapgen_map_key.h"

namespace editor
{
MapKey::MapKey(const map_key&key) : MapKey(key.str) {
    
}

MapKey::MapKey(const std::string_view&str) {
    auto s = utf8_to_utf32(str);
    if (s.length() != 1) {
        // FIXME: handle multi-code-point keys. Or better, get rid of them.
        std::abort();
    }
    value = s[0];
}

std::string MapKey::str() const
{
    return utf32_to_utf8(value);
}

MapKeyGenerator::MapKeyGenerator()
{
    const translation &trans = SNIPPET.get_snippet_ref_by_id( snippet_id( "me_auto_map_keys" ) );
    std::u32string s_u32 = utf8_to_utf32( trans.raw );
    // TODO: support combining characters
    opts.reserve( s_u32.size() );
    for( const char32_t &ch32 : s_u32 ) {
        opts.emplace_back( utf32_to_utf8( ch32 ) );
    }
}

void MapKeyGenerator::blacklist( const MapKey &opt )
{
    opts.erase( std::remove(opts.begin(), opts.end(), opt), opts.end() );
}

} // namespace editor
