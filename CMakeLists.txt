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
    MultidimensionalGeometricEditor_ru_RU.ts
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
    model/projection.h model/projection.cpp
    model/rotator.h model/rotator.cpp
    model/scene.h model/scene.cpp
    model/sceneColorificator.h model/sceneColorificator.cpp
    view/sceneRenderer.h view/sceneRenderer.cpp
    presenterMain.h presenterMain.cpp
    model/opengl/graphics/sceneGeometryManager.cpp model/opengl/graphics/sceneGeometryManager.h
    model/opengl/input/sceneInputHandler.cpp model/opengl/input/sceneInputHandler.h
    model/opengl/objectController/cameraController.cpp model/opengl/objectController/cameraController.h
    model/opengl/other/axisSystem.h model/opengl/other/axisSystem.cpp
    tools/numTools.h tools/numTools.cpp
    view/dataModels/sceneObjectModel.h view/dataModels/sceneObjectModel.cpp
    view/delegates/sceneObjectDelegate.h view/delegates/sceneObjectDelegate.cpp
    view/commands/addSceneObjectCommand.h view/commands/addSceneObjectCommand.cpp
    view/commands/removeSceneObjectCommand.h view/commands/removeSceneObjectCommand.cpp
    view/mainWindowTabWidget.h view/mainWindowTabWidget.cpp
    ${RESOURCES}
    ${UI_FILES}
)

# qt_add_translations(MultidimensionalGeometricEditor
#     TS_FILES ${TS_FILES}
# )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MultidimensionalGeometricEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        view/sceneRendererWidget.h view/sceneRendererWidget.cpp
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

###############
#    Tests    #
###############

include(FetchContent)

FetchContent_Declare(
    gtest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG        main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

set(TESTS
    tests/NDShape.cc
)

set(TESTING_FILES
    model/NDShape.h
    model/NDShape.cpp
)

enable_testing()

add_executable(
  Test
  ${TESTS}
  ${TESTING_FILES}
)
target_link_libraries(Test PRIVATE
  GTest::gtest_main
  Qt${QT_VERSION_MAJOR}::Widgets
)

include(GoogleTest)
gtest_discover_tests(Test)
