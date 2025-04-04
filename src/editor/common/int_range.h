#ifndef CATA_SRC_EDITOR_INT_RANGE_H
#define CATA_SRC_EDITOR_INT_RANGE_H

#include "defs.h"

class JsonOut;
class JSON_IN;
struct jmapgen_int;

namespace editor
{

struct IntRange {
    int min = 0;
    int max = 0;

    IntRange() = default;
    IntRange( const jmapgen_int &range );
    ~IntRange() = default;

    inline operator bool() const {
        return min != 0 || max != 0;
    }

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );
};

} // namespace editor

#endif // CATA_SRC_EDITOR_INT_RANGE_H
