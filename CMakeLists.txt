cmake_minimum_required(VERSION 3.12)

project(MimeDetector VERSION 1.2.5 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES translations/MimeDetector_de_DE.ts)
set(RC_FILES
    translations/translations.qrc
)

include(cmake/TranslationUtils.cmake)

set(PROJECT_SOURCES
    src/about.cpp
    src/about.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mimetypemodel.cpp
    src/mimetypemodel.h
    ${RC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    qt_add_executable(mimedetector
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_RES}
    )

else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    if(ANDROID)
        add_library(mimedetector SHARED
            ${PROJECT_SOURCES}
            ${APP_RES}
        )
    else()
        add_executable(mimedetector
            ${PROJECT_SOURCES}
            ${APP_RES}
        )
    endif()
endif()

IF(WIN32)
    SET(INTL_LDFLAGS -lintl)
ENDIF(WIN32)

target_include_directories(mimedetector PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mimedetector PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${ENCHANT_LDFLAGS} ${INTL_LDFLAGS})

target_compile_definitions(mimedetector PUBLIC APP_VERSION="${PROJECT_VERSION}" QT_DISABLE_DEPRECATED_BEFORE=0x051208)

set_target_properties(mimedetector PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.mimedetector
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mimedetector)
endif()
