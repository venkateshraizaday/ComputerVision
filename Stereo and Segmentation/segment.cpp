// Skeleton code for B657 A4 Part 2.
// D. Crandall
//
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <limits>
#include "CImg.h"
#include <assert.h>

using namespace cimg_library;
using namespace std;

#define dir_up 0
#define dir_down 1
#define dir_left 2
#define dir_right 3
#define DOUBLE_MAX numeric_limits<double>::max()


class Point {
    public:
        Point() {}
        Point(int _col, int _row) : row(_row), col(_col) {}
        int row, col;
};

class Node {
    public:
        double r, g, b;
        double *prior;
        double ** message[2];

        Node(double r, double g, double b) : r(r), g(g), b(b) {
            prior = new double[2];

            message[0] = new double*[4];
            message[1] = new double*[4];
            message[0][0] = new double[2];
            message[0][1] = new double[2];
            message[0][2] = new double[2];
            message[0][3] = new double[2];
            message[1][0] = new double[2];
            message[1][1] = new double[2];
            message[1][2] = new double[2];
            message[1][3] = new double[2];

            for (int i = 0; i < 2; i++) {
                message[0][0][i] = message[0][1][i] = message[0][2][i] = message[0][3][i] = 0.0;
                message[1][0][i] = message[1][1][i] = message[1][2][i] = message[1][3][i] = 0.0;
            }
        }

        ~Node() {
            delete [] prior;
            delete [] message[0][0];
            delete [] message[0][1];
            delete [] message[0][2];
            delete [] message[0][3];
            delete [] message[1][0];
            delete [] message[1][1];
            delete [] message[1][2];
            delete [] message[1][3];
            delete [] message[0];
            delete [] message[1];
        }

        double get_msg(int loop, int dir, int dist) {
            return message[loop%2][dir][dist];
        }

        void set_msg(int loop, int dir, int dist, double val) {
            message[loop%2][dir][dist] = val;
        }
};

int encode(int x, int y, int width) {
    return x * width + y;
}

double cost_func(double r, double g, double b, double rmean, double gmean, double bmean, double rvar, double gvar, double bvar) {
    double vr = pow(r-rmean, 2.0) / rvar;
    double vg = pow(g-gmean, 2.0) / gvar;
    double vb = pow(b-bmean, 2.0) / bvar;
    return (vr+vg+vb);
}

CImg<double> naive_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg) {
    int width = img.width();
    int height = img.height();
    CImg<double> result(width, height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            result(i, j) = 2;
        }
    }

    double beta = 5.0;
    double rmean = 0.0;
    double gmean = 0.0;
    double bmean = 0.0;
    double rvar = 0.0;
    double gvar = 0.0;
    double bvar = 0.0;
    for (int i = 0; i < fg.size(); i++) {
        rmean += img(fg[i].col, fg[i].row, 0, 0);
        gmean += img(fg[i].col, fg[i].row, 0, 1);
        bmean += img(fg[i].col, fg[i].row, 0, 2);
    }
    rmean /= fg.size();
    gmean /= fg.size();
    bmean /= fg.size();
    for (int i = 0; i < fg.size(); i++) {
        rvar += pow(img(fg[i].col, fg[i].row, 0, 0)-rmean, 2);
        gvar += pow(img(fg[i].col, fg[i].row, 0, 1)-gmean, 2);
        bvar += pow(img(fg[i].col, fg[i].row, 0, 2)-bmean, 2);
    }
    rvar /= fg.size();
    gvar /= fg.size();
    bvar /= fg.size();

    for (int i = 0; i < fg.size(); i++) {
        result(fg[i].col, fg[i].row) = 1;
    }
    for (int i = 0; i < bg.size(); i++) {
        result(bg[i].col, bg[i].row) = 0;
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (result(i, j) != 0 && result(i, j) != 1) {
                double f = cost_func(img(i, j, 0, 0), img(i, j, 0, 1), img(i, j, 0, 2), rmean, gmean, bmean, rvar, gvar, bvar);
                if (f < beta)
                    result(i, j) = 1;
                else
                    result(i, j) = 0;
            }
        }
    }

    return result;
}

void set_message(Node* nd_me, int dir_me, int loop, vector<double> *incomes) {
    int next = loop + 1;
    vector<double> temp_incomes(2, 0.0);
    double avg_val = 0.0;
    double min_val = DOUBLE_MAX;

    for (int i = 0; i < 2; i++) {
        min_val = DOUBLE_MAX;
        for (int j = 0; j < 2; j++) {
            double msg = 0.0;
            msg += nd_me->prior[j];

            if (i != j)
                msg++;

            for (int dir = 0; dir < 4; dir++) {
                if(dir != dir_me)
                    msg += incomes[dir][j];
            }

            if (msg < min_val)
                min_val = msg;
        }

        avg_val += min_val;
        temp_incomes[i] = min_val;
    }

    avg_val /= 2;
    // Normalize
    for (int i = 0; i < 2; i++) {
        nd_me->message[next%2][dir_me][i] = temp_incomes[i] - avg_val;
    }
}

CImg<double> mrf_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg) {
    int width = img.width();
    int height = img.height();
    vector<Node *> matrix;
    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) {
            matrix.push_back(new Node(img(i, j, 0, 0), img(i, j, 0, 1), img(i, j, 0, 2)));
        }
    }
    double states[2] = {0.0, 1.0};
    vector<double> incomes[4];
    incomes[0] = vector<double>(2, 0.0);
    incomes[1] = vector<double>(2, 0.0);
    incomes[2] = vector<double>(2, 0.0);
    incomes[3] = vector<double>(2, 0.0);

    // Compute mean, variance and beta
    double beta = 5.0;
    double rmean = 0.0;
    double gmean = 0.0;
    double bmean = 0.0;
    double rvar = 0.0;
    double gvar = 0.0;
    double bvar = 0.0;
    for (int i = 0; i < fg.size(); i++) {
        rmean += img(fg[i].col, fg[i].row, 0, 0);
        gmean += img(fg[i].col, fg[i].row, 0, 1);
        bmean += img(fg[i].col, fg[i].row, 0, 2);
    }
    rmean /= fg.size();
    gmean /= fg.size();
    bmean /= fg.size();
    for (int i = 0; i < fg.size(); i++) {
        rvar += pow(img(fg[i].col, fg[i].row, 0, 0)-rmean, 2);
        gvar += pow(img(fg[i].col, fg[i].row, 0, 1)-gmean, 2);
        bvar += pow(img(fg[i].col, fg[i].row, 0, 2)-bmean, 2);
    }
    rvar /= fg.size();
    gvar /= fg.size();
    bvar /= fg.size();

    CImg<double> result(width, height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            result(i, j) = 2;
        }
    }

    for (int i = 0; i < fg.size(); i++) {
        result(fg[i].col, fg[i].row) = 1;
    }
    for (int i = 0; i < bg.size(); i++) {
        result(bg[i].col, bg[i].row) = 0;
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int me = encode(j, i, width);
            if (result(i, j) == 0) {
                matrix[me]->prior[0] = 0;
                matrix[me]->prior[1] = DOUBLE_MAX;
            }else if (result(i, j) == 1) {
                matrix[me]->prior[0] = DOUBLE_MAX;
                matrix[me]->prior[1] = 0;
            }else {
                matrix[me]->prior[0] = beta;
                matrix[me]->prior[1] = cost_func(img(i, j, 0, 0), img(i, j, 0, 1), img(i, j, 0, 2), rmean, gmean, bmean, rvar, gvar, bvar);
            }
        }
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            result(i, j) = 0;
        }
    }

    int max_loop = 20;
    int loop = 1;
    while (loop <= max_loop) {
        for (int x = 1; x < height-1; x++) {
            for (int y = 1; y < width-1; y++) {
                int center = encode(x, y, width);
                int up = encode(x-1, y, width);
                int down = encode(x+1, y, width);
                int left = encode(x, y-1, width);
                int right = encode(x, y+1, width);

                for (int k = 0; k < 2; k++) {
                    incomes[dir_up][k] = matrix[up]->get_msg(loop, dir_down, k);
                    incomes[dir_down][k] = matrix[down]->get_msg(loop, dir_up, k);
                    incomes[dir_left][k] = matrix[left]->get_msg(loop, dir_right, k);
                    incomes[dir_right][k] = matrix[right]->get_msg(loop, dir_left, k);
                }

                set_message(matrix[center], dir_up, loop, incomes);
                set_message(matrix[center], dir_down, loop, incomes);
                set_message(matrix[center], dir_left, loop, incomes);
                set_message(matrix[center], dir_right, loop, incomes);
            }
        }
        loop++;
    }

    // Set results
    double min_val = DOUBLE_MAX;
    double temp = 0.0;
    int index = -1;
    for (int i = 1; i < height-1; i++) {
        for (int j = 1; j < width-1; j++) {
            int center = encode(i, j, width);
            int up = encode(i-1, j, width);
            int down = encode(i+1, j, width);
            int left = encode(i, j-1, width);
            int right = encode(i, j+1, width);
            min_val = DOUBLE_MAX;
            index = -1;
            for (int k = 0; k < 2; k++) {
                temp = matrix[center]->prior[k];
                temp += matrix[up]->get_msg(loop, dir_down, k);
                temp += matrix[down]->get_msg(loop, dir_up, k);
                temp += matrix[left]->get_msg(loop, dir_right, k);
                temp += matrix[right]->get_msg(loop, dir_left, k);

                if (temp < min_val) {
                    min_val = temp;
                    index = k;
                }
            }

            result(j, i) = states[index];
        }
    }

    return result;
}

void output_segmentation(const CImg<double> &img, const CImg<double> &labels, const string &fname) {
    assert(img.height() == labels.height());
    assert(img.width() == labels.width());

    CImg<double> img_fg = img, img_bg = img;

    for(int i=0; i<labels.height(); i++)
        for(int j=0; j<labels.width(); j++) {
            if(labels(j,i) == 0)
                img_fg(j,i,0,0) = img_fg(j,i,0,1) = img_fg(j,i,0,2) = 0;
            else if(labels(j,i) == 1)
                img_bg(j,i,0,0) = img_bg(j,i,0,1) = img_bg(j,i,0,2) = 0;
            else
                assert(0);
        }

    img_fg.get_normalize(0,255).save((fname + "_fg.png").c_str());
    img_bg.get_normalize(0,255).save((fname + "_bg.png").c_str());
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cerr << "usage: " << argv[0] << " image_file seeds_file" << endl;
        return 1;
    }

    string input_filename1 = argv[1], input_filename2 = argv[2];

    // read in images and gt
    CImg<double> image_rgb(input_filename1.c_str());
    CImg<double> seeds_rgb(input_filename2.c_str());

    // figure out seed points 
    vector<Point> fg_pixels, bg_pixels;
    for(int i=0; i<seeds_rgb.height(); i++)
        for(int j=0; j<seeds_rgb.width(); j++) {
            // blue --> foreground
            if(max(seeds_rgb(j, i, 0, 0), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 2) > 100)
                fg_pixels.push_back(Point(j, i));

            // red --> background
            if(max(seeds_rgb(j, i, 0, 2), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 0) > 100)
                bg_pixels.push_back(Point(j, i));
        }

    // do naive segmentation
    CImg<double> labels = naive_segment(image_rgb, fg_pixels, bg_pixels);
    output_segmentation(image_rgb, labels, input_filename1 + "-naive_segment_result");

    // do mrf segmentation
    labels = mrf_segment(image_rgb, fg_pixels, bg_pixels);
    output_segmentation(image_rgb, labels, input_filename1 + "-mrf_segment_result");

    return 0;
}
