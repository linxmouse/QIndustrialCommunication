# 获取当前目录下的源文件和头文件
file(GLOB SRCS "*.cpp" "*.c")
file(GLOB HEADERS "*.hpp" "*.h")

# 获取Core目录下的源文件和头文件
file(GLOB SRCS "Core/*.cpp" "Core/*.c")
file(GLOB HEADERS "Core/*.hpp" "Core/*.h")

# 获取实验杂项目录下的源文件和头文件
file(GLOB SRCS "Misc/*.cpp" "Misc/*.c")
file(GLOB HEADERS "Misc/*.hpp" "Misc/*.h")

# 获取Keyence子目录下的源文件和头文件
file(GLOB KEYENCE_SRCS "Keyence/*.cpp" "Keyence/*.cxx" "Keyence/*.c")
file(GLOB KEYENCE_HEADERS "Keyence/*.hpp" "Keyence/*.h")
# 将Keyence子目录下的文件添加到主列表中
list(APPEND SRCS ${KEYENCE_SRCS})
list(APPEND HEADERS ${KEYENCE_HEADERS})

# 获取Modbus子目录下的源文件和头文件
file(GLOB MODBUS_SRCS "Modbus/*.cpp" "Modbus/*.cxx" "Modbus/*.c")
file(GLOB MODBUS_HEADERS "Modbus/*.hpp" "Modbus/*.h")
# 将Modbus子目录下的文件添加到主列表中
list(APPEND SRCS ${MODBUS_SRCS})
list(APPEND HEADERS ${MODBUS_HEADERS})

# 获取Siemens子目录下的源文件和头文件
file(GLOB SIEMENS_SRCS "Siemens/*.cpp" "Siemens/*.cxx" "Siemens/*.c")
file(GLOB SIEMENS_HEADERS "Siemens/*.hpp" "Siemens/*.h")
# 将Siemens子目录下的文件添加到主列表中
list(APPEND SRCS ${SIEMENS_SRCS})
list(APPEND HEADERS ${SIEMENS_HEADERS})

# 获取Melsec子目录下的源文件和头文件
file(GLOB MELSEC_SRCS "Melsec/*.cpp" "Melsec/*.cxx" "Melsec/*.c")
file(GLOB MELSEC_HEADERS "Melsec/*.hpp" "Melsec/*.h")
# 将Melsec子目录下的文件添加到主列表中
list(APPEND SRCS ${MELSEC_SRCS})
list(APPEND HEADERS ${MELSEC_HEADERS})

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

# 将源代码添加到此项目的可执行文件。
add_library(QIC ${SRCS} ${HEADERS})
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET QIC PROPERTY CXX_STANDARD 11)
endif()

# target_link_libraries()链接QIC库时，指定的目录会自动被添加到编译器的包含路径中
target_include_directories(QIC 
	PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/Core"
	"${CMAKE_CURRENT_SOURCE_DIR}/Misc"
	"${CMAKE_CURRENT_SOURCE_DIR}/Keyence"
	"${CMAKE_CURRENT_SOURCE_DIR}/Modbus"
	"${CMAKE_CURRENT_SOURCE_DIR}/Siemens"
)

# TODO: 如有需要，请添加测试并安装目标。
find_package(Qt5 COMPONENTS Core Network)

target_link_libraries(QIC 
	PRIVATE
	Qt5::Core
	Qt5::Network
)
