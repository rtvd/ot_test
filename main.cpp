#include <fstream>
#include <sstream>
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

struct BoundingBox {
    double x1, y1, x2, y2;
};

BoundingBox bounding_box_from_rect(const cv::Rect2d &roi, const cv::Size &frame_size) {
    return {
        roi.x / (double)frame_size.width,
        roi.y / (double)frame_size.height,
        (roi.x + roi.width) / (double)frame_size.width,
        (roi.y + roi.height) / (double)frame_size.height
    };
}

class BoundingBoxReporter {
private:
    std::ofstream  &log;

public:
    explicit BoundingBoxReporter(std::ofstream  &log): log(log) {
        log << "x1,y1,x2,y2" << std::endl;

    }

    void report(const BoundingBox &bb, bool tracking_ok) const {
        std::stringstream line;
        if (tracking_ok) {
            line << bb.x1 << "," << bb.y1 << "," << bb.x2 << "," << bb.y2 << std::endl;
        } else {
            line << ",,," << std::endl;
        }
        std::cout << line.str();
        log << line.str();
    }
};

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

    cv::VideoCapture src_video(src_file);
    if (!src_video.isOpened()) {
        fprintf(stderr, "Failed to open the source file.\n");
        return 1;
    }

    cv::Size frame_size(
            (int)src_video.get(cv::CAP_PROP_FRAME_WIDTH),
            (int)src_video.get(cv::CAP_PROP_FRAME_HEIGHT)
    );

    cv::namedWindow(WINDOW_TITLE, 1);

    // Read and show the first frame
    cv::Mat first_frame;
    src_video >> first_frame;
    if (first_frame.empty()) {
        fprintf(stderr, "The video had no frames!\n");
        return 1;
    }
    imshow(WINDOW_TITLE, first_frame);

    // Wait for the user to click at the point of interest
    cv::Point2i frame_point;
    if (!get_mouse_click(frame_point, WINDOW_TITLE)) {
        fprintf(stderr, "ESC pressed, terminating.\n");
        return 1;
    }
    const double width = std::min(frame_size.height, frame_size.width)*(roi_size/100.0); // ROI size is in percents
    cv::Rect2d roi(frame_point.x - width/2, frame_point.y - width/2, width, width);

    // Create a tracker and initialise it
    cv::Ptr<cv::Tracker> tracker = make_tracker_kcf();
    tracker->init(first_frame, roi);
    
    // Open the output log
    std::ofstream log;
    log.open(log_file, std::ios_base::trunc | std::ios_base::out);
    BoundingBoxReporter bb_reporter(log);

    // Begin writing the video
    int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    double video_fps = src_video.get(cv::CAP_PROP_FPS);
    cv::VideoWriter dst_video(dst_file, codec, video_fps, frame_size, true);

    // Process all frames one by one
    bool tracking_ok = true;
    cv::Mat frame;
    BoundingBox bb(bounding_box_from_rect(roi, frame_size));
    bb_reporter.report(bb, tracking_ok);

    while (true) {
        src_video >> frame;
        if (frame.empty())
            break;

        if (tracking_ok && !tracker->update(frame, roi)) {
            tracking_ok = false;
        }
        if (tracking_ok) {
            bb = bounding_box_from_rect(roi, frame_size);

            std::stringstream overlay_text;
            overlay_text << "ROI's bounding box: (" << bb.x1 << ";" << bb.y1 << ")" <<
                " to (" << bb.x2 << ";" << bb.y2 << ")";
            cv::displayOverlay(WINDOW_TITLE, overlay_text.str(), 0);

            // paint marker on the fram
            cv::Rect2d marker(roi);
            cv::rectangle(frame, marker, cv::Scalar(0., 0., 0.), 1, cv::LINE_8, 0);
            marker.x --;
            marker.y --;
            marker.width += 2;
            marker.height += 2;
            cv::rectangle(frame, marker, cv::Scalar(255., 255., 255.), 1, cv::LINE_8, 0);
        } else {
            cv::displayOverlay(WINDOW_TITLE, "Tracking lost", 0);
        }

        bb_reporter.report(bb, tracking_ok);
        dst_video.write(frame);
        imshow(WINDOW_TITLE, frame);

        const int key = cv::waitKey(10);
        if (key == 27) {
            fprintf(stderr, "ESC was pressed.");
            break;
        }
    }

    log.close();
    dst_video.release();
    cv::destroyWindow(WINDOW_TITLE);

    return 0;
}
