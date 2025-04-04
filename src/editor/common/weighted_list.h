#ifndef CATA_SRC_EDITOR_WEIGHTED_LIST_H
#define CATA_SRC_EDITOR_WEIGHTED_LIST_H

#include "defs.h"

class JsonOut;
class JSON_IN;

namespace editor
{
namespace detail
{

template<typename T>
struct WeightedListEntry {
    T val;
    int weight = 0;

    WeightedListEntry() = default;
    WeightedListEntry( T val, int weight ) : val( val ), weight( weight ) {}
};

} // namespace detail

template<typename T>
struct WeightedList {
    std::vector<detail::WeightedListEntry<T>> entries;

    bool is_uniform() const {
        if( entries.empty() ) {
            return false;
        }
        int weight_zero = entries[0].weight;
        for( auto it = std::next( entries.cbegin() ); it != entries.cend(); it++ ) {
            if( it->weight != weight_zero ) {
                return false;
            }
        }
        return true;
    }
};

} // namespace editor

#endif // CATA_SRC_EDITOR_WEIGHTED_LIST_H
