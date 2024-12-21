cmake_minimum_required(VERSION 3.10)
project(computer_graphics)

set(CMAKE_CXX_STANDARD 17)

# Добавляем определение для подавления предупреждений об устаревших функциях
add_definitions(-DGL_SILENCE_DEPRECATION)

# Находим необходимые пакеты
find_package(SFML 2.5 COMPONENTS window system graphics REQUIRED)
find_package(OpenGL REQUIRED)

# Добавляем исполняемые файлы для всех лабораторных работ
add_executable(lab1 lab1_v10.cpp)
add_executable(lab2 lab2_v11.cpp)
add_executable(lab3 lab3_v3.cpp)
add_executable(lab4 lab4_v12.cpp)
add_executable(lab5 lab5_v5.cpp)

# Настройка для всех таргетов
foreach(target lab1 lab2 lab3 lab4 lab5)
    if(APPLE)
        target_include_directories(${target} PRIVATE 
            /opt/homebrew/include
            ${OPENGL_INCLUDE_DIRS}
        )
        target_link_directories(${target} PRIVATE /opt/homebrew/lib)
        
        find_library(OPENGL_FRAMEWORK OpenGL)
        
        target_link_libraries(${target}
            sfml-window
            sfml-system
            sfml-graphics
            ${OPENGL_FRAMEWORK}
            "-framework OpenGL"
        )
    else()
        target_link_libraries(${target}
            sfml-window
            sfml-system
            sfml-graphics
            OpenGL::GL
            OpenGL::GLU
        )
    endif()

    # Добавляем флаги компиляции
    if(APPLE)
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wno-deprecated-declarations)
    endif()
endforeach()