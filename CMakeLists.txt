cmake_minimum_required (VERSION 2.6)

project (nRF5x C CXX)

set(CMAKE_MODULE_PATH "/home/bootch/git/nRF5Cmake/")
message(" Module path is ${CMAKE_MODULE_PATH}")


# environment (location of NRF_SDK and other tools)
# i.e. file CMakeEnv.cmake on the MODULE_PATH
include("CMakeEnv")


# scripts specific to nRF5x building
# Defines cmake macros prefixed with "nRF5"
# Sets many cmake vars for tool flags
include("nRF5")


nRF5CheckSetupPreconditions()
nRF5SetBuildOptions()
nRF5SetIncludePaths()

# No SDK_SOURCES


# Imported libraries
# Uses: nRF5x, embeddedMath, NRFDrivers (when SD)

add_library(nRF5x52810 STATIC IMPORTED)
set_target_properties( nRF5x52810 
	PROPERTIES IMPORTED_LOCATION /home/bootch/git/nRF5x/cmakeBuild/libnRF5x52810.a
	INTERFACE_INCLUDE_DIRECTORIES "/home/bootch/git/nRF5x/src"
	)
	
add_library(embeddedMath STATIC IMPORTED)
set_target_properties( embeddedMath PROPERTIES 
   IMPORTED_LOCATION /home/bootch/git/embeddedMath/Debug52/libembeddedMath52.a
   INTERFACE_INCLUDE_DIRECTORIES /home/bootch/git/embeddedMath/src
   )

add_library(NRFDrivers STATIC IMPORTED)
set_target_properties( NRFDrivers PROPERTIES 
   IMPORTED_LOCATION /home/bootch/git/libNRFDrivers/Debug/libNRFDrivers.a 
   INTERFACE_INCLUDE_DIRECTORIES /home/bootch/git/libNRFDrivers/
   )





# library targets
# Each specific to a chip
# (It might be possible to build one library that works with all nRF52x chips?)

add_library(radioSoC52810SD "")

set_target_properties( radioSoC52810SD
    PROPERTIES
       CHIP "nrf52810e"
       SOFTDEVICE "s112"
       FLOAT_ABI "soft"
    )
nRF5CheckTargetProperties(radioSoC52810SD)
nRF5ConfigTargetByProperties(radioSoC52810SD)

# enable seggerRTT logging to library
target_compile_definitions(radioSoC52810SD PUBLIC LOGGING)



add_library(radioSoC52810NoSD "")

set_target_properties( radioSoC52810NoSD
    PROPERTIES
       CHIP "nrf52810e"
       SOFTDEVICE "none"
       FLOAT_ABI "soft"
    )
nRF5CheckTargetProperties(radioSoC52810NoSD)
nRF5ConfigTargetByProperties(radioSoC52810NoSD)

# enable seggerRTT logging to library
target_compile_definitions(radioSoC52810NoSD PUBLIC 
    LOGGING
    TASKS)





set(MY_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

list(APPEND MY_SOURCE_LIST
   ${MY_SOURCE_DIR}/clock/clockFacilitator.cpp
   ${MY_SOURCE_DIR}/clock/eventTimer.cpp
   ${MY_SOURCE_DIR}/clock/longClock.cpp
   ${MY_SOURCE_DIR}/clock/sleeper.cpp
   ${MY_SOURCE_DIR}/clock/timer.cpp
   ${MY_SOURCE_DIR}/clock/mcuSleep.cpp
   ${MY_SOURCE_DIR}/ensemble/ensemble.cpp
   ${MY_SOURCE_DIR}/exceptions/faultHandlers.cpp
   ${MY_SOURCE_DIR}/exceptions/powerAssertions.cpp
   ${MY_SOURCE_DIR}/exceptions/resetAssertions.cpp
   ${MY_SOURCE_DIR}/iRQHandlers/powerClockIRQHandler.cpp
   ${MY_SOURCE_DIR}/modules/ledService.cpp
   ${MY_SOURCE_DIR}/modules/powerManager.cpp
   ${MY_SOURCE_DIR}/modules/powerMonitor.cpp
   ${MY_SOURCE_DIR}/radio/radioConfig.cpp
   ${MY_SOURCE_DIR}/radio/radio.cpp
   ${MY_SOURCE_DIR}/radio/radioPower.cpp
   ${MY_SOURCE_DIR}/radio/radioXmitPower.cpp
   ${MY_SOURCE_DIR}/radioUseCase/radioUseCase.cpp
   ${MY_SOURCE_DIR}/services/brownoutRecorder.cpp
   ${MY_SOURCE_DIR}/services/customFlash.cpp
   ${MY_SOURCE_DIR}/services/ledFlasher.cpp
   ${MY_SOURCE_DIR}/services/ledFlasherTask.cpp
   ${MY_SOURCE_DIR}/services/logger.cpp
   ${MY_SOURCE_DIR}/services/mailbox.cpp
   ${MY_SOURCE_DIR}/services/system.cpp
)

# Build RTT logging into library
list(APPEND MY_SDK_SOURCE_LIST
   ${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT.c
   ${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_printf.c
   )





# identical: SD or NoSD
target_sources(
    radioSoC52810SD
    PUBLIC
       "${MY_SOURCE_LIST}"
       "${MY_SDK_SOURCE_LIST}"
    )
    
target_sources(
    radioSoC52810NoSD
    PUBLIC
       "${MY_SOURCE_LIST}"
       "${MY_SDK_SOURCE_LIST}"
    )
    
# identical: SD or NoSD
target_include_directories(radioSoC52810SD
   PUBLIC
       "${CMAKE_CURRENT_LIST_DIR}/src"
   )
target_include_directories(radioSoC52810NoSD
   PUBLIC
       "${CMAKE_CURRENT_LIST_DIR}/src"
   )


# SD: references NRFDrivers
target_link_libraries(radioSoC52810SD
   PUBLIC
       nRF5x52810
       embeddedMath
       NRFDrivers
   )
target_link_libraries(radioSoC52810NoSD
   PUBLIC
       nRF5x52810
       embeddedMath
   )
    




