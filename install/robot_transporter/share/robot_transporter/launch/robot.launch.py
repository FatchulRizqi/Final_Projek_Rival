import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    # 1. Mendapatkan path ke folder config di dalam package kamu
    pkg_name = 'robot_transporter'
    pkg_share = get_package_share_directory(pkg_name)
    twist_mux_params = os.path.join(pkg_share, 'config', 'twist_mux.yaml')

    # 2. Node pembaca hardware stik PS4 bawaan ROS 2 (menghasilkan topik /joy)
    joy_node = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        parameters=[{
            'deadzone': 0.05, # Mengurangi sensitivitas saat analog diam
            'autorepeat_rate': 20.0 # Seberapa cepat data dikirim (Hz)
        }]
    )

    # 3. Node buatanmu untuk mengubah input /joy menjadi /cmd_vel_manual & /servo_cmd
    teleop_node = Node(
        package=pkg_name,
        executable='teleop_node',
        name='teleop_node',
        output='screen'
    )

    # 4. Node buatanmu untuk membaca garis dan menghasilkan /cmd_vel_auto
    line_follower_node = Node(
        package=pkg_name,
        executable='line_follower',
        name='line_follower_node',
        output='screen'
    )

    # 5. Node Twist Mux pengatur prioritas kecepatan
    twist_mux_node = Node(
        package='twist_mux',
        executable='twist_mux',
        name='twist_mux',
        parameters=[twist_mux_params],
        # Twist mux secara default mengeluarkan hasil ke /cmd_vel_out, kita ubah (remap) ke /cmd_vel
        remappings=[('/cmd_vel_out', '/cmd_vel')]
    )

    # Kembalikan semua node di atas untuk dieksekusi bersamaan
    return LaunchDescription([
        joy_node,
        teleop_node,
        line_follower_node,
        twist_mux_node
    ])
