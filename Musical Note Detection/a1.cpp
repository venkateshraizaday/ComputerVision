#include <SImage.h>
#include <SImageIO.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <DrawText.h>

using namespace std;

// The simple image class is called SDoublePlane, with each pixel represented as
// a double (floating point) type. This means that an SDoublePlane can represent
// values outside the range 0-255, and thus can represent squared gradient magnitudes,
// harris corner scores, etc.
//
// The SImageIO class supports reading and writing PNG files. It will read in
// a color PNG file, convert it to grayscale, and then return it to you in
// an SDoublePlane. The values in this SDoublePlane will be in the range [0,255].
//
// To write out an image, call write_png_file(). It takes three separate planes,
// one for each primary color (red, green, blue). To write a grayscale image,
// just pass the same SDoublePlane for all 3 planes. In order to get sensible
// results, the values in the SDoublePlane should be in the range [0,255].
//

// Below is a helper functions that overlays rectangles
// on an image plane for visualization purpose.

// Draws a rectangle on an image plane, using the specified gray level value and line width.
//
void overlay_rectangle(SDoublePlane &input, int _top, int _left, int _bottom, int _right, double graylevel, int width)
{
	for (int w = -width / 2; w <= width / 2; w++) {
		int top = _top + w, left = _left + w, right = _right + w, bottom =
				_bottom + w;

		// if any of the coordinates are out-of-bounds, truncate them
		top = min(max(top, 0), input.rows() - 1);
		bottom = min(max(bottom, 0), input.rows() - 1);
		left = min(max(left, 0), input.cols() - 1);
		right = min(max(right, 0), input.cols() - 1);

		// draw top and bottom lines
		for (int j = left; j <= right; j++)
			input[top][j] = input[bottom][j] = graylevel;
		// draw left and right lines
		for (int i = top; i <= bottom; i++)
			input[i][left] = input[i][right] = graylevel;
	}
}

// DetectedSymbol class may be helpful!
//  Feel free to modify.
//
typedef enum {NOTEHEAD=0, QUARTERREST=1, EIGHTHREST=2} Type;
class DetectedSymbol {
public:
	int row, col, width, height;
	Type type;
	char pitch;
	double confidence;
};

// Function that outputs the ascii detection output file
void  write_detection_txt(const string &filename, const vector<struct DetectedSymbol> &symbols)
{
	ofstream ofs(filename.c_str());

	for(int i=0; i<symbols.size(); i++)
	{
		const DetectedSymbol &s = symbols[i];
		ofs << s.row << " " << s.col << " " << s.width << " " << s.height << " ";
		if(s.type == NOTEHEAD)
			ofs << "filled_note " << s.pitch;
		else if(s.type == EIGHTHREST)
			ofs << "eighth_rest _";
		else
			ofs << "quarter_rest _";
		ofs << " " << s.confidence << endl;
	}
}

// Function that outputs a visualization of detected symbols
void  write_detection_image(const string &filename, const vector<DetectedSymbol> &symbols, const SDoublePlane &input)
{
	SDoublePlane output_planes[3];
	for(int i=0; i<3; i++)
		output_planes[i] = input;

	for(int i=0; i<symbols.size(); i++)
	{
		const DetectedSymbol &s = symbols[i];

		overlay_rectangle(output_planes[s.type], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 255, 2);
		overlay_rectangle(output_planes[(s.type+1) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);
		overlay_rectangle(output_planes[(s.type+2) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);

		if(s.type == NOTEHEAD)
		{
			char str[] = {s.pitch, 0};
			draw_text(output_planes[0], str, s.row, s.col+s.width+1, 0, 2);
			draw_text(output_planes[1], str, s.row, s.col+s.width+1, 0, 2);
			draw_text(output_planes[2], str, s.row, s.col+s.width+1, 0, 2);
		}
	}

	SImageIO::write_png_file(filename.c_str(), output_planes[0], output_planes[1], output_planes[2]);
}


// bound the image on four borders.

int bound(int value, int max)
{
	if(value < 0) return 0;
	if(value < max) return value;
	return max - 1;
}


// The rest of these functions are incomplete. These are just suggestions to
// get you started -- feel free to add extra functions, change function
// parameters, etc.

// Convolve an image with a separable convolution kernel
//
SDoublePlane convolve_separable(const SDoublePlane &input, const SDoublePlane &row_filter, const SDoublePlane &col_filter, double r_count, double c_count)
{
	// Convolution code here
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	int rowWidth = row_filter.cols();
	int rowRadius = (rowWidth - 1) / 2;
	int colHeight = col_filter.rows();
	int colRadius = (colHeight - 1) / 2;

	SDoublePlane row_output(inputHeight, inputWidth);
	SDoublePlane col_output(inputHeight, inputWidth);

	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			double sum = 0;
			for(int r = 0; r < rowWidth; r++) {
				sum += input[i][bound((j - rowRadius + r), inputWidth)] * row_filter[0][r];
			}
			row_output[i][j] = sum / r_count;
		}
	}
	for(int j = 0; j < inputWidth; j++) {
		for(int i = 0; i < inputHeight; i++) {
			double sum = 0;
			for(int c = 0; c < colHeight; c++) {
				sum += row_output[bound((i - colRadius + c), inputHeight)][j] * col_filter[c][0];
			}
			col_output[i][j] = sum / c_count;
		}
	}

	return col_output;
}


// Convolve an image with a separable convolution kernel
// I(i - u, j - v) * K(u, v) (u \in [-1,1]), here we set u \in [0,3], thus u should be transplanted to (u-radius)
SDoublePlane convolve_general(const SDoublePlane &input, const SDoublePlane &filter)
{
	// Convolution code here
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	int filterHeight = filter.rows();
	int filterWidth = filter.cols();
	int widthRadius = (filterWidth - 1) / 2;
	int heightRadius = (filterHeight - 1) / 2;
	SDoublePlane output(inputHeight, inputWidth);

/*
 *  int count = 0;
	for(int u = 0; u < filterHeight; u++) {
		for(int v = 0; v < filterWidth; v++) {
			count += filter[u][v];
		}
	}
*/
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			double sum = 0.0;
			for(int u = 0; u < filterHeight; u++) {
				for(int v = 0; v < filterWidth; v++) {
					sum += abs(filter[u][v] - input[bound(i - u + heightRadius, inputHeight)][bound(j - v + widthRadius, inputWidth)]);
				}
			}
			output[i][j] = sum;
		}
	}
	return output;
}

// Apply a sobel operator to an image, returns the result
//
SDoublePlane sobel_gradient_filter(const SDoublePlane &input, bool _gx)
{
	// Implement a sobel gradient estimation filter with 1-d filters
	// output sqrt{dx^2 + dy^2}
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	SDoublePlane output(inputHeight, inputWidth);
	SDoublePlane x_output(inputHeight, inputWidth);
	SDoublePlane y_output(inputHeight, inputWidth);
	SDoublePlane x_row_filter(1,3);
	SDoublePlane x_col_filter(3,1);
	SDoublePlane y_row_filter(1,3);
	SDoublePlane y_col_filter(3,1);
	//double data[1][3] = {{1, 0, -1}};
	x_row_filter[0][0] = 1.0; x_row_filter[0][1] = 0; x_row_filter[0][2] = -1.0;
	x_col_filter[0][0] = 1.0; x_col_filter[1][0] = 2.0; x_col_filter[2][0] = 1.0;
	y_row_filter[0][0] = 1.0; y_row_filter[0][1] = 2.0; y_row_filter[0][2] = 1.0;
	y_col_filter[0][0] = 1.0; y_col_filter[1][0] = 0; y_col_filter[2][0] = -1.0;

	x_output = convolve_separable(input, x_row_filter, x_col_filter, 2, 4);
	y_output = convolve_separable(input, y_row_filter, y_col_filter, 4, 2);
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			output[i][j] = sqrt(pow(x_output[i][j], 2) + pow(y_output[i][j], 2));
		}
	}

	return output;
}

// Apply an edge detector to an image, returns the binary edge map
//
SDoublePlane find_edges(const SDoublePlane &input, double thresh=0)
{
	// Implement an edge detector of your choice, e.g.
	// use your sobel gradient operator to compute the gradient magnitude and threshold
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	SDoublePlane edge_map(inputHeight, inputWidth);
	edge_map = sobel_gradient_filter(input, true);
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			if(edge_map[i][j] > thresh) {
				edge_map[i][j] = 1;
			} else {
				edge_map[i][j] = 0;
			}
		}
	}
	return edge_map;
}


SDoublePlane visual_edge_map(const SDoublePlane &input, double thresh=0)
{
	//visualize the binary edge map.
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	SDoublePlane output(inputHeight, inputWidth);
	output = find_edges(input, thresh);
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			if(output[i][j] == 1) {
				output[i][j] = 0;
			} else {
				output[i][j] = 255;
			}
		}
	}
	return output;
}


SDoublePlane normalize(SDoublePlane &input, double scale)
{
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	double max = 0;
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			if(input[i][j] > max) {
				max = input[i][j];
			}
		}
	}
	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			input[i][j] = (input[i][j] / max ) * scale;
		}
	}
	return input;
}


SDoublePlane q_five_score(const SDoublePlane &input, const SDoublePlane &tem, double edge_thresh)
{
	int inputHeight = input.rows();
	int inputWidth = input.cols();
	int temHeight = tem.rows();
	int temWidth = tem.cols();
	double dis_max = sqrt(pow(inputWidth, 2) + pow(inputHeight, 2));
	double MElr[4] = {1, sqrt(2), 1, sqrt(2)};
	double d1, d2, d3, d4;
	double sum;

	SDoublePlane input_edge(inputHeight, inputWidth);
	SDoublePlane tem_edge(inputHeight, inputWidth);
	input_edge = find_edges(input, edge_thresh);
	tem_edge = find_edges(tem, edge_thresh);

	SDoublePlane Dscore(inputHeight, inputWidth);
	SDoublePlane score(inputHeight - temHeight + 1, inputWidth - temWidth + 1);

	for(int i = 0; i < inputHeight; i++) {
		for(int j = 0; j < inputWidth; j++) {
			if(input_edge[i][j] == 1) {
				Dscore[i][j] = 0;
			} else {
				Dscore[i][j] = dis_max;
			}
		}
	}
	for(int i = 1; i < inputHeight; i++) {
		for(int j = 1; j < inputWidth - 1; j++) {
			if(Dscore[i][j] > 0) {
				d1 = MElr[0] + Dscore[i][j-1];
				d2 = MElr[1] + Dscore[i-1][j-1];
				d3 = MElr[2] + Dscore[i-1][j];
				d4 = MElr[3] + Dscore[i-1][j+1];
				Dscore[i][j] = min(min(d1,d2),min(d3,d4));
			}
		}
	}
	for(int i = inputHeight - 2; i >= 0; i--) {
		for(int j = inputWidth - 2; j > 0; j--) {
			if(Dscore[i][j] > 0) {
				d1 = MElr[0] + Dscore[i][j+1];
				d2 = MElr[1] + Dscore[i+1][j+1];
				d3 = MElr[2] + Dscore[i+1][j];
				d4 = MElr[3] + Dscore[i+1][j-1];
				Dscore[i][j] = min(min(Dscore[i][j],min(d1,d2)),min(d3,d4));
			}
		}
	}

	double min = 10000;
	for(int i = 0; i <= inputHeight - temHeight; i++) {
		for(int j = 0; j <= inputWidth - temWidth; j++) {
			sum = 0;
			for(int k = 0; k < temHeight; k++) {
				for(int l = 0; l < temWidth; l++) {
					if(tem_edge[k][l] == 1) {
						sum += Dscore[i+k][j+l];
					}
				}
			}
			score[i][j] = sum;
			if(sum < min) min = sum;
		}
	}
	cout << "minimum score for this template matching is : " << min << "\n";
	return score;
}



void q_five_detect(const SDoublePlane &input, const SDoublePlane tem, int type, double symbol_thresh, vector<DetectedSymbol> &symbols)
{
	int scoreHeight = input.rows();
	int scoreWidth = input.cols();
	DetectedSymbol s;
	for(int i = 0; i < scoreHeight; i++) {
		for(int j = 0; j < scoreWidth; j++) {
			if(input[i][j] < symbol_thresh) {
				s.row = i;
				s.col = j;
				s.width = tem.cols();
				s.height = tem.rows();
				s.type = (Type) type;
				s.confidence = input[i][j];
				s.pitch = (rand() % 7) + 'A';
				symbols.push_back(s);
			}
		}
	}
}


vector<DetectedSymbol> hough_transform(SDoublePlane input)
{
	vector<DetectedSymbol> symbols;
	int h = input.rows();
	int w = input.cols();
	
	int thresh = w/2;
	int previous_row = 0;
	
	for(int i=0;i<h;i++)
	{
		int length = 0;
		for(int j=0;j<w;j++)
		{
			if(input[i][j] < 255)
				length++;
			else
				length = 0;
			if(length > thresh)
			{
				if(i != previous_row+1)
				{
					DetectedSymbol s;
					s.row = i;
					s.col = j - thresh;
					s.width = w*5/6;
					s.height = 1;
					s.type = (Type)2;
					s.confidence = 1.0;
					//s.pitch = (rand() % 7) + 'A';
					symbols.push_back(s);
				}
				previous_row = i;
				length = 0;
			}
		}
	}
	
	write_detection_image("staves.png", symbols, input);
	return symbols;
}

vector<DetectedSymbol> score_fourth(SDoublePlane input_image)
{
	vector<DetectedSymbol> symbols;
	
	SDoublePlane template_c[3];
	template_c[0]= SImageIO::read_png_file("template1.png");
	template_c[1]= SImageIO::read_png_file("template2.png");
	template_c[2]= SImageIO::read_png_file("template3.png");
	
	SDoublePlane c[3];
	
	c[0] = convolve_general(input_image,template_c[0]);
	c[1] = convolve_general(input_image,template_c[1]);
	c[2] = convolve_general(input_image,template_c[2]);
	
	int thresh[3] = {100000,100000,100000};
	int h = c[0].rows();
	int w = c[0].cols();
	
	SImageIO::write_png_file("scores4.png", c[0], c[0], c[0]);
	
	for(int a=0;a<3;a++)
	{
		for(int i=0; i<h ; i++)
			for(int j =0; j<w; j++)
				if(c[a][i][j] < thresh[a])
					thresh[a] = c[a][i][j];
	}
	
	thresh[0] += 2000;
	thresh[1] += 2000;
	thresh[2] += 100;
	
	for(int a=0;a<3;a++)
	{
		int count = 0;
		
		for(int i=0; i<h ; i++)
			for(int j =0; j<w; j++)
				if(c[a][i][j] < thresh[a])
				{
					DetectedSymbol s;
					s.row = i - (template_c[a].rows()/2 + 1);
					s.col = j - (template_c[a].cols()/2 + 1);
					s.width = template_c[a].cols();
					s.height = template_c[a].rows();
					s.type = (Type)a;
					s.confidence = 1.0;
					s.pitch = (rand() % 7) + 'A';
					symbols.push_back(s);
				}
	}
	
	return symbols;
}

// This main file just outputs a few test images. You'll want to change it to do
//  something more interesting!
//
int main(int argc, char *argv[])
{
	if(!(argc == 2))
	{
		cerr << "usage: " << argv[0] << " input_image" << endl;
		return 1;
	}

	static const double edge_thresh[3] = {76, 50, 70};
	static const double symbol_thresh[3] = {13, 20, 20};
	//static const double symbol_thresh = 0.25;
	string input_filename(argv[1]);
	SDoublePlane input_image= SImageIO::read_png_file(input_filename.c_str());
	
	// Step 6
	vector<DetectedSymbol> staves = hough_transform(input_image);
	double staves_height = (abs(staves[0].row - staves[4].row)/4);
	
	// Rescaleing Template Images

	// Step 4
	vector<DetectedSymbol> symbols4 = score_fourth(input_image);
	write_detection_image("detected4.png", symbols4, input_image);

	// test step 3 and 5 by applying sobel operator and seperable convolution, and visualize the edge map.
	SDoublePlane output_image = visual_edge_map(input_image, edge_thresh[0]);
	SImageIO::write_png_file("edges.png", output_image, output_image, output_image);

	// test step 5 symbol detection.
	SDoublePlane tem[3];
	tem[0] = SImageIO::read_png_file("template1.png");
	tem[1] = SImageIO::read_png_file("template2.png");
	tem[2] = SImageIO::read_png_file("template3.png");
	vector<DetectedSymbol> symbols;
	SDoublePlane score;
	string filename;

	for(int i = 0; i < 3; i++) {
		output_image = visual_edge_map(tem[i], edge_thresh[i]);
		//filename = "template"+to_string(i+1)+"_edges.png";
		//SImageIO::write_png_file(filename.c_str(), output_image, output_image, output_image);
		score = q_five_score(input_image, tem[i], edge_thresh[i]);
		q_five_detect(score, tem[i], i, symbol_thresh[i], symbols);
		score = normalize(score, 255);
		//filename = "score-tem-"+to_string(i+1)+"-2loops.png";
		//SImageIO::write_png_file(filename.c_str(), score, score, score);
	}

	write_detection_image("detected5.png", symbols, input_image);
	write_detection_image("detected7.png", symbols, input_image);
	write_detection_txt("detected.txt", symbols);
}
