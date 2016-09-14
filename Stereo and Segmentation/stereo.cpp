#include <iostream>
#include <assert.h>
#include <fstream>
#include <limits>
#include <map>
#include <math.h>
#include <vector>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

#define dir_up 0
#define dir_down 1
#define dir_left 2
#define dir_right 3

#define beta 5
#define DOUBLE_MAX numeric_limits<double>::max()


// ##########################################################################################
//                                      Help Function:
// ##########################################################################################

double sqr(double a) { return a*a; }

int encode(int x, int y, int w) { return x * w + y; }

class Node {
    public:
        double val;
        int max_disp;
        double *prior;
        double ** message[2];

        Node(double v, int n) : val(v), max_disp(n) {
            prior = new double[max_disp];

            message[0] = new double*[4];
            message[1] = new double*[4];
            message[0][0] = new double[max_disp];
            message[0][1] = new double[max_disp];
            message[0][2] = new double[max_disp];
            message[0][3] = new double[max_disp];
            message[1][0] = new double[max_disp];
            message[1][1] = new double[max_disp];
            message[1][2] = new double[max_disp];
            message[1][3] = new double[max_disp];

            for (int i = 0; i < max_disp; i++) {
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


// ##########################################################################################
//                                      Naive Stereo:
// ##########################################################################################

CImg<double> naive_stereo(const CImg<double> &input1, const CImg<double> &input2, int window_size, int max_disp) {  
    CImg<double> result(input1.width(), input1.height());

    for(int i = 0; i < input1.height(); i++) {
        for(int j = 0; j < input1.width(); j++) {
            pair<int, double> best_disp(0, INFINITY);
            for (int d = 0; d < max_disp; d++) {
                double cost = 0;
                for(int ii = max(i-window_size, 0); ii <= min(i+window_size, input1.height()-1); ii++)
                    for(int jj = max(j-window_size, 0); jj <= min(j+window_size, input1.width()-1); jj++)
                        cost += sqr(input1(min(jj+d, input1.width()-1), ii) - input2(jj, ii));

                if(cost < best_disp.second)
                    best_disp = make_pair(d, cost);
            }
            result(j, i) = best_disp.first;
        }
    }

    return result;
}


// ##########################################################################################
//                                      MRF Stereo:
// ##########################################################################################

// THE CODE BELOW THIS POINT WAS TAKEN FROM DAVID CRANDALL
// Distance transform: Linear
void dt(const double *src, double *dst, double *dst_ind, int s1, int s2, int d1, int d2, double scale, int off=0) {
    int d = (d1+d2) >> 1;
    int s = s1;
    for (int p = s1; p <= s2; p++)
        if (src[s] + abs(s-d-off) * scale> src[p] + abs(p-d-off) * scale)
            s = p;
    dst[d] = src[s] + abs(s-d-off) * scale;
    dst_ind[d] = s;

    if(d-1 >= d1)
        dt(src, dst, dst_ind, s1, s, d1, d-1, scale, off);
    if(d2 >= d+1)
        dt(src, dst, dst_ind, s, s2, d+1, d2, scale, off);
}

void dt_1d(const double *f, double scale, double *result, double *dst_ind, int beg, int end, int off=0) {
    dt(f, result, dst_ind, beg, end-1, beg, end-1, scale, off);
}
// END CODE FROM DAVID CRANDALL

void set_message(Node* nd_me, int dir_me, int loop, vector<double> *incomes, int max_disp) {
    int next = loop + 1;
    double message[max_disp];
    double path[max_disp];
    double temp_incomes[max_disp];
    for (int i = 0; i < max_disp; i++) {
        message[i] = 0.0;
        path[i] = 0.0;
        temp_incomes[i] = 0.0;
    }

    double avg_val = 0.0;
    double min_val = DOUBLE_MAX;

    // Get messages from three other directions
    for (int i = 0; i < max_disp; i++) {
        message[i] += nd_me->prior[i];
        for (int dir = 0; dir < 4; dir++) {
            if (dir != dir_me)
                message[i] += beta * incomes[dir][i];
        }
    }

    dt_1d(message, beta, temp_incomes, path, 0, max_disp-1);

    for (int i = 0; i < max_disp; i++) {
        // temp_incomes[i] += nd_me->prior[i];
        avg_val += temp_incomes[i];
    }

    avg_val /= max_disp;
    for (int i = 0; i < max_disp; i++) {
        nd_me->message[next%2][dir_me][i] = temp_incomes[i] - avg_val;
    }

}

CImg<double> mrf_stereo(const CImg<double> &input1, const CImg<double> &input2, int window_size, int max_disp) {
    int width = input1.width();
    int height = input1.height();
    CImg<double> result(width, height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            result(j, i) = 0.0;
        }
    }

    int states[max_disp];
    for (int i = 0; i < max_disp; i++) {
        states[i] = i;
    }

    // Set graph
    vector<Node *> matrix;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            matrix.push_back(new Node(input1(j,i), max_disp));
        }
    }

    // Set prior
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            for (int d = 0; d < max_disp; d++) {
                double cost = 0;
                for(int ii = max(i-window_size, 0); ii <= min(i+window_size, height-1); ii++)
                    for(int jj = max(j-window_size, 0); jj <= min(j+window_size, width-1); jj++)
                        cost += sqr(input1(min(jj+d, width-1), ii) - input2(jj, ii));

                int me = encode(i, j, width);
                matrix[me]->prior[d] = sqrt(cost);
            }
        }
    }

    // Set incomes
    vector<double> incomes[4];
    incomes[0] = vector<double>(max_disp, 0.0);
    incomes[1] = vector<double>(max_disp, 0.0);
    incomes[2] = vector<double>(max_disp, 0.0);
    incomes[3] = vector<double>(max_disp, 0.0);

    // Begin loop
    int max_loop = 20;
    int loop = 0;
    while (loop <= max_loop) {
        cout << "Loop " << loop << endl;
        for (int i = 1; i < height-1; i++) {
            for (int j = 1; j < width-1; j++) {
                int center = encode(i, j, width);
                int up = encode(i-1, j, width);
                int down = encode(i+1, j, width);
                int left = encode(i, j-1, width);
                int right = encode(i, j+1, width);

                for (int k = 0; k < max_disp; k++) {
                    incomes[dir_up][k] = matrix[up]->get_msg(loop, dir_down, k);
                    incomes[dir_down][k] = matrix[down]->get_msg(loop, dir_up, k);
                    incomes[dir_left][k] = matrix[left]->get_msg(loop, dir_right, k);
                    incomes[dir_right][k] = matrix[right]->get_msg(loop, dir_left, k);
                }

                set_message(matrix[center], dir_up, loop, incomes, max_disp);
                set_message(matrix[center], dir_down, loop, incomes, max_disp);
                set_message(matrix[center], dir_left, loop, incomes, max_disp);
                set_message(matrix[center], dir_right, loop, incomes, max_disp);
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
            for (int k = 0; k < max_disp; k++) {
                temp = matrix[center]->prior[k];
                temp += beta * matrix[up]->get_msg(loop, dir_down, k);
                temp += beta * matrix[down]->get_msg(loop, dir_up, k);
                temp += beta * matrix[left]->get_msg(loop, dir_right, k);
                temp += beta * matrix[right]->get_msg(loop, dir_left, k);

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



int main(int argc, char *argv[]) {
    if(argc != 4 && argc != 3) {
        cerr << "usage: " << argv[0] << " image_file1 image_file2 [gt_file]" << endl;
        return 1;
    }

    string input_filename1 = argv[1];
    string input_filename2 = argv[2];

    string gt_filename;
    if(argc == 4)
        gt_filename = argv[3];

    CImg<double> image1(input_filename1.c_str());
    CImg<double> image2(input_filename2.c_str());
    CImg<double> gt;

    if(gt_filename != "") {
        gt = CImg<double>(gt_filename.c_str());

        for(int i = 0; i < gt.height(); i++)
            for(int j = 0; j < gt.width(); j++)
                gt(j, i) = gt(j, i) / 3.0;
    }

    // Do naive stereo (matching only, no MRF)
    CImg<double> naive_disp = naive_stereo(image1, image2, 5, 50);
    naive_disp.get_normalize(0,255).save((input_filename1 + "-disp_naive.png").c_str());

    // Do stereo using mrf
    CImg<double> mrf_disp = mrf_stereo(image1, image2, 5, 50);
    mrf_disp.get_normalize(0,255).save((input_filename1 + "-disp_mrf.png").c_str());

    // Measure error with respect to ground truth, if we have it ...
    if(gt_filename != "") {
        cout << "Naive stereo technique mean error = " << (naive_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;
        cout << "MRF stereo technique mean error = " << (mrf_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;
    }

    return 0;
}

