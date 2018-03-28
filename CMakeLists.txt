cmake_minimum_required(VERSION 3.5)

project(ImageCharacterization)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/BrainTissueCharacterization")
set(EXTENSION_CATEGORY "CSIM")
set(EXTENSION_CONTRIBUTORS "Antonio C. da S. Senra Filho (University of São Paulo), Fabrício H. Simozo (University of São Paulo)")
set(EXTENSION_DESCRIPTION "This extension is composed by modules that can be used for characterizing different tissues in images.")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/BrainTissueCharacterization.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/BrainTissueCharacterization/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(TextureMapExtractor)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
