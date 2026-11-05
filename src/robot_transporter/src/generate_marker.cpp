#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <string>

int main() {
    cv::Ptr<cv::aruco::Dictionary> dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    for (int id = 1; id <= 4; id++) {
        cv::Mat marker;
        cv::aruco::drawMarker(dict, id, 400, marker, 1);

        // tambah tepi putih (wajib agar marker terbaca)
        cv::copyMakeBorder(marker, marker, 50, 50, 50, 50,
                           cv::BORDER_CONSTANT, cv::Scalar(255));

        cv::imwrite("marker_" + std::to_string(id) + ".png", marker);
    }
    return 0;
}
