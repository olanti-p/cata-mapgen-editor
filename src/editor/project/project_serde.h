#ifndef CATA_SRC_EDITOR_PROJECT_SERDE_H
#define CATA_SRC_EDITOR_PROJECT_SERDE_H

namespace editor
{

/**
 * Current project format version.
 */
constexpr int PROJECT_FORMAT_VERSION = 1;

/**
 * Format version of the project being loaded.
 */
int project_load_version();

} // namespace editor

#endif // CATA_SRC_EDITOR_PROJECT_SERDE_H
