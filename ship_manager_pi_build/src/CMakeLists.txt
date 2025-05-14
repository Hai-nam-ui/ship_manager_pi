cmake_minimum_required(VERSION 3.1.0)
project(ship_manager_pi)

set(CMAKE_CXX_STANDARD 11)

# Đường dẫn đến mã nguồn OpenCPN
set(OCPN_SRC_DIR "${CMAKE_SOURCE_DIR}/../..")
include_directories(${OCPN_SRC_DIR}/include ${OCPN_SRC_DIR}/ocpninclude)

# Tìm wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Tệp nguồn
set(SRC
    src/ship_manager_pi.h
    src/ship_manager_pi.cpp
)

# Tạo thư viện động
add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC})

# Liên kết thư viện
target_link_libraries(${CMAKE_PROJECT_NAME} ${wxWidgets_LIBRARIES} ${OCPN_SRC_DIR}/build/libopencpn.a)

# Sao chép biểu tượng
file(COPY ${CMAKE_SOURCE_DIR}/ship_manager_pi_icon.png DESTINATION ${CMAKE_BINARY_DIR})