# 计算此配置文件的位置
get_filename_component(QIC_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# 设置包含路径
set(QIC_INCLUDE_DIRS
	"${QIC_CMAKE_DIR}"
	"${QIC_CMAKE_DIR}/Core"
	"${QIC_CMAKE_DIR}/Misc"
	"${QIC_CMAKE_DIR}/Keyence"
	"${QIC_CMAKE_DIR}/Modbus"
	"${QIC_CMAKE_DIR}/Siemens"
	"${QIC_CMAKE_DIR}/Melsec"
)

# 检查是否已构建库目标
if(NOT TARGET QIC::QIC)
	# 包含导出的目标
	include("${QIC_CMAKE_DIR}/QICTargets.cmake")
endif()

set(QIC_LIBRARIES QIC::QIC)

# 确保依赖项可用
find_package(QT NAMES Qt5 Qt6 COMPONENTS Core Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)