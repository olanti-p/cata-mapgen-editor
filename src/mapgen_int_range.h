#pragma once
#ifndef CATA_SRC_MAPGEN_INT_RANGE_H
#define CATA_SRC_MAPGEN_INT_RANGE_H

#include <limits>
#include <cstdint>
#include <string_view>

#include "cata_assert.h"

class JsonObject;

/*
 * Actually a pair of integers that can rng, for numbers that will never exceed INT_MAX
 */
 struct jmapgen_int {
    int16_t val;
    int16_t valmax;
    explicit jmapgen_int( int v ) : val( v ), valmax( v ) {
        cata_assert( v <= std::numeric_limits<int16_t>::max() );
    }
    jmapgen_int( int v, int v2 ) : val( v ), valmax( v2 ) {
        cata_assert( v <= std::numeric_limits<int16_t>::max() );
        cata_assert( v2 <= std::numeric_limits<int16_t>::max() );
    }
    /**
     * Throws as usually if the json is invalid or missing.
     */
    jmapgen_int( const JsonObject &jo, std::string_view tag );
    /**
     * Throws if the json is malformed (e.g. a string not an integer, but does not throw
     * if the member is just missing (the default values are used instead).
     */
    jmapgen_int( const JsonObject &jo, std::string_view tag, const int &def_val,
                 const int &def_valmax );

    int get() const;
};

#endif // CATA_SRC_MAPGEN_INT_RANGE_H
