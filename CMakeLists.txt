cmake_minimum_required(VERSION 3.20)
project(zadanie_cern CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # kompilator musi obslugiwac C++17

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ROOT REQUIRED COMPONENTS Core Hist RIO) # znajdz roota w systemie

find_path(PYTHIA8_INCLUDE_DIR Pythia8/Pythia.h
        HINTS $ENV{PYTHIA8}/include $ENV{PYTHIA8_DIR}/include
        PATHS /opt/homebrew/include /usr/local/include /usr/include)
find_library(PYTHIA8_LIBRARY pythia8
        HINTS $ENV{PYTHIA8}/lib $ENV{PYTHIA8_DIR}/lib
        PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)

if(NOT PYTHIA8_INCLUDE_DIR OR NOT PYTHIA8_LIBRARY)
    message(FATAL_ERROR
            "Nie znaleziono Pythia 8. Ustaw zmienna srodowiskowa PYTHIA8 na katalog instalacji "
            "(zawierajacy include/ i lib/) albo przekaz -DPYTHIA8_INCLUDE_DIR=... -DPYTHIA8_LIBRARY=...")
endif()
# draw plots uruchamiane w root wiec musimy zbudowac program simulate w pliku simulate.cpp
add_executable(simulate simulate.cpp)
# konfiguracja -> dodaj czesci roota i biblioteke pythia
target_include_directories(simulate PRIVATE ${PYTHIA8_INCLUDE_DIR})
target_link_libraries(simulate PRIVATE
        ROOT::Core ROOT::Hist ROOT::RIO
        ${PYTHIA8_LIBRARY})