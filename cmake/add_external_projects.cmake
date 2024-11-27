include(ExternalProject)

find_program(MAKE_CMD NAMES gmake nmake make)

ExternalProject_add(libDaisy 
    GIT_REPOSITORY "https://github.com/electro-smith/libDaisy.git"
    GIT_TAG "origin/master"

    SOURCE_DIR ${LIBDAISY_DIR}

    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${LIBDAISY_DIR} && ${MAKE_CMD}
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    TEST_COMMAND ""
)

ExternalProject_add(DaisySP 
    GIT_REPOSITORY "https://github.com/electro-smith/DaisySP.git"
    GIT_TAG "origin/master"

    SOURCE_DIR ${DAISYSP_DIR}

    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${DAISYSP_DIR} && ${MAKE_CMD}
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    TEST_COMMAND ""
)