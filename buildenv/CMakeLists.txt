# cmake -Sbuildenv -Bbuild\Cmake-Debug-x64 -DCMAKE_BUILD_TYPE=Debug
# cmake --build build\Cmake-Debug-x64 --config Debug
# cmake -Sbuildenv -Bbuild\Cmake-Debug-x86 -DCMAKE_BUILD_TYPE=Debug -T host=x64 -A Win32
# cmake --build build\Cmake-Debug-x86 --config Debug
# cmake -Sbuildenv -Bbuild\Cmake-Release-x64 -DCMAKE_BUILD_TYPE=Release -DSIGNER_CERT:STRING=<CERT_PATH> -DSIGNER_PASS:STRING=<CERT_PASSWORD>
# cmake --build build\Cmake-Release-x64 --config Release --target WinPinMenu-package-inst

cmake_minimum_required(VERSION 3.8...3.25)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

get_filename_component(WORKSPACE_ROOT ".." ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(MAIN_TARGET_SOURCE_DIR "${WORKSPACE_ROOT}/src/app")
set(MAIN_TARGET_RESOURCE_DIR "${MAIN_TARGET_SOURCE_DIR}/res")

include("${CMAKE_CURRENT_LIST_DIR}/LocalBuild.cmake" OPTIONAL)

file(READ "${MAIN_TARGET_SOURCE_DIR}/buildnumber.txt" BUILD_NUMBER)
include("${CMAKE_CURRENT_LIST_DIR}/Product.cmake")
if(NOT ${BUILD_NUMBER})
    set(BUILD_NUMBER 1)
endif()

project(
	${PRODUCT_NAME}
	VERSION ${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH}.${BUILD_NUMBER}
	DESCRIPTION ${PRODUCT_DESCRIPTON}
	HOMEPAGE_URL ${PRODUCT_HOMEPAGE_URL}
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(CPACK_SINGLE_TARGET_SYSTEM ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/TargetArch.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/InstallSetup.cmake)

set(PRODUCT_LICENSE_FILE "${WORKSPACE_ROOT}/LICENSE")

set(MAIN_TARGET_NAME ${PROJECT_NAME})

configure_file("${MAIN_TARGET_SOURCE_DIR}/productmeta.h.in" "${MAIN_TARGET_SOURCE_DIR}/productmeta.h")
configure_file("${MAIN_TARGET_SOURCE_DIR}/app.manifest.in" "${MAIN_TARGET_RESOURCE_DIR}/app.manifest")

add_custom_target(${MAIN_TARGET_NAME} ALL)

add_custom_target(${MAIN_TARGET_NAME}-restore)
add_custom_command(
    TARGET ${MAIN_TARGET_NAME}-restore PRE_BUILD
    COMMAND ${CMAKE_VS_MSBUILD_COMMAND}
        "${CMAKE_CURRENT_LIST_DIR}/${MAIN_TARGET_NAME}.sln"
        -t:restore
        -p:RestorePackagesConfig=true
    COMMENT "Restoring NuGet packages"
)

include_external_msproject(${MAIN_TARGET_NAME}-vc "${MAIN_TARGET_SOURCE_DIR}/${MAIN_TARGET_NAME}.vcxproj" ${MAIN_TARGET_NAME}-restore)
set_target_properties(${MAIN_TARGET_NAME}-vc PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_custom_command(
    TARGET ${MAIN_TARGET_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND}
		-DBINARY_DIR=${CMAKE_BINARY_DIR}
		-DSOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
		-DTARGET_SOURCE_DIR=${MAIN_TARGET_SOURCE_DIR}
		-DBUILD_TYPE=$<CONFIG>
		-DTARGET_NAME=${MAIN_TARGET_NAME}
		-P ${CMAKE_CURRENT_LIST_DIR}/cmake/PostBuild.cmake
    COMMENT "Performing post-build tasks ($<CONFIG>)"
)

add_custom_command(
    TARGET ${MAIN_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DSIGNER_ISSUER=${SIGNER_ISSUER}
        -DSIGNER_CERT=${SIGNER_CERT}
        -DSIGNER_PASS=${SIGNER_PASS}
        -DTARGET="${CMAKE_BINARY_DIR}/../$<CONFIG>-${TARGET_ARCH_ID}/${MAIN_TARGET_NAME}.exe"
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/SignTool.cmake
    COMMENT "Signing ${MAIN_TARGET_NAME}"
)

add_dependencies(${MAIN_TARGET_NAME} ${MAIN_TARGET_NAME}-restore ${MAIN_TARGET_NAME}-vc)

install(FILES ${PRODUCT_LICENSE_FILE} CONFIGURATIONS Release DESTINATION ${INSTALL_DOCDIR})
install(PROGRAMS "${CMAKE_BINARY_DIR}/../$<CONFIG>-${TARGET_ARCH_ID}/${MAIN_TARGET_NAME}.exe" CONFIGURATIONS Release DESTINATION ${CMAKE_INSTALL_BINDIR})

include(${CMAKE_CURRENT_LIST_DIR}/cpack/Package.cmake)

add_custom_command(
    TARGET ${MAIN_TARGET_NAME}-package-inst POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DSIGNER_ISSUER=${SIGNER_ISSUER}
        -DSIGNER_CERT=${SIGNER_CERT}
        -DSIGNER_PASS=${SIGNER_PASS}
        -DTARGET="${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.exe"
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/SignTool.cmake
    COMMENT "Signing ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}"
)
