cmake_minimum_required(VERSION 3.16)
project(PacmanSFML VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executável sai em build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# SFML 2.6.2 baixado e compilado junto com o projeto (linkagem estática)
option(BUILD_SHARED_LIBS "Compilar bibliotecas como DLL" OFF)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW ON
)
# O CMakeLists do SFML 2.6.2 declara compatibilidade com o CMake 3.7, o que gera o aviso
# "Compatibility with CMake < 3.10" nas versões novas do CMake. Só para o SFML, ele passa
# a ser tratado como compatível com o 3.10, e o aviso some.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_MakeAvailable(SFML)
unset(CMAKE_POLICY_VERSION_MINIMUM)

add_executable(pacman
    src/main.cpp
    src/Fantasma.cpp
    src/Jogo.cpp
    src/Mapas.cpp
    src/Pacman.cpp
    src/Ranking.cpp
    src/Recursos.cpp
    src/Telas.cpp
    src/Tipos.cpp
)
target_link_libraries(pacman PRIVATE sfml-graphics)

# MinGW: embute libstdc++/libgcc para o .exe rodar fora do terminal do MSYS2
if(MINGW)
    target_link_options(pacman PRIVATE -static)
endif()

# Copia a pasta assets/ para o lado do executável
add_custom_command(TARGET pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:pacman>/assets
    VERBATIM)