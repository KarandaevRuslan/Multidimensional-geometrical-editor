cmake_minimum_required(VERSION 3.16)
project(MultidimensionalGeometricEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools UiTools OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools UiTools OpenGL OpenGLWidgets)

set(TS_FILES
    MultidimensionalGeometricEditor_en_US.ts
    MultidimensionalGeometricEditor_en_US.ts
)

set(RESOURCES
    resources.qrc
)

set(UI_FILES
    forms/mainWindow.ui
)

set(PROJECT_SOURCES
    tools/configManager.cpp tools/configManager.h
    tools/logger.cpp tools/logger.h
    view/mainWindow.cpp view/mainWindow.h
    main.cpp
    model/NDShape.cpp model/NDShape.h
    ${TS_FILES}
    ${RESOURCES}
    ${UI_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MultidimensionalGeometricEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        model/projection.h model/projection.cpp

    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MultidimensionalGeometricEditor SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(MultidimensionalGeometricEditor
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(MultidimensionalGeometricEditor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::UiTools
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MultidimensionalGeometricEditor)
endif()

set_target_properties(MultidimensionalGeometricEditor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MultidimensionalGeometricEditor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MultidimensionalGeometricEditor)
endif()
