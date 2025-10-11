# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/mbaguero/esp/esp-idf/components/bootloader/subproject"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/tmp"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/src/bootloader-stamp"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/src"
  "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/mbaguero/esp/lab1/lab1_3/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
