#ifndef CATA_SRC_EDITOR_MAP_KEY_H
#define CATA_SRC_EDITOR_MAP_KEY_H

class map_key;
class JsonOut;
class TextJsonValue;

namespace editor
{

struct MapKey {
    uint32_t value = 0;

    MapKey() = default;
    explicit constexpr MapKey(uint32_t value_) : value(value_) {}
    explicit MapKey(const std::string_view&);
    explicit MapKey(const map_key&);
    MapKey(const MapKey&) = default;
    MapKey(MapKey&&) = default;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonValue& jsin);

    MapKey& operator= (const MapKey&) = default;
    MapKey& operator= (MapKey&&) = default;

    bool operator==(const MapKey& rhs) const {
        return value == rhs.value;
    }
    bool operator!=(const MapKey& rhs) const {
        return value != rhs.value;
    }

    operator bool() const {
        return value;
    }

    std::string str() const;

    bool is_default_fill_ter_allowed() const {
        // ' ' and '.' symbols allow for default fill_ter
        return value == 32 || value == 46;
    }
};

// copy of default_map_key, but for MapKey class
const MapKey default_editor_map_key(46); // '.' symbol

} // namespace editor

namespace std
{
    template<>
    struct hash<editor::MapKey> {
        size_t operator()(const editor::MapKey& k) const noexcept {
            return hash<uint32_t> {}(k.value);
        }
    };
} // namespace std

#endif // CATA_SRC_EDITOR_MAP_KEY_H
