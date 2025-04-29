#ifndef CATA_SRC_EDITOR_PROJECT_H
#define CATA_SRC_EDITOR_PROJECT_H

#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "common/uuid.h"

namespace editor
{
struct State;

struct Project {
    std::string project_uuid;
    UUIDGenerator uuid_generator;
    std::vector<Mapgen> mapgens;
    std::vector<Palette> palettes;

    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonValue& jsin);

    const Mapgen *get_mapgen( const UUID &fid ) const;
    inline Mapgen *get_mapgen( const UUID &fid ) {
        const Project *this_c = this;
        return const_cast<Mapgen *>( this_c->get_mapgen( fid ) );
    }

    const Palette *get_palette( const UUID &pid ) const;
    inline Palette *get_palette( const UUID &pid ) {
        const Project *this_c = this;
        return const_cast<Palette *>( this_c->get_palette( pid ) );
    }

    const Palette* find_palette_by_string(const std::string& id) const;
    Palette* find_palette_by_string(const std::string& id) {
        const Project* this_c = this;
        return const_cast<Palette*>(this_c->find_palette_by_string(id));
    }

    const Mapgen* find_nested_mapgen_by_string(const std::string& id) const;
    Mapgen* find_nested_mapgen_by_string(const std::string& id) {
        const Project* this_c = this;
        return const_cast<Mapgen*>(this_c->find_nested_mapgen_by_string(id));
    }
};

void show_project_overview_ui( State &state, Project &project, bool &show );

std::unique_ptr<Project> create_empty_project();

} // namespace editor

#endif // CATA_SRC_EDITOR_PROJECT_H
