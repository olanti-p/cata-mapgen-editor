#ifndef CATA_SRC_EDITOR_MATH_H
#define CATA_SRC_EDITOR_MATH_H

#include "point.h"

#include <vector>

inline int divide_wrapping( int v, int m )
{
    if( v >= 0 ) {
        return v / m;
    }
    return ( v - m + 1 ) / m;
}

inline int divide_wrapping( int v, int m, int &r )
{
    const int result = divide_wrapping( v, m );
    r = v - result * m;
    return result;
}

inline int wrap_index( int idx, int size )
{
    int ret = idx;
    divide_wrapping( idx, size, ret );
    return ret;
}

#endif // CATA_SRC_EDITOR_MATH_H
