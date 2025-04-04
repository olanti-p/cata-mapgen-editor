#ifndef CATA_SRC_EDITOR_UUID_H
#define CATA_SRC_EDITOR_UUID_H

#include <cstdint>

#include "defs.h"

class JsonOut;
class JSON_IN;

namespace editor
{

using UUID = uint64_t;
constexpr UUID UUID_INVALID = 0;

struct UUIDGenerator {
    private:
        UUID counter = UUID_INVALID;

    public:
        inline UUID operator()() {
            counter++;
            return counter;
        }

        void serialize( JsonOut &jsout ) const;
        void deserialize(const TextJsonValue& jsin);
};

} // namespace editor

#endif // CATA_SRC_EDITOR_UUID_H
