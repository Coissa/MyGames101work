#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
constexpr double MY_PI = 3.1415926;
//绕y旋转会崩溃 很奇怪
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix_any(float rotation_angle, float x, float y, float z)//以向量(x ,y ,z)为轴，进行 rotation_angle 弧度的旋转
{
    float l = sqrt(x * x + y * y + z * z);
    x = x / l;
    y = y / l;
    z = z / l;
    Eigen::Vector4f v(x, y, z, 0);
    Eigen::Matrix4f M = v * v.transpose();
    float a = rotation_angle;
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f N = Eigen::Matrix4f::Identity();
    N << 0, -z, y, 0,
        z, 0, x, 0,
        -y, x, 0, 0,
        0, 0, 0, 0;
    Eigen::Matrix4f I = Eigen::Matrix4f::Identity();
    I << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 / cosf(a);
    model = cosf(a) * I + (1 - cosf(a)) * M + sinf(a) * N;
    return model;
}
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    float sita = rotation_angle;
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    model << cosf(sita), -sinf(sita), 0, 0,
        sinf(sita), cosf(sita), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function
    float n = zNear;
    float f = zFar;
    float headfov = eye_fov * MY_PI / 2.0f/ 180.0f;
    float y = n * tanf(headfov);
    float x = y * aspect_ratio;
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f b = Eigen::Matrix4f::Identity();
    b << 1 / x, 0, 0, 0,
        0, 1 / y, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    projection << n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, -n * f,
        0, 0, 1, 0;
    projection = projection * b;
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix_any(angle, 0, 1, 3));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }
   
    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix_any(angle,0,1,3));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 0.1 * MY_PI;
        }
        else if (key == 'd') {
            angle -= 0.1 * MY_PI;
        }
    }

    return 0;
}
