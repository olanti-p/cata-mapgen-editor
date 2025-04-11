#ifndef CATA_SRC_EDITOR_MAP_KEY_GEN_H
#define CATA_SRC_EDITOR_MAP_KEY_GEN_H

#include "common/map_key.h"

#include <vector>

namespace editor
{

struct MapKeyGenerator {
    private:
        std::vector<MapKey> opts;

    public:
        MapKeyGenerator();
        ~MapKeyGenerator() = default;

        void blacklist( const MapKey &opt );

        inline MapKey operator()() {
            if( opts.empty() ) {
                // TODO: return from unicode ranges
                return default_editor_map_key;
            } else {
                return opts[0];
            }
        }
};

} // namespace editor

#endif // CATA_SRC_EDITOR_MAP_KEY_GEN_H
