#include "timestamp.h"

#include "string_formatter.h"

#include <chrono>

namespace editor
{

std::string get_timestamp_ms_now()
{
    using namespace std::chrono;
    uint64_t ms = duration_cast<milliseconds>( system_clock::now().time_since_epoch() ).count();
    return string_format( "%u", ms );
}

} // namespace editor
