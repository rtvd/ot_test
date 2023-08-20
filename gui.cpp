//
// Created by rtvd on 19/08/23.
//

#include <opencv4/opencv2/highgui.hpp>
#include "gui.h"

static void mouse_click_callback(int event, int x, int y, int flags, void *userdata) {
    if (event != cv::EVENT_LBUTTONUP)
        return;

    auto *frame_point = (cv::Point2i *) userdata;
    frame_point->x = x;
    frame_point->y = y;
}

bool get_mouse_click(cv::Point2i &point, const char *window_title) {
    point.x = -1;
    point.y = -1;
    cv::setMouseCallback(window_title, mouse_click_callback, &point);
    while (true) {
        if (point.x != -1 && point.y != -1)
            break;
        if (cv::waitKey(100) == 27) {
            cv::setMouseCallback(window_title, nullptr, nullptr);
            return false;
        }
    }
    cv::setMouseCallback(window_title, nullptr, nullptr);
    return true;
}