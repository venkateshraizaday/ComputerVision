#include <string>
class CNN: public Classifier
{
public:
	CNN(const vector<string> &_class_list) : Classifier(_class_list) {

		train_filename 			= "CNN_Train_SVM.dat";
		test_filename 			= "CNN_Test_SVM.dat";
		model_filename 			= "CNN_Model_SVM.dat";
		predictions_filename 	= "CNN_Predictions_SVM.dat";
	}

	// Classify a single image.
	virtual string classify(const string &filename){};

	// Load in a trained model.
	virtual void load_model(){};

	// Run training on a given dataset.
	virtual void train(const Dataset &filenames, string algo);

	virtual void train(const char* filename){};

	// Loop through all test images, hiding correct labels and checking if we get them right.
	virtual void test(const Dataset &filenames, string algo);

	virtual void test(const char* filename){};

	void computetDescriptors(const Dataset &filenames, const char * svm_file);

	void buildSVMFile(const char *feature_filename, const char *svm_file, int &label);

protected:
	vector<string> class_list;
	string train_filename;
	string test_filename;
	string model_filename;
	string predictions_filename;
	std::vector<int> labels;
};

void CNN::train(const Dataset &filenames, string algo){

	computetDescriptors(filenames, train_filename.c_str());

	string command = "svm_multiclass/./svm_multiclass_learn -c 100 " + train_filename + " " + model_filename;

	system(command.c_str());

}


void CNN::test(const Dataset &filenames, string algo){

	computetDescriptors(filenames, test_filename.c_str());

	string command = "svm_multiclass/./svm_multiclass_classify " + test_filename + " " + model_filename + " " + predictions_filename;

	system(command.c_str());

}

void CNN::computetDescriptors(const Dataset &filenames, const char * svm_file){

	string layer_num = "12";

	string cnnFeaturesFile = "overfeatFeatures.dat";
	int category = 1;
	for(Dataset::const_iterator c_iter = filenames.begin(); c_iter != filenames.end(); ++c_iter)
	{
  	// for each class
		cout << "Processing " << c_iter->first << endl;

		for(int i = 0; i < c_iter->second.size(); i++)
		{
			// for each image
			labels.push_back(category);

			string command = "./overfeat/bin/linux_64/overfeat -L " + layer_num + " " + c_iter->second.at(i).c_str() + " >> " + cnnFeaturesFile;
			
			system(command.c_str());

			buildSVMFile(cnnFeaturesFile.c_str(), svm_file, category);
		}
		category++;
	}

}


void CNN::buildSVMFile(const char *feature_filename, const char *svm_file, int &label){


	ofstream output(svm_file, std::ofstream::out | std::ofstream::ate);

	std::ifstream infile(feature_filename);

	std::string line;

	std::vector<string> cnn_features;
	while (std::getline(infile, line))
	{
		cnn_features.push_back(line);
	}

	infile.close();

	string dimensions = cnn_features[0];
	
	int n_found = dimensions.find(" ");
	string s_aux = dimensions.substr(0, n_found);
	int n = atoi( s_aux.c_str() );


	int h_found = dimensions.find(" ", n_found+1);
	s_aux = dimensions.substr(n_found+1, h_found - (n_found+1));
	int h = atoi( s_aux.c_str() );

	int w_found = dimensions.find(" ", h_found+1);
	s_aux = dimensions.substr(h_found+1, w_found - (h_found+1));
	int w = atoi( s_aux.c_str() );


	std::vector<float> feature_matrix;

	istringstream iss(cnn_features[1]);
	string s;
	while ( getline( iss, s, ' ' ) ) {
		feature_matrix.push_back( atoi(s.c_str() ) );
	}

	if(!output)
	{
		printf("File not opened \n");
		return;
	}

	int i = 0;
	int j = 1;
	int begin = 0;
	int end = w * h - 1;

	output << label << " "; 
	while(end < feature_matrix.size())
	{

		output << j << ":" << feature_matrix.at(begin) << " ";

		j++;

		begin = end + 1;
		end = end + w * h - 1;
	}
	
	output << endl;

	output.close();
}
