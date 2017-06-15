# --------------------------------------------------------------------------
#                   OpenMS -- Open-Source Mass Spectrometry
# --------------------------------------------------------------------------
# Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
# ETH Zurich, and Freie Universitaet Berlin 2002-2016.
#
# This software is released under a three-clause BSD license:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of any author or any participating institution
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# For a full list of authors, refer to the file AUTHORS.
# --------------------------------------------------------------------------
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
# INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# --------------------------------------------------------------------------
# $Maintainer: Stephan Aiche $
# $Authors: Stephan Aiche $
# --------------------------------------------------------------------------


# --------------------------------------------------------------------------
# general definitions used for building OpenMS packages
set(CPACK_PACKAGE_NAME "OpenMS")
set(CPACK_PACKAGE_VENDOR "OpenMS.de")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenMS - A framework for mass spectrometry")
set(CPACK_PACKAGE_VERSION "${OPENMS_PACKAGE_VERSION_MAJOR}.${OPENMS_PACKAGE_VERSION_MINOR}.${OPENMS_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${OPENMS_PACKAGE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${OPENMS_PACKAGE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${OPENMS_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenMS-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/cmake/OpenMSPackageDescriptionFile.cmake)
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/License.txt)
set(CPACK_RESOURCE_FILE_WELCOME ${PROJECT_SOURCE_DIR}/cmake/OpenMSPackageResourceWelcomeFile.txt)
set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/cmake/OpenMSPackageResourceReadme.txt)

########################################################### Share

install(DIRECTORY share/
  DESTINATION OpenMS-${CPACK_PACKAGE_VERSION}/share
  COMPONENT share
  FILE_PERMISSIONS      OWNER_WRITE OWNER_READ
                        GROUP_READ
                        WORLD_READ
  DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ
                        WORLD_EXECUTE WORLD_READ
  REGEX "^\\..*" EXCLUDE ## Exclude hidden files (svn, git, DSStore)
  REGEX ".*\\/\\..*" EXCLUDE ## Exclude hidden files in subdirectories
)

########################################################### Libraries
# Libraries hack, avoid cmake interferring with our own lib fixes
#install(DIRECTORY ${PROJECT_BINARY_DIR}/lib/
#  DESTINATION OpenMS-${CPACK_PACKAGE_VERSION}/${PACKAGE_LIB_DIR}/
#  COMPONENT library
#)

########################################################### TOPP Binaries
# Binary hack, avoid cmake interferring with our own lib fixes
#install(DIRECTORY ${PROJECT_BINARY_DIR}/bin/
#  DESTINATION OpenMS-${CPACK_PACKAGE_VERSION}/${PACKAGE_BIN_DIR}
#  COMPONENT applications
#  FILE_PERMISSIONS      OWNER_EXECUTE OWNER_WRITE OWNER_READ
#                        GROUP_READ GROUP_EXECUTE
#                        WORLD_READ WORLD_EXECUTE
#  DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#                        GROUP_READ GROUP_EXECUTE
#                        WORLD_READ WORLD_EXECUTE
#  PATTERN "*.app" EXCLUDE
#  ## TODO exclude Tutorial_binaries?
#  REGEX "^\\..*" EXCLUDE ## Exclude hidden files (svn, git, DSStore)
#  REGEX ".*\\/\\..*" EXCLUDE ## Exclude hidden files in subdirectories
#)

########################################################### Documentation Preparation

#------------------------------------------------------------------------------
# Since doc_tutorial is built with ALL when ENABLE_TUTORIALS is ON, we can assume these
# PDFs are present. The ALL target is executed automatically with "make package"

if (ENABLE_TUTORIALS)
  install(FILES       ${PROJECT_BINARY_DIR}/doc/OpenMS_tutorial.pdf
          DESTINATION doc
          COMPONENT doc
          PERMISSIONS OWNER_WRITE OWNER_READ
                      GROUP_READ
                      WORLD_READ

  )

  install(FILES     ${PROJECT_BINARY_DIR}/doc/TOPP_tutorial.pdf
          DESTINATION doc
          COMPONENT doc
          PERMISSIONS OWNER_WRITE OWNER_READ
                      GROUP_READ
                      WORLD_READ
  )
else()
  message("Warning: Configuring for packaging without tutorials. If you want to build a full package make sure that configuration with -DENABLE_TUTORIALS=On succeeds.")
endif ()

########################################################### Documentation
install(FILES       ${PROJECT_BINARY_DIR}/doc/index.html
        DESTINATION doc
        RENAME OpenMSAndTOPPDocumentation.html
        COMPONENT doc
        PERMISSIONS OWNER_WRITE OWNER_READ
                    GROUP_READ
                    WORLD_READ
)

install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html
        DESTINATION doc
        COMPONENT doc
        FILE_PERMISSIONS      OWNER_WRITE OWNER_READ
                              GROUP_READ
                              WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                              GROUP_EXECUTE GROUP_READ
                              WORLD_EXECUTE WORLD_READ
        REGEX "^\\..*" EXCLUDE ## Exclude hidden files (svn, git, DSStore)
        REGEX ".*\\/\\..*" EXCLUDE ## Exclude hidden files in subdirectories
)

########################################################### Fixing dynamic dependencies
# Done on Windows via copying external and internal dlls to the install/bin/ folder
# Done on Mac via fixup_bundle for the GUI apps (TOPPView, TOPPAS) and via fix_mac_dependencies for the TOPP tools
# which recursively gathers dylds, copies them to install/lib/ and sets the install_name of the binaries to @executable_path/../lib
# Not done on Linux. Either install systemwide (omit CMAKE_INSTALL_PREFIX or set it to /usr/) or install and add the
# install/lib/ folder to the LD_LIBRARY_PATH

#install(CODE "
#  include(BundleUtilities)
#  GET_BUNDLE_ALL_EXECUTABLES(\${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR} EXECS)
#  fixup_bundle(\"${EXECS}\" \"\" \"\${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}\")
#  " COMPONENT applications)

########################################################### SEARCHENGINES
set(THIRDPARTY_COMPONENT_GROUP)
if(EXISTS ${SEARCH_ENGINES_DIRECTORY})

  ## populates the THIRDPARTY_COMPONENT_GROUP list
  install_thirdparty_folder("Comet")
  install_thirdparty_folder("Fido")
  install_thirdparty_folder("MSGFPlus")
  install_thirdparty_folder("OMSSA")
  install_thirdparty_folder("XTandem")
  install_thirdparty_folder("LuciPHOr2")
  install_thirdparty_folder("MyriMatch")
endif()