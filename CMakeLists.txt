cmake_minimum_required(VERSION 3.16)

project(MiniTasks VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg SvgWidgets)

add_executable(IconGenerator assets/icon_generator.cpp)
target_link_libraries(IconGenerator PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::SvgWidgets)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/app.ico
    COMMAND ${CMAKE_COMMAND} -E env "PATH=C:/Qt/6.6.0/msvc2019_64/bin;$ENV{PATH}" $<TARGET_FILE:IconGenerator>
    DEPENDS IconGenerator ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.svg
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets
    COMMENT "Generating app.ico from icon.svg"
)

add_custom_target(GenerateIcon DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/app.ico)

include_directories(include)

add_executable(MiniTasks
    src/main.cpp
    assets/app.rc
    src/TaskStorage.cpp include/TaskStorage.h
    src/TaskItemWidget.cpp include/TaskItemWidget.h
    src/TaskEditModal.cpp include/TaskEditModal.h
    src/TaskPopup.cpp include/TaskPopup.h
    src/SidePanel.cpp include/SidePanel.h
    src/FloatingButton.cpp include/FloatingButton.h
    src/utils/SmartParser.cpp include/utils/SmartParser.h
)

add_dependencies(MiniTasks GenerateIcon)

target_link_libraries(MiniTasks PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::SvgWidgets dwmapi)

# Optional: Disable console window in release mode on Windows
if(WIN32)
    set_target_properties(MiniTasks PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Deploy Qt DLLs and resources next to executable
if(WIN32)
    qt_generate_deploy_app_script(
        TARGET MiniTasks
        FILENAME_VARIABLE deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
    install(TARGETS MiniTasks DESTINATION bin)
endif()

# CPack NSIS Setup Installer Configuration
set(CPACK_PACKAGE_NAME "MiniTasks")
set(CPACK_PACKAGE_VENDOR "https://github.com/AashuPatel/")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Minimal, predictable task list.")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\assets\\\\app.ico")
set(CPACK_GENERATOR "NSIS")

set(CPACK_NSIS_DISPLAY_NAME "MiniTasks Desktop App")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\assets\\\\app.ico")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")

# Tell NSIS to create a Start Menu shortcut, Desktop shortcut, and a Startup folder shortcut for auto-boot.
set(CPACK_NSIS_CREATE_ICONS_EXTRA "
  CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\MiniTasks.lnk\\\" \\\"$INSTDIR\\\\bin\\\\MiniTasks.exe\\\"
  CreateShortCut \\\"$DESKTOP\\\\MiniTasks.lnk\\\" \\\"$INSTDIR\\\\bin\\\\MiniTasks.exe\\\"
  CreateShortCut \\\"$SMSTARTUP\\\\MiniTasks.lnk\\\" \\\"$INSTDIR\\\\bin\\\\MiniTasks.exe\\\"
")

set(CPACK_NSIS_DELETE_ICONS_EXTRA "
  Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\MiniTasks.lnk\\\"
  Delete \\\"$DESKTOP\\\\MiniTasks.lnk\\\"
  Delete \\\"$SMSTARTUP\\\\MiniTasks.lnk\\\"
")

include(CPack)
