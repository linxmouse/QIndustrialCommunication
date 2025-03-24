# ����������ļ���λ��
get_filename_component(QIC_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# ���ð���·��
set(QIC_INCLUDE_DIRS
	"${QIC_CMAKE_DIR}"
	"${QIC_CMAKE_DIR}/Core"
	"${QIC_CMAKE_DIR}/Misc"
	"${QIC_CMAKE_DIR}/Keyence"
	"${QIC_CMAKE_DIR}/Modbus"
	"${QIC_CMAKE_DIR}/Siemens"
	"${QIC_CMAKE_DIR}/Melsec"
)

# ����Ƿ��ѹ�����Ŀ��
if(NOT TARGET QIC::QIC)
	# ����������Ŀ��
	include("${QIC_CMAKE_DIR}/QICTargets.cmake")
endif()

set(QIC_LIBRARIES QIC::QIC)

# ȷ�����������
find_package(QT NAMES Qt5 Qt6 COMPONENTS Core Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)