### 1、Copy `QIndustrialCommunication` 文件夹到您的项目中
### 2、然后在您的项目的CMakeLists.txt中添加
```cmake
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/QIndustrialCommunication")
find_package(QIC REQUIRED)

target_link_libraries(YourProject PRIVATE ${QIC_LIBRARIES})
```