# Copies docs/user-guide for one edition.
# Usage: cmake -DSOURCE=<dir> -DDESTINATION=<dir> -DEDITION=multi|ndi -P copy-user-guide.cmake
#
# index.html may contain <!--multi-->...<!--/multi--> and <!--ndi-->...<!--/ndi-->
# blocks; the other edition's blocks are removed and the markers are stripped.
# For the NDI edition, "Multi-Recorder" becomes "NDI-Recorder" and images found in
# images/ndi replace the images with the same name.

if(NOT SOURCE OR NOT DESTINATION OR NOT EDITION MATCHES "^(multi|ndi)$")
    message(FATAL_ERROR "copy-user-guide.cmake needs SOURCE, DESTINATION and EDITION=multi|ndi.")
endif()

file(REMOVE_RECURSE "${DESTINATION}")
if(EDITION STREQUAL "ndi")
    set(drop "multi")
    file(COPY "${SOURCE}/" DESTINATION "${DESTINATION}")
else()
    set(drop "ndi")
    file(COPY "${SOURCE}/" DESTINATION "${DESTINATION}" PATTERN "ndi" EXCLUDE)
endif()

file(READ "${SOURCE}/index.html" html)
set(open "<!--${drop}-->")
set(close "<!--/${drop}-->")
string(LENGTH "${close}" close_length)
while(1)
    string(FIND "${html}" "${open}" start)
    if(start EQUAL -1)
        break()
    endif()
    string(SUBSTRING "${html}" ${start} -1 rest)
    string(FIND "${rest}" "${close}" end)
    if(end EQUAL -1)
        message(FATAL_ERROR "User guide: '${open}' has no matching '${close}'.")
    endif()
    math(EXPR after "${start} + ${end} + ${close_length}")
    string(SUBSTRING "${html}" 0 ${start} before)
    string(SUBSTRING "${html}" ${after} -1 tail)
    set(html "${before}${tail}")
endwhile()
string(REPLACE "<!--${EDITION}-->" "" html "${html}")
string(REPLACE "<!--/${EDITION}-->" "" html "${html}")

if(EDITION STREQUAL "ndi")
    string(REPLACE "Multi-Recorder" "NDI-Recorder" html "${html}")
    file(GLOB ndi_images RELATIVE "${SOURCE}/images/ndi" "${SOURCE}/images/ndi/*.png")
    foreach(image IN LISTS ndi_images)
        string(REPLACE "\"images/${image}\"" "\"images/ndi/${image}\"" html "${html}")
    endforeach()
endif()

file(WRITE "${DESTINATION}/index.html" "${html}")
