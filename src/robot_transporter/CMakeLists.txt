cmake_minimum_required(VERSION 3.8)
project(robot_transporter)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})

# Cari package OpenCV
find_package(OpenCV REQUIRED)

# ... (pastikan add_executable kamu sudah ada seperti di bawah ini)
add_executable(generate_marker src/generate_marker.cpp)
target_include_directories(generate_marker PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(generate_marker ${OpenCV_LIBS})

add_executable(teleop_node src/teleop_node.cpp)
ament_target_dependencies(teleop_node rclcpp geometry_msgs sensor_msgs)

add_executable(line_follower src/line_follower.cpp)
ament_target_dependencies(line_follower rclcpp geometry_msgs std_msgs)

add_executable(aruco_detector src/aruco_detector.cpp)
ament_target_dependencies(aruco_detector rclcpp std_msgs OpenCV)
target_include_directories(aruco_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(aruco_detector ${OpenCV_LIBS})

install(TARGETS generate_marker aruco_detector teleop_node line_follower DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
