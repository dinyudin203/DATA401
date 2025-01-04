CMAKE_MINIMUM_REQUIRED(VERSION 3.16.3)

PROJECT(Assign_2)

# Common include / link directories
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include/ ${CMAKE_SOURCE_DIR}/freeglut/include/ ${CMAKE_SOURCE_DIR}/glew/inc/ )
LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/freeglut/lib/ ${CMAKE_SOURCE_DIR}/glew/lib/)
 
# collect source files
FILE( GLOB SRC *.cpp )
FILE( GLOB HDR *.h *.frag *.vert *.geom )

# depends on OS
IF (UNIX)
  
 # write your own setup for linux

ELSE (UNIX)

   SET( EXTERNAL_LINK_LIBS freeglut.lib glew32.lib )
					 
   ADD_EXECUTABLE( assign_2 ${SRC} ${HDR} ) 
   TARGET_LINK_LIBRARIES( assign_2 ${EXTERNAL_LINK_LIBS} )

   # copy DLLS
   FILE(GLOB_RECURSE DLLS ${CMAKE_SOURCE_DIR}/freeglut/*.dll ${CMAKE_SOURCE_DIR}/glew/*.dll)
   SOURCE_GROUP("DLL Files" FILES ${DLLS})
   FILE(COPY ${DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    
ENDIF (UNIX)
