/* Include files for OpenCv library used for Kmeans */
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

#include <ctime>
#include <fstream>
#include <fstream>
using namespace cv;


class BOW: public Classifier
{
public:
	BOW(const vector<string> &_class_list) : Classifier(_class_list) {

		train_filename 			= "BOW_Train_SVM.dat";
		test_filename 			= "BOW_Test_SVM.dat";
		predictions_filename 	= "BOW_Predictions_SVM.dat";
		model_filename			= "BOW_Model_SVM.dat";

	}

	// Classify a single image.
	virtual string classify(const string &filename) {};

	// Load in a trained model.
	virtual void load_model() {};

	// Run training on a given dataset.
	virtual void train(const Dataset &filenames, string algo);

	// Loop through all test images, hiding correct labels and checking if we get them right.
	virtual void test(const Dataset &filenames, string algo);

	// Compute the histogram of visual words for each image
	void computeHistogram(std::vector<int> &imagesID, std::vector<int> &labels, int &k, int &imageNumber, std::vector< std::vector<float> > &histogram);

	// builds the input file for the SVM classifier
	void buildSVMFile(std::vector< std::vector<float> > &histograms, const char *filename, std::vector<int> &imgLabels);
	
	// Obtain the sift descriptors for each image, runs kmeans, compute the final descriptor for each image and builds the input file for SVM
	void computeDescriptors(const Dataset &filenames, const char * train_file);

	void getSVMLabels(const char *filename, std::vector<int> &imgLabels);


protected:
	vector<string> class_list;

	std::vector< std::vector<float> > histograms;

	cv::Mat descriptors;
	cv::Mat centers;

	std::vector<int> labels;

	string train_filename;
	string test_filename;
	string predictions_filename;
	string model_filename;

};


void BOW::train(const Dataset &filenames, string algo){

	computeDescriptors(filenames, train_filename.c_str());


	string command = "svm_multiclass/./svm_multiclass_learn -c 100 " + train_filename + " " + model_filename;

	// clock_t svm_begin = clock();
	struct timeval tv;
	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);

	system(command.c_str());

	// clock_t svm_end = clock();
	// double svm_elapsed_secs = double(svm_end - svm_begin) / CLOCKS_PER_SEC/ 60.0;
	// printf("time elapsed for svm: %f [min]\n", svm_elapsed_secs);

	double elapsed = 0.0;
	gettimeofday(&tv, NULL);
	elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
	printf("time elapsed for svm train: %f [min]\n", elapsed);
}


void BOW::test(const Dataset &filenames, string algo){

	// std::vector<int> labels;
	std::vector<int> svm_labels;

	computeDescriptors(filenames, test_filename.c_str());

	struct timeval tv;
	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);

	string command = "svm_multiclass/./svm_multiclass_classify " + test_filename + " " + model_filename + " " + predictions_filename;

	system(command.c_str());

	double elapsed = 0.0;
	gettimeofday(&tv, NULL);
	elapsed = (tv.tv_sec - start_tv.tv_sec) + (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
	printf("time elapsed for svm test: %f [min]\n", elapsed);
}

/*
* BOW::computeDescriptors method uses the OpenCV library for kmeans algorithm from: http://docs.opencv.org/2.4/modules/core/doc/clustering.html
*/
void BOW::computeDescriptors(const Dataset &filenames, const char * train_file){

	std::vector<int> imagesID;

	int category = 1;
	int imgId = 0;
	int k = 100;
	int imageNumber = 0;
	int classNumber = filenames.size();

	clock_t sift_begin = clock();

	for(Dataset::const_iterator c_iter = filenames.begin(); c_iter != filenames.end(); ++c_iter)
	{
  	// for each class
		cout << "Processing " << c_iter->first << endl;

		imageNumber = imageNumber + c_iter->second.size();

		for(int i = 0; i < c_iter->second.size(); i++)
		{
      	// for each image
			CImg<double> new_img(c_iter->second.at(i).c_str());

			CImg<double> new_gray = new_img.get_RGBtoHSI().get_channel(2);
			int height = new_gray._height;
			int width = new_gray._width;

  			// new_gray.resize(int(height/2), int(width/2));

			vector<SiftDescriptor> desc_img = Sift::compute_sift(new_gray);

			labels.push_back(category);

			for (int j = 0; j < desc_img.size(); ++j)
			{ 
        	// for each sift point
				cv::Mat aux_desc(desc_img.at(j).descriptor, true);
				aux_desc = aux_desc.t();

				descriptors.push_back(aux_desc);
				imagesID.push_back(imgId);
			}
			imgId++;
		}
		category++;
	}

	printf("Analizing %d descriptors... \n", descriptors.rows , descriptors.cols);

	clock_t sift_end = clock();
	double sift_elapsed_secs = double(sift_end - sift_begin) / CLOCKS_PER_SEC / 60.0;
	printf("time elapsed for sift: %f [min]\n", sift_elapsed_secs);


	std::vector<int> labels_v;
	clock_t kmeans_begin = clock();

	cv::kmeans(descriptors, k, labels_v, cv::TermCriteria( cv::TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centers);

	clock_t kmeans_end = clock();
	double kmeans_elapsed_secs = double(kmeans_end - kmeans_begin) / CLOCKS_PER_SEC/ 60.0;
	printf("time elapsed for kmeans: %f [min]\n", kmeans_elapsed_secs);

	computeHistogram(imagesID, labels_v, k, imageNumber, histograms);

	buildSVMFile(histograms, train_file, labels);

}


void BOW::computeHistogram(std::vector<int> &imagesID, std::vector<int> &labels, int &k, int &imageNumber, std::vector< std::vector<float> > &histogram){

	for (int i = 0; i < imageNumber; ++i)
	{
		std::vector<float> hist_i(k);
		for (int ii = 0; ii < k; ++ii)
		{
			hist_i[ii] = 0;
		}
		histogram.push_back(hist_i);
	}

	int max = 0;

	for (int j = 0; j < imagesID.size(); ++j)
	{   
		histogram[imagesID.at(j)][labels.at(j)] = histogram[imagesID.at(j)][labels.at(j)] + 1;
		if (histogram[imagesID.at(j)][labels.at(j)] > max)
			max = histogram[imagesID.at(j)][labels.at(j)];
	}

	// histogram normalization
	for (int k = 0; k < imagesID.size(); ++k)
	{   
		histogram[imagesID.at(k)][labels.at(k)] = histogram[imagesID.at(k)][labels.at(k)] / max;
	}
}


void BOW::buildSVMFile(std::vector< std::vector<float> > &histograms, const char *filename, std::vector<int> &imgLabels){

	ofstream output(filename);

	if(!output)
	{
		printf("File not opened \n");
		return;
	}

	for(int i = 0; i < histograms.size(); ++i)
	{
		output << imgLabels.at(i) << " "; 
		for(int j = 0; j < histograms.at(i).size(); ++j)
		{
			output << j+1 << ":" << histograms.at(i).at(j) << " ";
		}
		output << endl;
	}

	output.close();
}

void BOW::getSVMLabels(const char *filename, std::vector<int> &imgLabels){

	std::ifstream infile(filename);

	std::string line;
	while (std::getline(infile, line))
	{
		cout << line[0] << " ";
		imgLabels.push_back(int(line[0]));
	}

	cout << endl;

	infile.close();
}








