#ifndef CATA_SRC_EDITOR_TOOL_BUCKET_H
#define CATA_SRC_EDITOR_TOOL_BUCKET_H

#include "common/canvas_2d.h"
#include "tool.h"

namespace editor
{
struct SelectionMask;
} // namespace editor

namespace editor::tools
{

struct BucketSettings : public ToolSettings {
    bool global = false;
    bool in_selection = false;  // TODO

    void serialize( JsonOut &jsout ) const override;
    void deserialize( JSON_IN &jsin ) override;

    void show() override;
};

struct BucketControl : public ToolControl {
    void handle_tool_operation( ToolTarget &target ) override;

    std::vector<point> find_affected_tiles(
        BucketSettings &settings,
        Canvas2D<MapKey> &canvas,
        SelectionMask &selection,
        point pos,
        MapKey new_value
    ) const;
    static void apply( Canvas2D<MapKey> &canvas, const std::vector<point> &tiles, MapKey new_value );
};

struct Bucket : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<BucketControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<BucketSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_BUCKET_H
