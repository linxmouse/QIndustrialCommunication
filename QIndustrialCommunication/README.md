### 1��Copy `QIndustrialCommunication` �ļ��е�������Ŀ��
### 2��Ȼ����������Ŀ��CMakeLists.txt�����
```cmake
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/QIndustrialCommunication")
find_package(QIC REQUIRED)

target_link_libraries(YourProject PRIVATE ${QIC_LIBRARIES})
```