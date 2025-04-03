#include "selection_mask.h"

namespace editor
{

void SelectionMask::clear_all()
{
    data.set_all( Bool( false ) );
    num_selected = 0;
}

void SelectionMask::set_all()
{
    data.set_all( Bool( true ) );
    num_selected = data.get_size().x * data.get_size().y;
}

} // namespace editor
