#ifndef CATA_SRC_EDITOR_UUID_H
#define CATA_SRC_EDITOR_UUID_H

#include <cstdint>

class JsonOut;
class JsonIn;

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
        void deserialize( JsonIn &jsin );
};

} // namespace editor

#endif // CATA_SRC_EDITOR_UUID_H
