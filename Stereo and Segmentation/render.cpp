#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <math.h>
#include "CImg.h"
#include <assert.h>

using namespace cimg_library;
using namespace std;

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cerr << "usage: " << argv[0] << " image_file disp_file" << endl;
        return 1;
    }

    string input_filename1 = argv[1];
    string input_filename2 = argv[2];

    CImg<double> image_rgb(input_filename1.c_str());
    CImg<double> image_disp(input_filename2.c_str());
    CImg<double> image_disp_inverse = image_disp;
    CImg<double> image_result(image_rgb.width(), image_rgb.height(), 1, 3, 0);

    for (int i = 0; i < image_rgb.width(); i++) {
        for (int j = 0; j < image_rgb.height(); j++) {
            // Inverse value of disp image
            image_disp_inverse(i, j) = (int)(abs(image_disp_inverse(i, j) - 255)/5);
            // Assign all empty value to 255
            image_result(i, j) = 255;
        }
    }

    for (int j = 0; j < image_rgb.height(); j++) {
        for (int i = 0; i < image_rgb.width(); i++) {
            int ni = i + image_disp_inverse(i,j);
            if (ni <= image_rgb.width()) {
                image_result(ni, j, 0, 0) = image_rgb(i, j, 0, 0);
            }
            image_result(i, j, 0, 1) = image_rgb(i, j, 0, 1);
            image_result(i, j, 0, 2) = image_rgb(i, j, 0, 2);
        }
    }

    image_result.get_normalize(0,255).save((input_filename1 + "-stereogram.png").c_str());
    return 0;
}

