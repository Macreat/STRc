# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/tmp"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/src"
  "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/MateoAlmeida/Desktop/wrkspSTR/str-esp-32-C-c/STRc/code/P2/RGBHttp/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
