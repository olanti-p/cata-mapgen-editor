#ifndef CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H
#define CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H

namespace editor
{

struct PaletteImportReport {
    int num_pieces_total = 0;
    int num_pieces_failed = 0;
    int num_values_folded = 0;
    int num_mappings = 0;
    int num_constrained = 0;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H
