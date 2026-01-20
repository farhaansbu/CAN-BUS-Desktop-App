
add_executable (CAN_BUS_IOT_exe
                App.cpp
                DashboardUI.cpp
                CanFrameParser.cpp
                TcpServer.cpp
                TelemetryModel.cpp
                main.cpp)

target_include_directories(CAN_BUS_IOT_exe
        PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(CAN_BUS_IOT_exe 
                      PRIVATE sfml-network
                      PRIVATE sfml-window
                      PRIVATE sfml-system
                      PRIVATE ImGui-SFML::ImGui-SFML)

add_custom_command(TARGET CAN_BUS_IOT_exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:CAN_BUS_IOT_exe>/assets
)

target_compile_definitions(CAN_BUS_IOT_exe PRIVATE
    ASSETS_DIR="$<TARGET_FILE_DIR:CAN_BUS_IOT_exe>/assets"
)



