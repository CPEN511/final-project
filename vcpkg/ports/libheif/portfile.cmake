vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO  strukturag/libheif 
    REF "v${VERSION}"
    SHA512 ef32fced3a66d888caf2202b55bc4c81094045abfd2806216bbf0c359a30663c500ed5e33a9cef316bcfd933498e87753e9af6b3c179e84c370efd62900493c0
    HEAD_REF master
    PATCHES
        gdk-pixbuf.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DWITH_EXAMPLES=OFF
        -DWITH_DAV1D=OFF
        -DPKG_CONFIG_USE_CMAKE_PREFIX_PATH=ON
)
vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/libheif/)
# libheif's pc file assumes libstdc++, which isn't always true.
vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/lib/pkgconfig/libheif.pc" " -lstdc++" "")
if(NOT VCPKG_BUILD_TYPE)
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/libheif.pc" " -lstdc++" "")
endif()
vcpkg_fixup_pkgconfig()

if (VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/libheif/heif.h" "defined(_MSC_VER) && !defined(LIBHEIF_STATIC_BUILD)" "defined(_WIN32)")
else()
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/libheif/heif.h" "defined(_MSC_VER) && !defined(LIBHEIF_STATIC_BUILD)" "0")
endif()
vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/libheif/heif.h" "#ifdef LIBHEIF_EXPORTS" "#if 0")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib/libheif" "${CURRENT_PACKAGES_DIR}/debug/lib/libheif")

vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/libheif/heif_version.h" "#define LIBHEIF_PLUGIN_DIRECTORY \"${CURRENT_PACKAGES_DIR}/lib/libheif\"" "")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
