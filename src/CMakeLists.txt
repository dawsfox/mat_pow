set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -T ${ELDF_MATPOW}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${ELDF_MATPOW} ${CMAKE_BINARY_DIR}/src/softcache/cachemanager.o")

include_directories( ../include
                     ../include/epiphany
                     ../include/arm)

add_subdirectory(softcache)

# Given that I am not able to use two compilers with cmake. I will
# use custom command and custom target to manually build this
if(DEBUG)
    set(EXTERNAL_FLAGS -I${EPIPHANY_HOME}/tools/host/include -L${PTHREAD_LIB} -lpthread ${E_LOADER_LIB} ${E_HAL_LIB} -g)
else()
    set(EXTERNAL_FLAGS -I${EPIPHANY_HOME}/tools/host/include -I${CMAKE_SOURCE_DIR}/eDARTS/include -I${CMAKE_SOURCE_DIR}/eDARTS/include/runtime/host -L${CMAKE_BINARY_DIR}/eDARTS/src/runtime/host -ldarts_api -ldarts_print_server -L${PTHREAD_LIB} -lpthread ${E_LOADER_LIB} ${E_HAL_LIB})
endif()

# For the host we need to compile manually without cmake
add_custom_command(
   OUTPUT sequential.out
   COMMAND gcc sequential/file_sequential_mat_pow.c ${EXTERNAL_FLAGS} -o sequential.out
   DEPENDS sequential/file_sequential_mat_pow.c
   COMMENT "CROSS-COMPILING HOST FILE file_sequential_mat_pow.c "
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   VERBATIM
)

add_custom_target( sequential
                   COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/sequential.out ${CMAKE_CURRENT_BINARY_DIR}/sequential
                    DEPENDS sequential.out sequential/file_sequential_mat_pow.c )

add_custom_command(
   OUTPUT pthread.out
   COMMAND gcc pthread/pthread_mat_pow.c ${EXTERNAL_FLAGS} -o pthread.out
   DEPENDS pthread/pthread_mat_pow.c
   COMMENT "CROSS-COMPILING HOST FILE pthread_mat_pow.c "
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   VERBATIM
)

add_custom_target( pthread
		   COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/pthread.out ${CMAKE_CURRENT_BINARY_DIR}/pthread
		   DEPENDS pthread.out pthread/pthread_mat_pow.c )




add_custom_command(
   OUTPUT epiphany_mat_pow.out
   COMMAND gcc epiphany/main.c ${EXTERNAL_FLAGS} -o epiphany_mat_pow.out
   DEPENDS epiphany/main.c
   COMMENT "CROSS-COMPILING HOST FILE epiphany_mat_pow.out"
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   VERBATIM
)

add_custom_target( epiphany_mat_pow
	           COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/epiphany_mat_pow.out ${CMAKE_CURRENT_BINARY_DIR}/epiphany_mat_pow
		   DEPENDS epiphany_mat_pow.out epiphany/main.c)

# For e_mat_pow.elf
add_executable(e_mat_pow.elf epiphany/e_mat_pow.c)
target_link_libraries(e_mat_pow.elf ${EPIPHANY_LIBS})
add_dependencies(e_mat_pow.elf cachemanager)

#add_dependencies(e_mat_pow.elf darts_fib_test darts_api edarts_cachemanager)
#add_dependencies(e_mat_pow.elf)
