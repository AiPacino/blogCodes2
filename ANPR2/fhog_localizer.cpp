#include "fhog_localizer.hpp"

#include <ocv_libs/core/attribute.hpp>
#include <ocv_libs/core/resize.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

license_plate_localizer::license_plate_localizer()
{

}

void license_plate_localizer::
localize_plate(const cv::Mat &input,
               std::vector<cv::Rect> &regions)
{
    preprocess(input);
    filter_background();
    find_plate_contours();
}

void license_plate_localizer::set_show_debug_message(bool value)
{
    debug_ = value;
}

void license_plate_localizer::filter_background()
{
    morphology_filter();
    //sobel_filter();
}

void license_plate_localizer::find_plate_contours()
{
    cv::findContours(binary_input_, contours_,
                     CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_SIMPLE);
    ocv::print_contour_attribute_name(std::cout);
    auto it = std::remove_if(std::begin(contours_), std::end(contours_),
                             [](auto const &contour)
    {
        return cv::boundingRect(contour).area() < 2000;
    });
    contours_.erase(it, std::end(contours_));

    cv::Mat input_copy;
    for(auto const &contour : contours_){
        int const Thickness = 2;
        resize_input_.copyTo(input_copy);
        cv::rectangle(input_copy, cv::boundingRect(contour),
        {255,0,0}, Thickness);
        if(debug_){
            ocv::print_contour_attribute(contour, 0.02, std::cout);
            cv::imshow("contour", input_copy);
            cv::imshow("region", resize_input_(cv::boundingRect(contour)));
            cv::waitKey();
        }
    }
}

void license_plate_localizer::morphology_filter()
{    
    //use tophat operations to filter the white background
    //of the license plate
    auto const tophat_kernel =
            cv::getStructuringElement(cv::MORPH_RECT, {30,10});
    cv::morphologyEx(intensity_, intensity_, CV_MOP_TOPHAT,
                     tophat_kernel);

    //use sobel filter to get the horizontal edges
    cv::Sobel(intensity_, intensity_, CV_8U, 1, 0, 3);

    //filter out some unwanted noise
    cv::GaussianBlur(intensity_, intensity_, {5, 5}, 0);

    //fill up the gaps of license plate
    auto const close_kernel =
            cv::getStructuringElement(cv::MORPH_RECT, {15,5});
    cv::morphologyEx(intensity_, intensity_, CV_MOP_CLOSE,
                     close_kernel);

    //generate threshold by
    cv::threshold(intensity_, binary_input_, 0, 255,
                  CV_THRESH_BINARY | CV_THRESH_OTSU);

    if(debug_){
        cv::imshow("intensity", intensity_);
        cv::imshow("binary input", binary_input_);
        cv::waitKey();
    }
}

void license_plate_localizer::preprocess(const cv::Mat &input)
{    
    CV_Assert(input.type() == CV_8UC3);

    ocv::resize_aspect_ratio(input, resize_input_, {640, 0});
    cv::cvtColor(resize_input_, hsv_, CV_BGR2HSV);
    cv::split(hsv_, split_hsv_);

    //the v channel of hsv could express the intensity
    //value better than the average weighted of 3 channels
    split_hsv_[2].copyTo(intensity_);
}

/**
 * For experiment(compare the results with morphology_filter)
 */
void license_plate_localizer::sobel_filter()
{
    cv::Sobel(intensity_, binary_input_, CV_8U, 0, 1, 3);
    cv::Mat threshold_input;
    cv::threshold(binary_input_, threshold_input, 0, 255,
                  CV_THRESH_BINARY | CV_THRESH_OTSU);
    auto const close_kernel =
            cv::getStructuringElement(cv::MORPH_RECT, {17,3});
    cv::morphologyEx(threshold_input, binary_input_,
                     CV_MOP_CLOSE, close_kernel);
    if(debug_){
        std::cerr<<"show image\n";
        cv::imshow("intensity", intensity_);
        cv::imshow("threshold_input", threshold_input);
        cv::imshow("binary_input_", binary_input_);
        cv::waitKey();
    }
}