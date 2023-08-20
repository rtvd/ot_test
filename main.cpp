#include <cstdio>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/videoio.hpp>

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

    // Process all frames one by one
    int n_frames_read = 0;
    cv::Mat frame;
    while (true) {
        src_video >> frame;
        if (frame.empty())
            break;
        n_frames_read ++;
        cv::rectangle(frame, cv::Rect(10, 20, 60, 100), cv::Scalar(0.9, 0.8, 0.7), 1, cv::LINE_8, 0);
        imshow(WINDOW_TITLE, frame);

        const int key = cv::waitKey(1);
        if (key == 27) {
            printf("ESC was pressed.");
            break;
        }
    }

    printf("Done. Processed %d frames.\n", n_frames_read);
    cv::destroyWindow(WINDOW_TITLE);

    return 0;
}
