#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>

class ArucoVisionNode : public rclcpp::Node {
public:
  ArucoVisionNode() : Node("vision_node") {
    // Inisialisasi publisher untuk mengirim ID lokasi tujuan ke topic
    // /box_target_id
    publisher_ =
        this->create_publisher<std_msgs::msg::Int32>("box_target_id", 10);

    // --- KONFIGURASI IP WEBCAM ---
    // Ganti 192.168.x.x dengan alamat IPv4 yang muncul di layar aplikasi IP
    // Webcam HP kamu. Pastikan ada imbuhan "/video" di bagian belakang URL.
    std::string ip_camera_url = "https://10.129.226.254:8080/video";

    cap_.open(ip_camera_url, cv::CAP_FFMPEG);
    if (!cap_.isOpened()) {
      cap_.set(cv::CAP_PROP_BUFFERSIZE, 1);
      RCLCPP_ERROR(
          this->get_logger(),
          "Kamera IP tidak terdeteksi! Cek koneksi WiFi dan kesesuaian IP.");
    } else {
      RCLCPP_INFO(this->get_logger(), "Berhasil terhubung ke Kamera IP HP.");
    }

    // Setup Aruco menggunakan sintaks OpenCV bawaan ROS 2 (cv::Ptr)
    dictionary_ = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000);
    parameters_ = cv::aruco::DetectorParameters::create();

    // Loop kamera berjalan di timer setiap 33 ms (~30 FPS)
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(33),
        std::bind(&ArucoVisionNode::timer_callback, this));

    RCLCPP_INFO(
        this->get_logger(),
        "Sistem Visi Aktif. Memublikasikan instruksi ke '/box_target_id'...");
  }

  ~ArucoVisionNode() {
    cap_.release();
    cv::destroyAllWindows();
  }

private:
  void timer_callback() {
    cv::Mat frame;
    // Menangkap frame dari IP Webcam via WiFi
    cap_ >> frame;
    if (frame.empty())
      return;

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;

    // Eksekusi fungsi deteksi marker
    cv::aruco::detectMarkers(frame, dictionary_, markerCorners, markerIds,
                             parameters_, rejectedCandidates);

    // Jika marker pada box terbaca
    if (!markerIds.empty()) {
      cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);

      // Memublikasikan angka tujuan box (1 dari 4 lokasi yang tersedia)
      auto msg = std_msgs::msg::Int32();
      msg.data = markerIds[0];
      publisher_->publish(msg);

      RCLCPP_INFO(this->get_logger(),
                  "Box terbaca! Publish ID lokasi tujuan: %d", msg.data);
    }

    // Tampilkan feed dari kamera HP ke layar laptop driver
    cv::imshow("Kamera IP Robot Transporter", frame);
    cv::waitKey(1); // Delay 1ms agar window OpenCV bisa merender gambar
  }

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  cv::VideoCapture cap_;
  cv::Ptr<cv::aruco::Dictionary> dictionary_;
  cv::Ptr<cv::aruco::DetectorParameters> parameters_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ArucoVisionNode>());
  rclcpp::shutdown();
  return 0;
}
