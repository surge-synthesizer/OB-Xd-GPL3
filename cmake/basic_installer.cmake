# A basic installer setup.
#
# This cmake file introduces two targets
#  obxf-staged:      moves all the built assets to a well named directory
#  obxf-installer:   depends on obxf-staged, builds an installer
#
# Right now obxf-installer builds just the crudest zip file but this is the target
# on which we will hang the proper installers later

set(OBXF_PRODUCT_DIR ${CMAKE_BINARY_DIR}/obxf_products)
file(MAKE_DIRECTORY ${OBXF_PRODUCT_DIR})

add_custom_target(obxf-staged)
add_custom_target(obxf-installer)

set(TARGET_BASE OB-Xf)

function(obxf_package format)
    get_target_property(output_dir ${TARGET_BASE} RUNTIME_OUTPUT_DIRECTORY)

    if(TARGET ${TARGET_BASE}_${format})
        add_dependencies(obxf-staged ${TARGET_BASE}_${format})
        add_custom_command(
                TARGET obxf-staged
                POST_BUILD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND echo "Installing ${output_dir}/${format} to ${OBXF_PRODUCT_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${output_dir}/${format}/ ${OBXF_PRODUCT_DIR}/
        )
    endif()
endfunction()

obxf_package(VST3)
obxf_package(AU)
obxf_package(CLAP)
obxf_package(LV2)
obxf_package(Standalone)

if (WIN32)
    message(STATUS "Including special windows cleanup installer stage")
    add_custom_command(TARGET obxf-staged
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Cleaning up windows goobits"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${OBXF_PRODUCT_DIR}/obxf.exp"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${OBXF_PRODUCT_DIR}/obxf.ilk"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${OBXF_PRODUCT_DIR}/obxf.lib"
            COMMAND ${CMAKE_COMMAND} -E rm -f "${OBXF_PRODUCT_DIR}/obxf.pdb"
            )
endif ()

add_dependencies(obxf-installer obxf-staged)

add_custom_command(
        TARGET obxf-installer
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND echo "Installing ${output_dir}/${format} to ${OBXF_PRODUCT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${OBXF_PRODUCT_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/installer/ZipReadme.txt ${OBXF_PRODUCT_DIR}/Readme.txt
)


find_package(Git)


string(TIMESTAMP OBXF_DATE "%Y-%m-%d")
set(OBXF_ZIP obxf-${OBXF_DATE}-${GIT_COMMIT_HASH}-${CMAKE_SYSTEM_NAME}${OBXF_EXTRA_ZIP_NAME}.zip)
set(OBXF_ASSETS_ZIP obxf-${OBXF_DATE}-${GIT_COMMIT_HASH}-assets.zip)
message(STATUS "Zip File Name is ${OBXF_ZIP}")

if (APPLE)
    message(STATUS "Configuring for mac installer")
    add_custom_command(
            TARGET obxf-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${sstplugininfra_SOURCE_DIR}/scripts/installer_mac/make_installer.sh "OB-Xf" ${OBXF_PRODUCT_DIR} ${CMAKE_SOURCE_DIR}/resources/installer_mac ${CMAKE_BINARY_DIR}/installer "${OBXF_DATE}-${GIT_COMMIT_HASH}"
    )
elseif (WIN32)
    message(STATUS "Configuring for win installer")
    add_custom_command(
            TARGET obxf-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND 7z a -r installer/${OBXF_ZIP} ${OBXF_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "ZIP Installer in: installer/${OBXF_ZIP}")
    message(STATUS "Skipping NuGet for now")
    #find_program(OBXF_NUGET_EXE nuget.exe PATHS ENV "PATH")
    #if(OBXF_NUGET_EXE)
    #    message(STATUS "NuGet found at ${OBXF_NUGET_EXE}")
    #    add_custom_command(
    #        TARGET obxf-installer
    #        POST_BUILD
    #        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    #        COMMAND ${OBXF_NUGET_EXE} install Tools.InnoSetup -version 6.2.1
    #        COMMAND Tools.InnoSetup.6.2.1/tools/iscc.exe /O"installer" /DOBXF_SRC="${CMAKE_SOURCE_DIR}" /DOBXF_BIN="${CMAKE_BINARY_DIR}" /DMyAppVersion="${OBXF_DATE}-${GIT_COMMIT_HASH}" "${CMAKE_SOURCE_DIR}/resources/installer_win/monique${BITS}.iss")
    #else()
    #    message(STATUS "NuGet not found")
    #endif()
else ()
    message(STATUS "Basic Installer: Target is installer/${OBXF_ZIP}")
    add_custom_command(
            TARGET obxf-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_COMMAND} -E tar cvf installer/${OBXF_ZIP} --format=zip ${OBXF_PRODUCT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E echo "Installer in: installer/${OBXF_ZIP}")


    # Only build the assets zip on linux, to be CI friendly
    add_custom_command(
            TARGET obxf-installer
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/assets
            COMMAND ${CMAKE_COMMAND} -E tar cvf ${CMAKE_BINARY_DIR}/installer/${OBXF_ASSETS_ZIP} --format=zip .
            COMMAND ${CMAKE_COMMAND} -E echo "Installer assets: installer/${OBXF_ASSETS_ZIP}")

endif ()
