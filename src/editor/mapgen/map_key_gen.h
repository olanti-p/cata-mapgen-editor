#ifndef CATA_SRC_EDITOR_MAP_KEY_GEN_H
#define CATA_SRC_EDITOR_MAP_KEY_GEN_H

#include "mapgen_map_key.h"

#include <vector>

namespace editor
{

struct MapKeyGenerator {
    private:
        std::vector<map_key> opts;

    public:
        MapKeyGenerator();
        ~MapKeyGenerator() = default;

        void blacklist( const map_key &opt );

        inline map_key operator()() {
            if( opts.empty() ) {
                return default_map_key;
            } else {
                return opts[0];
            }
        }
};

} // namespace editor

#endif // CATA_SRC_EDITOR_MAP_KEY_GEN_H
