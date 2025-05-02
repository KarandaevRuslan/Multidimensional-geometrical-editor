cmake_minimum_required(VERSION 3.16)
project(NDEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets UiTools OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets UiTools OpenGL OpenGLWidgets)

set(TS_FILES
#    NDEditor_en_US.ts
#    NDEditor_ru_RU.ts
)

# Generate .qm files using lrelease
find_program(LRELEASE_EXECUTABLE lrelease)
if (LRELEASE_EXECUTABLE)
    message(STATUS "Found lrelease: ${LRELEASE_EXECUTABLE}")
    set(QM_FILES)
    foreach(TS_FILE IN LISTS TS_FILES)
        get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
        set(QM_OUT "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm")
        add_custom_command(
            OUTPUT ${QM_OUT}
            COMMAND ${LRELEASE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/${TS_FILE} -qm ${QM_OUT}
            DEPENDS ${CMAKE_SOURCE_DIR}/${TS_FILE}
            COMMENT "Compiling translation: ${TS_FILE} -> ${QM_OUT}"
        )
        list(APPEND QM_FILES ${QM_OUT})
    endforeach()
else()
    message(WARNING "lrelease not found, translation files will not be built")
endif()

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
    view/sceneRendererWidget.h view/sceneRendererWidget.cpp
    view/commands/changeSceneObjectCommand.h view/commands/changeSceneObjectCommand.cpp
    view/sceneObjectEditorWidget.h view/sceneObjectEditorWidget.cpp
    view/NDShapeEditorDialog.h view/NDShapeEditorDialog.cpp
    view/axesGroupBox.h view/axesGroupBox.cpp
    view/addSceneObjectDialog.h view/addSceneObjectDialog.cpp
    tools/sceneSerialization.h
    view/dataModels/vertexTableModel.h view/dataModels/vertexTableModel.cpp
    view/dataModels/adjacencyMatrixModel.h view/dataModels/adjacencyMatrixModel.cpp
    view/commands/shapeCommand.h view/commands/shapeCommand.cpp
    view/delegates/noHoverDelegate.h view/delegates/noHoverDelegate.cpp
    view/adjacencyMatrixView.h view/adjacencyMatrixView.cpp
    view/dataModels/rotatorTableModel.h view/dataModels/rotatorTableModel.cpp
    view/wheelShiftTableView.h view/wheelShiftTableView.cpp
    view/delegates/rotatorDelegate.h view/delegates/rotatorDelegate.cpp
    view/sceneObjectListView.h view/sceneObjectListView.cpp
    ${RESOURCES}
    ${UI_FILES}
    ${QM_FILES}
)

if (WIN32)
  set(APP_ICON_RC "${CMAKE_SOURCE_DIR}/resources.rc")
endif()

# macOS: tell CMake which .icns to bundle
set_source_files_properties(
  "${CMAKE_SOURCE_DIR}/images/app_icon.icns"
  PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
set(MACOSX_ICON_FILE "images/app_icon.icns")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NDEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RC}
    )
else()
    if(ANDROID)
        add_library(NDEditor SHARED
            ${PROJECT_SOURCES}
            ${APP_ICON_RC}
        )
    else()
        add_executable(NDEditor
            ${PROJECT_SOURCES}
            ${APP_ICON_RC}
            MACOSX_BUNDLE
            WIN32
        )
    endif()
endif()

target_link_libraries(NDEditor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::UiTools
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NDEditor)
endif()

set_target_properties(NDEditor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_ICON_FILE ${MACOSX_ICON_FILE}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS NDEditor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NDEditor)
endif()

###############
#    Tests    #
###############
if (CMAKE_BUILD_TYPE STREQUAL "Debug"
    OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
    OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
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
endif()
