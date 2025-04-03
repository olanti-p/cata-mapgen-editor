#ifndef CATA_SRC_EDITOR_PROJECT_EXPORT_H
#define CATA_SRC_EDITOR_PROJECT_EXPORT_H

#include <string>

namespace editor
{
struct Project;
} // namespace editor

namespace editor_export
{

std::string to_string( const editor::Project &project );

std::string format_string( const std::string &js );

} // namespace editor_export

#endif // CATA_SRC_EDITOR_PROJECT_EXPORT_H
