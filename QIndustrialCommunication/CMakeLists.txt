# CMakeList.txt: QIndustrialCommunication 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
file(GLOB SRCS "*.cpp" "*.c")
file(GLOB HEADERS "*.hpp" "*.h")

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

# 将源代码添加到此项目的可执行文件。
# add_library(QIndustrialCommunication ${SRCS} ${HEADERS})
add_executable(QIndustrialCommunication ${SRCS} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET QIndustrialCommunication PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
find_package(Qt5 COMPONENTS Core Network)

target_link_libraries(QIndustrialCommunication 
	PRIVATE
	Qt5::Core
	Qt5::Network
)
