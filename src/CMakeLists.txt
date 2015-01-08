

FILE(GLOB_RECURSE FOO_SRC  *.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIB_NAME} SHARED ${FOO_SRC})

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi" )

target_link_libraries(${LIB_NAME} ${GLFW_LIBRARIES})
target_link_libraries(${LIB_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${LIB_NAME} ${LIBS})

install(TARGETS ${LIB_NAME} DESTINATION lib)