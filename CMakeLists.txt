cmake_minimum_required(VERSION 3.11)
project(pbf)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(SOURCE_DIRS 
    src
    src/renderer
    src/simulator
    src/common
    include
)

foreach(DIR ${SOURCE_DIRS})
    file(GLOB DIR_SOURCES 
        ${DIR}/*.cpp
        ${DIR}/*.hpp
        ${DIR}/*.h   
    )
    list(APPEND SOURCES ${DIR_SOURCES})
endforeach()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW3_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad::glad
    glm::glm
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
) 