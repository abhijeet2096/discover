add_executable(plasma-discover-exporter main.cpp MuonExporter.cpp)

target_link_libraries(plasma-discover-exporter Discover::Common KF5::CoreAddons KF5::I18n)
