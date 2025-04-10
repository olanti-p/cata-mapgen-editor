#include "map_key_gen.h"

#include "text_snippets.h"
#include "catacharset.h"

namespace editor
{

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

void MapKeyGenerator::blacklist( const map_key &opt )
{
    opts.erase( std::remove(opts.begin(), opts.end(), opt), opts.end() );
}

} // namespace editor
