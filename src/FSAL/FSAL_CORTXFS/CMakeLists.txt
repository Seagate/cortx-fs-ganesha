cmake_minimum_required(VERSION 2.6.3)

set(LIB_FS_GANESHA ${PROJECT_NAME_BASE}-fs-ganesha)

PROJECT(${LIB_FS_GANESHA} C)

include(CheckIncludeFiles)
include(CheckLibraryExists)

set(KVSFS_GANESHA_MAJOR_VERSION 1)
set(KVSFS_GANESHA_MINOR_VERSION 0)
set(KVSFS_GANESHA_PATCH_LEVEL 1)
set(KVSFS_GANESHA_EXTRA_VERSION ${RELEASE_VER})

set(KVSFS_GANESHA_BASE_VERSION ${BASE_VERSION})
set(NFS_SETUP_DIR ${CMAKE_SOURCE_DIR}/conf)

################################################################################
# Required arguments

set(DEFAULT_GANESHASRC "")
set(DEFAULT_GANESHABUILD "")
set(DEFAULT_CAPIINC "")
set(DEFAULT_NSALINC "")
set(DEFAULT_DSALINC "")
set(DEFAULT_LIBNSAL "")
set(DEFAULT_CORTXFSINC "")
set(DEFAULT_LIBCORTXFS "")
set(DEFAULT_CORTXUTILSINC "")
set(DEFAULT_LIBCORTXUTILS "")

set(CORTXUTILSINC ${DEFAULT_CORTXUTILSINC} CACHE PATH "Path to folder with fault.h")
set(LIBCORTXUTILS ${DEFAULT_LIBCORTXUTILS} CACHE PATH "Path to folder with libcortx-utils.so")

set(DEFAULT_CORTXUTILSINC "")
set(DEFAULT_LIBCORTXUTILS "")

set(CORTXUTILSINC ${DEFAULT_CORTXUTILSINC} CACHE PATH "Path to folder with fault.h")
set(LIBCORTXUTILS ${DEFAULT_LIBCORTXUTILS} CACHE PATH "Path to folder with libcortx-utils.so")

set(GANESHASRC ${DEFAULT_GANESHASRC} CACHE PATH "Path to NFS-Ganesha Source (nfs-ganesha/src)")
set(GANESHABUILD ${DEFAULT_GANESHABUILD} CACHE PATH "Path to NFS-Ganesha Build (nfs-ganesha/build)")
set(FSAL_DESTINATION "/usr/lib64/ganesha" CACHE PATH "Target directory for FSAL")
set(CAPIINC ${DEFAULT_CAPIINC} CACHE PATH "Path to CAPI")
set(NSALINC ${DEFAULT_NSALINC} CACHE PATH "Path to folder with nsal headers")
set(DSALINC ${DEFAULT_DSALINC} CACHE PATH "Path to folder with nsal headers")
set(LIBNSAL ${DEFAULT_LIBNSAL} CACHE PATH "Path to folder with libcortx-nsal.so")

set(CORTXFSINC ${DEFAULT_CORTXFSINC} CACHE PATH "Path to folder with nsal headers")
set(LIBCORTXFS ${DEFAULT_LIBCORTXFS} CACHE PATH "Path to folder with libcortx-nsal.so")

set(PROJECT_NAME_BASE ${PROJECT_NAME_BASE})
set(INSTALL_DIR_ROOT ${INSTALL_DIR_ROOT})

# TODO: Add a condition here and check nfs-ganesha configuration
set(SYSTEM_LIBRARIES
	"/usr/lib64/libjemalloc.so")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${GANESHASRC}/libntirpc)
include_directories(${GANESHASRC}/libntirpc/ntirpc)
include_directories(${GANESHABUILD}/include)
include_directories(${GANESHASRC}/include)
include_directories(${GANESHASRC}/include/FSAL)
include_directories("/usr/include/motr")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CAPIINC}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-variable -DM0_INTERNAL='' -DM0_EXTERN=extern -Wno-attributes -D_REENTRANT -D_GNU_SOURCE ")

# Option (To enable/disable ENABLE_TSDB_ADDB mode.)
option(ENABLE_TSDB_ADDB "Enable ENABLE_TSDB_ADDB mode." OFF)

if (ENABLE_TSDB_ADDB)
	set(BCOND_ENABLE_TSDB_ADDB "%bcond_without")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_TSDB_ADDB")
else (ENABLE_TSDB_ADDB)
	set(BCOND_ENABLE_TSDB_ADDB "%bcond_with")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif (ENABLE_TSDB_ADDB)

message( STATUS "ENABLE_TSDB_ADDB : ${ENABLE_TSDB_ADDB}" )

# TODO: Wrap this with a check against build type
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -fPIC")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")

set(CMAKE_REQUIRED_INCLUDES ${CORTXUTILSINC})

CHECK_INCLUDE_FILES("fault.h" HAVE_CORTX_UTILS_H)

message(STATUS "HAVE_CORTX_UTILS_H=${HAVE_CORTX_UTILS_H}")

if(NOT HAVE_CORTX_UTILS_H)
 if(STRICT_PACKAGE)
    message(FATAL_ERROR "STRICT_PACKAGE: Cannot find CORTX-UTILS runtime. Disabling CORTXFS build")
 else(STRICT_PACKAGE)
    message(WARNING "Cannot find CORTX-UTILS runtime. Disabling CORTXFS build")
    set(USE_CORTXFS OFF)
  endif(STRICT_PACKAGE)
endif(NOT HAVE_CORTX_UTILS_H)

include_directories(${CORTXUTILSINC})
link_directories(${LIBCORTXUTILS})

include_directories(${NSALINC})
link_directories(${LIBNSAL})

include_directories(${DSALINC})

include_directories(${CORTXFSINC})
link_directories(${LIBCORTXFS})

set(CMAKE_REQUIRED_INCLUDES ${CORTXUTILSINC})

CHECK_INCLUDE_FILES("fault.h" HAVE_CORTX_UTILS_H)

message(STATUS "HAVE_CORTX_UTILS_H=${HAVE_CORTX_UTILS_H}")

if(NOT HAVE_CORTX_UTILS_H)
 if(STRICT_PACKAGE)
    message(FATAL_ERROR "STRICT_PACKAGE: Cannot find CORTX-UTILS runtime. Disabling KVSFS build")
 else(STRICT_PACKAGE)
    message(WARNING "Cannot find CORTX-UTILS runtime. Disabling KVSFS build")
    set(USE_FSAL_CORTXFS OFF)
  endif(STRICT_PACKAGE)
endif(NOT HAVE_CORTX_UTILS_H)

include_directories(${CORTXUTILSINC})
link_directories(${LIBCORTXUTILS})

add_subdirectory(config)
set(LIBCONFIG libconfig)

SET(fsalkvsfs_LIB_SRCS
   fsal_internal.c
   fsal_global_tables.c
   main.c
   export.c
   handle.c
   file.c
   xattrs.c
   mds.c
   ds.c
   addb_cfs_plugin.c
)

add_library(libganesha OBJECT ${fsalkvsfs_LIB_SRCS})
set(LIBGANESHA libganesha)

add_library(${LIB_FS_GANESHA} SHARED
	   $<TARGET_OBJECTS:${LIBGANESHA}>
	   $<TARGET_OBJECTS:${LIBCONFIG}>
	   )

target_link_libraries(${LIB_FS_GANESHA}
  ${PROJECT_NAME_BASE}-fs
  ${SYSTEM_LIBRARIES}
)

set_target_properties(${LIB_FS_GANESHA} PROPERTIES VERSION 4.2.0 SOVERSION 4)

########### install files ###############

install(TARGETS ${LIB_FS_GANESHA} COMPONENT fsal DESTINATION  ${FSAL_DESTINATION} )

# rpmbuild specific stuff
set(CPACK_PACKAGE_FILE_NAME "${LIB_FS_GANESHA}-Source" )
set(CPACK_PACKAGE_VENDOR "Seagate")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NFS-Ganesha FSAL for ${PROJECT_NAME_BASE}-FS")
SET(CPACK_PACKAGE_VERSION_MAJOR ${KVSFS_GANESHA_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${KVSFS_GANESHA_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${KVSFS_GANESHA_PATCH_LEVEL})

# Tell CPack the kind of packages to be generated
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")

set(CPACK_SOURCE_IGNORE_FILES
  "/.git/;/.gitignore/;/build/;/.bzr/;~$;${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)

set(PKG_NAME "${CPACK_PACKAGE_NAME}.tar.gz")
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Now create a useable specfile
configure_file(
  "${PROJECT_SOURCE_DIR}/${LIB_FS_GANESHA}.spec-in.cmake"
  "${PROJECT_SOURCE_DIR}/${LIB_FS_GANESHA}.spec"
)

set(RPM_BUILD_OPTIONS " --define '_srcrpmdir ${CMAKE_CURRENT_BINARY_DIR}' --define '_lib_path ${CMAKE_BINARY_DIR}' --define '_nfs_setup_dir ${CMAKE_SOURCE_DIR}/conf' ")

add_custom_target( rpm DEPENDS dist)
add_custom_command(TARGET rpm
	COMMAND sh -c "rpmbuild ${RPM_BUILD_OPTIONS}  -tb ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz"
	VERBATIM
	DEPENDS dist)

# A quick check of the dependencies of the library
add_custom_target(check_ldd DEPENDS ${LIB_FS_GANESHA})
add_custom_command(TARGET check_ldd
	COMMAND env LD_LIBRARY_PATH=${LIBCORTXFS} LD_PRELOAD=/usr/lib64/libganesha_nfsd.so /usr/bin/ldd -r ${CMAKE_BINARY_DIR}/${LIB_FS_GANESHA}.so
	VERBATIM)

