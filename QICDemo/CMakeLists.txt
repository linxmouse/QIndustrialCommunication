# 将源代码添加到此项目的可执行文件。
add_executable(QICDemo Demo.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET QICDemo PROPERTY CXX_STANDARD 11)
endif()

# 如果QIC的头文件需要特别的包含路径
target_include_directories(QICDemo PRIVATE "../QIndustrialCommunication")

# TODO: 如有需要，请添加测试并安装目标。
find_package(Qt5 COMPONENTS Core Network)

target_link_libraries(QICDemo 
	PRIVATE
	Qt5::Core
	Qt5::Network
	QIC
)
