#include "uuid.h"
#include "int_range.h"

#include "json.h"
#include "../../mapgen.h"

namespace editor
{

IntRange::IntRange( const jmapgen_int &range )
{
    min = range.val;
    max = range.valmax;
}

int IntRange::roll() const {
    return jmapgen_int(min, max).get();
}

void UUIDGenerator::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "counter", counter );
    jsout.end_object();
}

void UUIDGenerator::deserialize(const TextJsonValue& jsin)
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "counter", counter );
}

void IntRange::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "min", min );
    jsout.member( "max", max );
    jsout.end_object();
}

void IntRange::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "min", min );
    jo.read( "max", max );
}

} // namespace editor
