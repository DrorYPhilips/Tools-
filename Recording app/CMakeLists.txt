cmake_minimum_required(VERSION 3.20)
project(NdiRecorder LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Multi-Recorder currently requires 64-bit Windows.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NDI_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/NDI 6 SDK")
set(NDI_LIBRARY "${NDI_SDK_ROOT}/Lib/x64/Processing.NDI.Lib.x64.lib")
set(NDI_RUNTIME "${NDI_SDK_ROOT}/Bin/x64/Processing.NDI.Lib.x64.dll")
set(EPIPHAN_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Epiphan DVI2USB3.0 SDK/C++/epiphan_sdk-3.30.3.0007/epiphan_sdk-3.30.3.0007/epiphan" CACHE PATH "Epiphan SDK directory")
option(ENABLE_EPIPHAN "Enable native Epiphan capture when the SDK is present" ON)
set(DATAPATH_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Datapath sdk/SDK/RGBEASY/C" CACHE PATH "Datapath RGBEasy SDK directory")
option(ENABLE_DATAPATH "Enable native Datapath capture when the SDK is present" ON)

file(GLOB_RECURSE APP_REVISION_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.rc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
list(APPEND APP_REVISION_FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${APP_REVISION_FILES})
list(SORT APP_REVISION_FILES)
set(APP_REVISION_INPUT "ENABLE_EPIPHAN=${ENABLE_EPIPHAN}\nENABLE_DATAPATH=${ENABLE_DATAPATH}\n")
foreach(file IN LISTS APP_REVISION_FILES)
    file(RELATIVE_PATH relative_path "${CMAKE_CURRENT_SOURCE_DIR}" "${file}")
    file(SHA256 "${file}" file_hash)
    string(APPEND APP_REVISION_INPUT "${relative_path}:${file_hash}\n")
endforeach()
string(SHA256 APP_REVISION_HASH "${APP_REVISION_INPUT}")
string(SUBSTRING "${APP_REVISION_HASH}" 0 12 APP_REVISION)

# The build number is kept in the source tree so clean rebuilds keep it; it
# increments only when the source fingerprint changes.
set(APP_BUILD_NUMBER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/build-number.txt")
set(APP_BUILD_NUMBER 0)
set(APP_BUILD_REVISION "")
if(EXISTS "${APP_BUILD_NUMBER_FILE}")
    file(STRINGS "${APP_BUILD_NUMBER_FILE}" APP_BUILD_NUMBER_LINE LIMIT_COUNT 1)
    if(APP_BUILD_NUMBER_LINE MATCHES "^([0-9]+) ([0-9a-f]+)$")
        set(APP_BUILD_NUMBER "${CMAKE_MATCH_1}")
        set(APP_BUILD_REVISION "${CMAKE_MATCH_2}")
    endif()
endif()
if(NOT APP_BUILD_REVISION STREQUAL APP_REVISION)
    math(EXPR APP_BUILD_NUMBER "${APP_BUILD_NUMBER} + 1")
    file(WRITE "${APP_BUILD_NUMBER_FILE}" "${APP_BUILD_NUMBER} ${APP_REVISION}\n")
endif()
configure_file(src/app_version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/app_version.h" @ONLY)

if(NOT EXISTS "${NDI_LIBRARY}" OR NOT EXISTS "${NDI_RUNTIME}")
    message(FATAL_ERROR "NDI 6 SDK files were not found in '${NDI_SDK_ROOT}'.")
endif()

set(RECORDER_LIBRARIES
    "${NDI_LIBRARY}" comctl32 comdlg32 mf mfplat mfreadwrite mfuuid ole32 oleaut32 pdh shell32 strmiids vfw32 ws2_32)

# Both editions build from the same sources. NDI_ONLY removes USB, VFW, Epiphan and
# Datapath sources, so the NDI edition needs only the NDI runtime.
function(configure_recorder target edition)
    target_include_directories(${target} PRIVATE "${NDI_SDK_ROOT}/Include" "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_compile_definitions(${target} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${target} PRIVATE ${RECORDER_LIBRARIES})
    if(edition STREQUAL "ndi")
        target_compile_definitions(${target} PRIVATE NDI_ONLY)
    else()
        configure_epiphan(${target})
        configure_datapath(${target})
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NDI_RUNTIME}" "$<TARGET_FILE_DIR:${target}>/Processing.NDI.Lib.x64.dll")
endfunction()

function(configure_epiphan target)
    if(ENABLE_EPIPHAN AND EXISTS "${EPIPHAN_SDK_ROOT}/frmgrab/lib/win/x64/frmgrab.lib"
       AND EXISTS "${EPIPHAN_SDK_ROOT}/frmgrab/bin/win/x64/frmgrab.dll"
       AND EXISTS "${EPIPHAN_SDK_ROOT}/frmgrab/include/frmgrab.h"
       AND EXISTS "${EPIPHAN_SDK_ROOT}/include/v2u_defs.h")
        target_compile_definitions(${target} PRIVATE HAVE_EPIPHAN_SDK)
        target_include_directories(${target} PRIVATE
            "${EPIPHAN_SDK_ROOT}/frmgrab/include" "${EPIPHAN_SDK_ROOT}/include")
        target_link_libraries(${target} PRIVATE "${EPIPHAN_SDK_ROOT}/frmgrab/lib/win/x64/frmgrab.lib")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${EPIPHAN_SDK_ROOT}/frmgrab/bin/win/x64/frmgrab.dll"
                "$<TARGET_FILE_DIR:${target}>/frmgrab.dll")
    elseif(ENABLE_EPIPHAN)
        message(WARNING "Epiphan SDK not found; native Epiphan capture is disabled.")
    endif()
endfunction()

function(configure_datapath target)
    if(ENABLE_DATAPATH AND EXISTS "${DATAPATH_SDK_ROOT}/INCLUDE/RGB.H"
       AND EXISTS "${DATAPATH_SDK_ROOT}/INCLUDE/RGBAPI.H"
       AND EXISTS "${DATAPATH_SDK_ROOT}/INCLUDE/DGCMEDIA.H")
        target_compile_definitions(${target} PRIVATE HAVE_DATAPATH_SDK)
        target_include_directories(${target} PRIVATE "${DATAPATH_SDK_ROOT}/INCLUDE")
    elseif(ENABLE_DATAPATH)
        message(WARNING "Datapath SDK headers not found; native Datapath capture is disabled.")
    endif()
endfunction()

# Every build produces both editions, each in its own folder with its runtime
# DLLs and user guide:
#   <build>/<config>/Multi-Recorder/Multi-Recorder.exe  (NDI, USB, VFW, Epiphan, Datapath)
#   <build>/<config>/NDI-Recorder/NDI-Recorder.exe      (NDI only)
function(add_recorder target name edition)
    add_executable(${target} WIN32 src/main.cpp)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${name}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${name}")
    configure_recorder(${target} ${edition})
    # Runs on every build so guide edits reach the app folder without relinking.
    add_custom_target(${target}-user-guide ALL
        COMMAND ${CMAKE_COMMAND}
            "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/docs/user-guide"
            "-DDESTINATION=$<TARGET_FILE_DIR:${target}>/user-guide"
            "-DEDITION=${edition}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy-user-guide.cmake"
        COMMENT "Copying the ${name} user guide")
    add_dependencies(${target}-user-guide ${target})
endfunction()

add_recorder(multi-recorder Multi-Recorder multi)
add_recorder(ndi-recorder NDI-Recorder ndi)

# Development tools that render the user-guide screenshots from the real window
# code. They are not part of the default build; build one per edition with
#   cmake --build build --config Release --target guide-screenshots-multi guide-screenshots-ndi
foreach(edition IN ITEMS multi ndi)
    add_executable(guide-screenshots-${edition} EXCLUDE_FROM_ALL tools/guide_screenshots.cpp)
    configure_recorder(guide-screenshots-${edition} ${edition})
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(multi-recorder-tests tests/recording_test.cpp)
    configure_recorder(multi-recorder-tests multi)
    add_dependencies(multi-recorder-tests multi-recorder-user-guide)
    add_executable(ndi-recorder-tests tests/recording_test.cpp)
    configure_recorder(ndi-recorder-tests ndi)
    add_dependencies(ndi-recorder-tests ndi-recorder-user-guide)

    add_test(NAME recording-orientation COMMAND multi-recorder-tests)
    add_test(NAME hevc-compression COMMAND multi-recorder-tests --hevc)
    set_tests_properties(hevc-compression PROPERTIES SKIP_RETURN_CODE 77)
    add_test(NAME lagarith-compression COMMAND multi-recorder-tests --lagarith)
    set_tests_properties(lagarith-compression PROPERTIES SKIP_RETURN_CODE 77)
    add_test(NAME mp4-bitrate COMMAND multi-recorder-tests --bitrate)
    add_test(NAME recording-frame-rate COMMAND multi-recorder-tests --frame-rate)
    add_test(NAME recording-monochrome COMMAND multi-recorder-tests --monochrome)
    add_test(NAME recording-privacy-masks COMMAND multi-recorder-tests --privacy-masks)
    add_test(NAME ndi-settings COMMAND multi-recorder-tests --ndi-settings)
    add_test(NAME user-guide COMMAND multi-recorder-tests --user-guide
        "$<TARGET_FILE_DIR:multi-recorder>/user-guide/index.html")
    add_test(NAME ndi-edition-recording COMMAND ndi-recorder-tests)
    add_test(NAME ndi-edition-sources COMMAND ndi-recorder-tests --ndi-edition)
    add_test(NAME ndi-edition-user-guide COMMAND ndi-recorder-tests --user-guide
        "$<TARGET_FILE_DIR:ndi-recorder>/user-guide/index.html")
endif()