#ifndef CATA_SRC_EDITOR_RULER_H
#define CATA_SRC_EDITOR_RULER_H

#include "coordinates.h"

namespace editor
{
struct RulerState {
    std::optional<point_abs_etile> start;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_RULER_H
