cmake_minimum_required(VERSION 3.21)

project(FrameCheck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc videoio)
find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(CURL REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

add_executable(FrameCheck WIN32
    main.cpp
)

target_include_directories(FrameCheck PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(FrameCheck PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    Tesseract::libtesseract
)

option(FRAMECHECK_BUILD_TESTS "Build focused frame analysis tests" OFF)
if(FRAMECHECK_BUILD_TESTS)
    enable_testing()
    qt6_generate_moc(main.cpp "${CMAKE_CURRENT_BINARY_DIR}/test-moc/main.moc")
    set_source_files_properties(main.cpp PROPERTIES SKIP_AUTOMOC OFF)
    add_executable(FrameCheckTests tests.cpp "${CMAKE_CURRENT_BINARY_DIR}/test-moc/main.moc")
    set_target_properties(FrameCheckTests PROPERTIES AUTOMOC OFF)
    target_compile_definitions(FrameCheckTests PRIVATE FRAMECHECK_TEST)
    target_include_directories(FrameCheckTests PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/test-moc" ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(FrameCheckTests PRIVATE Qt6::Widgets ${OpenCV_LIBS} Tesseract::libtesseract)
    add_test(NAME FrameCheckTests COMMAND FrameCheckTests)
endif()

# Portable package: cmake --install build --prefix <folder>
get_filename_component(FRAMECHECK_MSYS_PREFIX "${Tesseract_DIR}/../../.." ABSOLUTE)
set(FRAMECHECK_TESSDATA_FILE "${FRAMECHECK_MSYS_PREFIX}/share/tessdata/eng.traineddata")

set(FRAMECHECK_QT_PLUGIN_DIR "${FRAMECHECK_MSYS_PREFIX}/share/qt6/plugins")

install(TARGETS FrameCheck
    RUNTIME_DEPENDENCY_SET FrameCheckRuntime
    RUNTIME DESTINATION .)
install(RUNTIME_DEPENDENCY_SET FrameCheckRuntime
    DIRECTORIES "${FRAMECHECK_MSYS_PREFIX}/bin"
    PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
    POST_EXCLUDE_REGEXES ".*[Ss]ystem32/.*"
    DESTINATION .)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/USER_GUIDE.html" DESTINATION .)
install(FILES "${FRAMECHECK_TESSDATA_FILE}" DESTINATION tessdata)
install(FILES "${FRAMECHECK_QT_PLUGIN_DIR}/platforms/qwindows.dll" DESTINATION platforms)
install(FILES "${FRAMECHECK_QT_PLUGIN_DIR}/styles/qmodernwindowsstyle.dll" DESTINATION styles)
install(FILES
    "${FRAMECHECK_QT_PLUGIN_DIR}/imageformats/qjpeg.dll"
    "${FRAMECHECK_QT_PLUGIN_DIR}/imageformats/qgif.dll"
    "${FRAMECHECK_QT_PLUGIN_DIR}/imageformats/qico.dll"
    DESTINATION imageformats)
