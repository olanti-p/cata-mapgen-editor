file(GLOB IMGUI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(
    imgui OBJECT
    ${IMGUI_SOURCES} ${IMGUI_HEADERS}
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PRIVATE cataclysm-bn-tiles-common)
