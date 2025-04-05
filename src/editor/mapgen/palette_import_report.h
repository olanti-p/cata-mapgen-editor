#ifndef CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H
#define CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H

namespace editor
{

struct PaletteImportReport {
    int num_total = 0;
    int num_failed = 0;
    int num_pieces_omitted = 0;
    int num_values_folded = 0;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_IMPORT_REPORT_H
