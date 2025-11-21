//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <cmath>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;
    
    Eigen::Vector3f getColor(float u, float v) {
        auto u_img= u * width;
        auto v_img = (1 - v) * height;
        int x = std::clamp((int)std::floor(u_img), 0, width - 1);
        int y = std::clamp((int)std::floor(v_img), 0, height - 1);
        float x1 = (float)x;
        float y1 = (float)y;
        cv::Vec3b c00 = image_data.at<cv::Vec3b>(y, x);
        cv::Vec3b c10 = image_data.at<cv::Vec3b>(y, x1);
        cv::Vec3b c01 = image_data.at<cv::Vec3b>(y1, x);
        cv::Vec3b c11 = image_data.at<cv::Vec3b>(y1, x1);
        float a = 0;
        float b = 0;
        if (x1 != x && y1 != y) {
            a = (u_img - x) / (x1 - x);
            b = (v_img - y) / (y1 - y);
        }
        else {
            return  Eigen::Vector3f(c00[0], c00[1], c00[2]);
        }
        cv::Vec3b t = ((c10 - c00) * a + c00);
        cv::Vec3b m = ((c11 - c01) * a + c01);
        cv::Vec3b color = t + (m - t) * b;
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    


    Eigen::Vector3f getColor1(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
};
#endif //RASTERIZER_TEXTURE_H
