#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <iostream>

class TeleopJoyNode : public rclcpp::Node {
public:
    TeleopJoyNode() : Node("teleop_joy_node") {
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

        // Subscribe ke topic /joy yang dipancarkan oleh stik PS4
        subscriber_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&TeleopJoyNode::joy_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "Sistem Kendali PS4 Aktif. Menunggu input joystick...");
    }

private:
    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr joy_msg) {
        auto twist_msg = geometry_msgs::msg::Twist();

        // Menggunakan Analog Kiri (Atas-Bawah) untuk maju-mundur (Axis index 1)
        // Menggunakan Analog Kanan (Kiri-Kanan) untuk belok (Axis index 3)
        // Nilai joystick berkisar dari -1.0 hingga 1.0

        double max_speed = 0.5; // Batas kecepatan maju 0.5 m/s
        double max_turn = 1.0;  // Batas kecepatan putar 1.0 rad/s

        twist_msg.linear.x = joy_msg->axes[1] * max_speed;
        twist_msg.angular.z = joy_msg->axes[3] * max_turn;

        // Note: Untuk Arm aktuator, kamu bisa menggunakan tombol X, O, Kotak, Segitiga
        // melalui array joy_msg->buttons[index_tombol] nantinya.

        publisher_->publish(twist_msg);
    }

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscriber_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TeleopJoyNode>());
    rclcpp::shutdown();
    return 0;
}
