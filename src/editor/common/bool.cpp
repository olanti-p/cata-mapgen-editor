#include "bool.h"

#include "json.h"

namespace editor
{

void Bool::serialize( JsonOut &jsout ) const
{
    jsout.write_bool( value );
}

void Bool::deserialize(const TextJsonValue& jsin)
{
    value = jsin.get_bool();
}

} // namespace editor
