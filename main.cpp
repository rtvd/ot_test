#include <cstdio>
#include <algorithm>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/tracking.hpp>

#include "gui.h"

static const char *const WINDOW_TITLE = "Object Tracking Test";

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Please run the program like this:\n");
        fprintf(stderr, "./ot_test inputvideo.mp4\n\n");
        return 1;
    }

    const char *src_file = argv[1];
    printf("Reading video from file '%s' ...\n", src_file);
    cv::VideoCapture src_video(src_file);
    if (!src_video.isOpened()) {
        fprintf(stderr, "Failed to open the source file.\n");
        return 1;
    }

    const int frame_width_px = (int)src_video.get(cv::CAP_PROP_FRAME_WIDTH);
    const int frame_height_px = (int)src_video.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Video's frame size is %d x %d.\n", frame_width_px, frame_height_px);

    cv::namedWindow(WINDOW_TITLE, 1);

    // Read and show the first frame
    cv::Mat first_frame;
    src_video >> first_frame;
    if (first_frame.empty()) {
        fprintf(stderr, "The video had no frames!\n");
        return 1;
    }
    src_video.set(cv::CAP_PROP_POS_FRAMES, 0);  // rewind back
    imshow(WINDOW_TITLE, first_frame);

    // Wait for the user to click at the point of interest
    cv::Point2i frame_point;
    if (!get_mouse_click(frame_point, WINDOW_TITLE)) {
        fprintf(stderr, "ESC pressed, terminating.\n");
        return 1;
    }
    printf("Clicked at %d x %d.\n", frame_point.x, frame_point.y);
    const double width = std::min(frame_height_px, frame_width_px)/20.0; // 5%
    cv::Rect2d roi(frame_point.x - width/2, frame_point.y - width/2, width, width);
    printf("Initial ROI: (%.1f;%.1f) to (%.1f;%.1f).\n",
           roi.x, roi.y, roi.x + roi.width, roi.y + roi.height);

    // Create a tracker
    cv::Ptr<cv::Tracker> tracker = cv::TrackerKCF::create();

    // Process all frames one by one
    int n_frames_read = 0;
    cv::Mat frame;
    while (true) {
        src_video >> frame;
        if (frame.empty())
            break;
        n_frames_read ++;
        cv::rectangle(frame, roi, cv::Scalar(0.9, 0.8, 0.7), 1, cv::LINE_8, 0);
        imshow(WINDOW_TITLE, frame);

        const int key = cv::waitKey(1000);
        if (key == 27) {
            printf("ESC was pressed.");
            break;
        }
    }

    printf("Done. Processed %d frames.\n", n_frames_read);
    cv::destroyWindow(WINDOW_TITLE);

    return 0;
}
