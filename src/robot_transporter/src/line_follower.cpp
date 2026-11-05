#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <std_msgs/msg/bool.hpp>

class LineFollowerNode : public rclcpp::Node {
public:
    LineFollowerNode() : Node("line_follower_node"), is_auto_finished_(false) {
        cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel_auto", 10);
        status_pub_ = this->create_publisher<std_msgs::msg::Bool>("/auto_status", 10);
        sensor_sub_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
            "/line_sensors", 10, 
            std::bind(&LineFollowerNode::sensor_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "Line Follower Node Aktif. Menunggu data sensor...");
    }

private:
    void sensor_callback(const std_msgs::msg::Int32MultiArray::SharedPtr msg) {
        if (is_auto_finished_) return;

        if (msg->data.size() < 3) {
            RCLCPP_WARN(this->get_logger(), "Data sensor tidak valid!");
            return;
        }

        int L = msg->data[0];
        int C = msg->data[1];
        int R = msg->data[2];

        geometry_msgs::msg::Twist twist;
        twist.linear.x = 0.0;
        twist.angular.z = 0.0;

        if (L == 1 && C == 1 && R == 1) {
            twist.linear.x = 0.0;
            twist.angular.z = 0.0;
            is_auto_finished_ = true;
            
            std_msgs::msg::Bool status_msg;
            status_msg.data = true;
            status_pub_->publish(status_msg);
            
            RCLCPP_INFO(this->get_logger(), "Target tercapai! Menunggu input PS4 (Mode Manual).");
        }
        else if (L == 0 && C == 1 && R == 0) {
            twist.linear.x = 0.15;
            twist.angular.z = 0.0;
        }
        else if ((L == 1 && C == 1 && R == 0) || (L == 1 && C == 0 && R == 0)) {
            twist.linear.x = 0.1;
            twist.angular.z = 0.5;
        }
        else if ((L == 0 && C == 1 && R == 1) || (L == 0 && C == 0 && R == 1)) {
            twist.linear.x = 0.1;
            twist.angular.z = -0.5;
        }

        cmd_pub_->publish(twist);
    }

    bool is_auto_finished_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr status_pub_;
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr sensor_sub_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LineFollowerNode>());
    rclcpp::shutdown();
    return 0;
}
