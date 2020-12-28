project(host-gdi)

set(SRC
	main.cpp)
source_group("\\" FILES ${SRC})

add_executable(${PROJECT_NAME} WIN32
	${SRC})
