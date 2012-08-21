CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(calculator C)

SET(SRCS 
    src/calc-main.c 
    src/calculator_edje.c 
    src/calculator_parser.c
    src/calc-expression.c
    src/calc-string.c
    src/calc-view.c
	src/decnumber/decNumber.c
	src/decnumber/decContext.c)
    
SET(VENDOR "tizen")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
#SET(INSTALL_DIR_APPS "/opt/apps/${PKGNAME}")
SET(INSTALL_DIR_PRE "/opt")

SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${PREFIX}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons")
SET(DESKTOPICONDIR "${ICONDIR}/default/small")
SET(EDJDIR "${RESDIR}/edje")
    
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED elementary  utilX
                                dlog svi evas edje
                                ecore-x ecore ecore-input capi-appfw-application
)

FIND_LIBRARY(LIB_M m)

# Apply Public emulator image
FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.calculator.png DESTINATION ${ICONDIR})
    MESSAGE("install confidential icon ...")
ELSE("${ARCH}" STREQUAL "arm")
    INSTALL(FILES ${CMAKE_SOURCE_DIR}/03_mainmenu_icon_calculator.png DESTINATION ${ICONDIR} RENAME
    org.tizen.calculator.png)
    MESSAGE("install releasable icon ...")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS("-fpie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DSLP_PROF")

ADD_DEFINITIONS("-DVENDOR=\"${VENDOR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

ADD_DEFINITIONS("-DINSTALL_DIR_APPS=\"${INSTALL_DIR_APPS}\"")
ADD_DEFINITIONS("-DBINDIR=\"${BINDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")


ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-pie" ${LIB_M})

ADD_CUSTOM_TARGET(calculator.edj 
    COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images
    ${CMAKE_SOURCE_DIR}/calculator.edc ${CMAKE_BINARY_DIR}/calculator.edj
    DEPENDS ${CMAKE_SOURCE_DIR}/calculator.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} calculator.edj)

ADD_CUSTOM_TARGET(calculator_theme.edj
    COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/theme
    ${CMAKE_SOURCE_DIR}/theme/calculator_theme.edc ${CMAKE_BINARY_DIR}/theme/calculator_theme.edj
    DEPENDS ${CMAKE_SOURCE_DIR}/theme/calculator_theme.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} calculator_theme.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION 
${BINDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/calculator.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/theme/calculator_theme.edj DESTINATION 
${EDJDIR})

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})

# process setting
install(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.ini DESTINATION /opt/share/process-info )

ADD_SUBDIRECTORY(po)

# desktop icon
set(PREFIX ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/org.tizen.calculator.xml DESTINATION /opt/share/packages)
install(FILES ${CMAKE_SOURCE_DIR}/org.tizen.calculator.png DESTINATION  /opt/share/icons/default/small)
