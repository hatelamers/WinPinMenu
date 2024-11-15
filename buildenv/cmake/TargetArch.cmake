if(MSVC)
	if(DEFINED CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
		set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID})
	elseif(DEFINED CMAKE_GENERATOR_PLATFORM)
		set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_GENERATOR_PLATFORM})
	elseif(DEFINED CONFIG_TARGET_ARCH)
		set(CMAKE_SYSTEM_PROCESSOR ${CONFIG_TARGET_ARCH})
	endif()
	message(STATUS "Setting CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
endif()
set(TARGET_ARCH_ID "${CMAKE_SYSTEM_PROCESSOR}")
set(HOST_ARCH_ID "${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(DEFINED CMAKE_OSX_ARCHITECTURES)
    list(LENGTH CMAKE_OSX_ARCHITECTURES _osx_archs_count)
    if(${_osx_archs_count} GREATER 1)
        set(TARGET_ARCH_ID "universal")
		set(HOST_ARCH_ID "universal")
    endif()
endif()
message(STATUS "Target architecture ID: ${TARGET_ARCH_ID}, host architecture ID: ${HOST_ARCH_ID}")
