#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>

class TeleopNode : public rclcpp::Node {
public:
  TeleopNode() : Node("teleop_node") {
    // Publisher untuk pergerakan roda (menuju twist_mux)
    cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
        "/cmd_vel_manual", 10);

    // Publisher untuk servo arm/gripper (menuju STM32)
    servo_pub_ = this->create_publisher<std_msgs::msg::Int32MultiArray>(
        "/servo_cmd", 10);

    // Subscriber untuk membaca input dari stik PS4
    joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "/joy", 10,
        std::bind(&TeleopNode::joy_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Teleop PS4 Node Aktif!");
  }

private:
  void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
    // --- 1. LOGIKA RODA (Analog Kiri) ---
    // Axes 1: Analog Kiri Atas/Bawah (Maju/Mundur)
    // Axes 0: Analog Kiri Kiri/Kanan (Belok)

    geometry_msgs::msg::Twist twist;
    // Kalikan dengan konstanta kecepatan maksimal yang diinginkan (misal 0.5
    // m/s)
    twist.linear.x = msg->axes[1] * 0.5;
    twist.angular.z = msg->axes[0] * 1.0;

    cmd_pub_->publish(twist);

    if (msg->axes[0] != 0.0 || msg->axes[1] != 0.0) {
      RCLCPP_INFO(this->get_logger(),
                  "Analog Kiri -> Maju/Mundur: %.2f | Belok: %.2f",
                  msg->axes[1], msg->axes[0]);
    }
    // --- 2. LOGIKA ARM & GRIPPER (Tombol Aksi) ---
    // Pemetaan standar PS4 (Bisa bervariasi tergantung driver OS):
    // Buttons 0: X (Silang)
    // Buttons 1: Lingkaran
    // Buttons 2: Segitiga
    // Buttons 3: Kotak
    // Buttons 4: L1
    // Buttons 5: R1

    std_msgs::msg::Int32MultiArray servo_msg;
    // Asumsi format array: [Sudut_Arm, Sudut_Gripper]
    int arm_angle = 90;     // Posisi default
    int gripper_angle = 90; // Posisi default

    // Logika Gripper (Capit)
    if (msg->buttons[0] == 1) { // Tekan X -> Tutup capit
      gripper_angle = 0;
      RCLCPP_INFO(this->get_logger(), "Gripper TUTUP");
    } else if (msg->buttons[3] == 1) { // Tekan Kotak -> Buka capit
      gripper_angle = 180;
      RCLCPP_INFO(this->get_logger(), "Gripper BUKA");
    }

    // Logika Arm (Naik/Turun)
    if (msg->buttons[5] == 1) { // Tekan R1 -> Angkat lengan
      arm_angle = 180;
      RCLCPP_INFO(this->get_logger(), "Arm NAIK");
    } else if (msg->buttons[4] == 1) { // Tekan L1 -> Turunkan lengan
      arm_angle = 0;
      RCLCPP_INFO(this->get_logger(), "Arm TURUN");
    }

    // Hanya publish perintah servo jika ada tombol yang ditekan (menghindari
    // spam ke mikrokontroler)
    if (msg->buttons[0] == 1 || msg->buttons[3] == 1 || msg->buttons[4] == 1 ||
        msg->buttons[5] == 1) {
      servo_msg.data = {arm_angle, gripper_angle};
      servo_pub_->publish(servo_msg);
    }
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr servo_pub_;
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TeleopNode>());
  rclcpp::shutdown();
  return 0;
}
