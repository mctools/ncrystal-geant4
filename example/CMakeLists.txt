################################################################################
##                                                                            ##
##  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   ##
##                                                                            ##
##  Copyright 2015-2025 NCrystal developers                                   ##
##                                                                            ##
##  Licensed under the Apache License, Version 2.0 (the "License");           ##
##  you may not use this file except in compliance with the License.          ##
##  You may obtain a copy of the License at                                   ##
##                                                                            ##
##      http://www.apache.org/licenses/LICENSE-2.0                            ##
##                                                                            ##
##  Unless required by applicable law or agreed to in writing, software       ##
##  distributed under the License is distributed on an "AS IS" BASIS,         ##
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  ##
##  See the License for the specific language governing permissions and       ##
##  limitations under the License.                                            ##
##                                                                            ##
################################################################################

cmake_minimum_required(VERSION 3.16...3.31)

project( MyGeant4Project VERSION 0.0.1 LANGUAGES CXX )

#First we must find both Geant4 and NCrystalGeant4. We do so via find_package
#statements, but in order to support pip installed ncrystal-geant4, we ask the
#ncrystal-geant4 command for where to find the NCrystalGeant4 configuration
#files.

if( NOT DEFINED NCrystalGeant4_DIR )
  #Be sure to pick up location when NCrystalGeant4 was installed via "pip
  #install ncrystal-geant4".
  execute_process(
    COMMAND ncrystal-geant4 --cmakedir
    OUTPUT_VARIABLE NCrystalGeant4_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
find_package( Geant4 11.1.0 REQUIRED )
find_package( NCrystalGeant4 0.0.1 REQUIRED )

#Now we add our own code, in this case an executable:
add_executable( "myapp" "ncrystalgeant4_example.cc" )

#Our executable should link against both NCrystalGeant4 and Geant4:
target_link_libraries(
  "myapp"
  NCrystalGeant4::NCrystalGeant4
  ${Geant4_LIBRARIES}
)

install(TARGETS "myapp" RUNTIME DESTINATION bin)
