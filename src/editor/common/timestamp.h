#ifndef CATA_SRC_EDITOR_TIMESTAMP_H
#define CATA_SRC_EDITOR_TIMESTAMP_H

#include "uuid.h"
#include "canvas_2d.h"

#include <vector>

namespace editor
{

/** Returns timestamp string in milliseconds. */
std::string get_timestamp_ms_now();

} // namespace editor

#endif // CATA_SRC_EDITOR_TIMESTAMP_H
