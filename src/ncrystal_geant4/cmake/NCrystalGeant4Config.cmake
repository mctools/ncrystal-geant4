cmake_policy(PUSH)#NB: We POP at the end of this file.
cmake_policy(VERSION 3.16...3.31)

if(TARGET NCrystalGeant4::NCrystalGeant4)
  return()
endif()

#Export a few directory paths (relocatable):
set( NCrystalGeant4_CMAKEDIR "${CMAKE_CURRENT_LIST_DIR}" )
set( NCrystalGeant4_INCDIR "${CMAKE_CURRENT_LIST_DIR}/include" )
set( NCrystalGeant4_SRCDIR "${CMAKE_CURRENT_LIST_DIR}/src" )
set(
  NCrystalGeant4_SRCFILES
  "${NCrystalGeant4_SRCDIR}/G4NCInstall.cc"
  "${NCrystalGeant4_SRCDIR}/G4NCMatHelper.cc"
  "${NCrystalGeant4_SRCDIR}/G4NCProcWrapper.cc"
  "${NCrystalGeant4_SRCDIR}/G4NCManager.cc"
)

include( CMakeFindDependencyMacro )

if( NOT TARGET NCrystal::NCrystal )
  if ( NOT DEFINED NCrystal_DIR )
    execute_process(
      COMMAND "ncrystal-config" "--show" "cmakedir"
      OUTPUT_VARIABLE NCrystal_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
  find_dependency( NCrystal 4.1.4 REQUIRED )
endif()

if ( NOT Geant4_LIBRARIES )
  message(
    FATAL_ERROR
    "Make sure your find_package(Geant4) call comes before find_package(NCrystalGeant4)"
  )
endif()

set_source_files_properties(
  ${NCrystalGeant4_SRCFILES}
  PROPERTIES
#  INCLUDE_DIRECTORIES
#  "${NCrystalGeant4_INCDIR}"
#  "${NCrystalGeant4_SRCDIR}"
  LANGUAGE "CXX"
  #OBJECT_DEPENDS "${hdrfiles}"
)

add_library( NCrystalGeant4 STATIC EXCLUDE_FROM_ALL ${NCrystalGeant4_SRCFILES} )
add_library( NCrystalGeant4::NCrystalGeant4 ALIAS NCrystalGeant4)

target_link_libraries(
  NCrystalGeant4
  PUBLIC
  NCrystal::NCrystal
  ${Geant4_LIBRARIES}
)

target_include_directories(
  NCrystalGeant4
  PUBLIC "${NCrystalGeant4_INCDIR}"
)

cmake_policy(POP)
