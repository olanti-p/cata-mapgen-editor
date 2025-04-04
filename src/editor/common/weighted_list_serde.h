#ifndef CATA_SRC_EDITOR_WEIGHTED_LIST_SERDE_H
#define CATA_SRC_EDITOR_WEIGHTED_LIST_SERDE_H

#include "json.h"
#include "weighted_list.h"

template<typename T>
void serialize( const editor::WeightedList<T> &list, JsonOut &jsout )
{
    jsout.start_array();
    for( const auto &e : list.entries ) {
        jsout.start_object();
        jsout.member( "w", e.weight );
        jsout.member( "v", e.val );
        jsout.end_object();
    }
    jsout.end_array();
}

template<typename T>
void deserialize( editor::WeightedList<T> &list, const TextJsonValue &jsin )
{
    TextJsonArray ja = jsin.get_array();
    for( JSON_OBJECT jo : ja ) {
        editor::detail::WeightedListEntry<T> e;
        jo.read( "w", e.weight );
        jo.read( "v", e.val );
        list.entries.push_back( std::move( e ) );
    }
}

#endif // CATA_SRC_EDITOR_WEIGHTED_LIST_SERDE_H
