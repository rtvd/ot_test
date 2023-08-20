#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/tracking.hpp>

#include "gui.h"

static const char *const WINDOW_TITLE = "Object Tracking Test";

cv::Ptr<cv::Tracker> make_tracker_mil() {
    return cv::TrackerMIL::create();
}

cv::Ptr<cv::Tracker> make_tracker_boosting() {
    return cv::TrackerBoosting::create();
}

cv::Ptr<cv::Tracker> make_tracker_median_flow() {
    return cv::TrackerMedianFlow::create();
}

cv::Ptr<cv::Tracker> make_tracker_tld() {
    return cv::TrackerTLD::create();
}

cv::Ptr<cv::Tracker> make_tracker_kcf() {
    return cv::TrackerKCF::create();
}

cv::Ptr<cv::Tracker> make_tracker_goturn() {
    return cv::TrackerGOTURN::create();
}

cv::Ptr<cv::Tracker> make_tracker_mosse() {
    return cv::TrackerMOSSE::create();
}

int main(int argc, char **argv) {
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Please run the program like this:\n");
        fprintf(stderr, "./ot_test <tracker> <inputvideo> <outputvideo> <logfile> [<ROI size>]\n\n");
        return 1;
    }

    const char *tracker_name = argv[1];
    const char *src_file = argv[2];
    const char *dst_file = argv[3];
    const char *log_file = argv[4];

    double roi_size = 5.0;
    if (argc == 6) {
        const char *roi_size_txt = argv[5];
        char *ptr = nullptr;
        roi_size = strtof(roi_size_txt, &ptr);
        if ((ptr - roi_size_txt) != strlen(roi_size_txt)) {
            fprintf(stderr, "Failed to parse ROI size '%s'\n", roi_size_txt);
            return 1;
        }
    }
    printf("ROI size: %f%%\n", roi_size);

    cv::Ptr<cv::Tracker> (*make_tracker)();
    if (strcmp(tracker_name, "MIL") == 0) {
        make_tracker = &make_tracker_mil;
    } else if (strcmp(tracker_name, "Boosting") == 0) {
        make_tracker = &make_tracker_boosting;
    } else if (strcmp(tracker_name, "MedianFlow") == 0) {
        make_tracker = &make_tracker_median_flow;
    } else if (strcmp(tracker_name, "TLD") == 0) {
        make_tracker = &make_tracker_tld;
    } else if (strcmp(tracker_name, "KCF") == 0) {
        make_tracker = &make_tracker_kcf;
    } else if (strcmp(tracker_name, "GOTURN") == 0) {
        make_tracker = &make_tracker_goturn;
    } else if (strcmp(tracker_name, "MOSSE") == 0) {
        make_tracker = &make_tracker_mosse;
    } else {
        fprintf(stderr, "Tracker '%s' is not supported.\n", tracker_name);
    }

    printf("Reading video from file '%s' ...\n", src_file);
    cv::VideoCapture src_video(src_file);
    if (!src_video.isOpened()) {
        fprintf(stderr, "Failed to open the source file.\n");
        return 1;
    }

    cv::Size frame_size(
            (int)src_video.get(cv::CAP_PROP_FRAME_WIDTH),
            (int)src_video.get(cv::CAP_PROP_FRAME_HEIGHT)
    );
    printf("Video's frame size is %d x %d.\n", frame_size.width, frame_size.height);

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
    const double width = std::min(frame_size.height, frame_size.width)*(roi_size/100.0); // ROI size is in percents
    cv::Rect2d roi(frame_point.x - width/2, frame_point.y - width/2, width, width);
    printf("Initial ROI: (%.1f;%.1f) to (%.1f;%.1f).\n",
           roi.x, roi.y, roi.x + roi.width, roi.y + roi.height);

    // Create a tracker and initialise it
    cv::Ptr<cv::Tracker> tracker = make_tracker_kcf();
    tracker->init(first_frame, roi);
    
    // Open the output log
    std::ofstream log;
    log.open(log_file, std::ios_base::trunc | std::ios_base::out);
    log << "frame,roi_x,roi_y,roi_w,roi_h" << std::endl;
    
    // Begin writing the video
    int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    double video_fps = src_video.get(cv::CAP_PROP_FPS);
    cv::VideoWriter dst_video(dst_file, codec, video_fps, frame_size, true);

    // Process all frames one by one
    bool tracking_ok = true;
    int n_frames_read = 0;
    cv::Mat frame;
    while (true) {
        src_video >> frame;
        if (frame.empty())
            break;
        n_frames_read ++;
        if (tracking_ok && !tracker->update(frame,roi)) {
            tracking_ok = false;
        }
        if (tracking_ok) {
            log << n_frames_read << "," << roi.x << "," << roi.y << "," << roi.width << "," << roi.height << std::endl;

            // paint marker on the fram
            cv::Rect2d marker(roi);
            cv::rectangle(frame, marker, cv::Scalar(0., 0., 0.), 1, cv::LINE_8, 0);
            marker.x --;
            marker.y --;
            marker.width += 2;
            marker.height += 2;
            cv::rectangle(frame, marker, cv::Scalar(255., 255., 255.), 1, cv::LINE_8, 0);
        } else {
            log << n_frames_read << ",,,," << std::endl;
        }

        dst_video.write(frame);
        imshow(WINDOW_TITLE, frame);

        const int key = cv::waitKey(10);
        if (key == 27) {
            printf("ESC was pressed.");
            break;
        }
    }

    printf("Done. Processed %d frames.\n", n_frames_read);
    log.close();
    dst_video.release();
    cv::destroyWindow(WINDOW_TITLE);

    return 0;
}
