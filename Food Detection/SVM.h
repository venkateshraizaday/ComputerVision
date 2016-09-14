class SVM : public Classifier
{
	public:
	SVM(const vector<string> &_class_list) : Classifier(_class_list) {}

	// Nearest neighbor training. All this does is read in all the images, resize
	// them to a common size, convert to greyscale, and dump them as vectors to a file
	virtual void train(const Dataset &filenames, string algo)
	{
		create_file(filenames, "imgTrainData.dat");
		
		// train svm
		system("svm_multiclass/./svm_multiclass_learn -c 5000 imgTrainData.dat model");
	}

	virtual string classify(const string &filename)
	{
		// no need
	}

	virtual void load_model()
	{
		// svm_multiclass does this...
	}
	
	virtual void test(const Dataset &filenames, string algo)
	{
		cerr << "Loading model..." << endl;
		
		create_file(filenames, "imgTestData.dat");
		system("svm_multiclass/./svm_multiclass_classify imgTestData.dat model predictions");
	}
	
	protected:
	// extract features from an image, which in this case just involves resampling and 
	// rearranging into a vector of pixel data.
	CImg<double> extract_features_baseline(const string &filename)
	{
		return (CImg<double>(filename.c_str())).resize(size,size,1,3).unroll('x');
	}
	
	void create_file(const Dataset &filenames, const char* filename)
	{
		int class_num = 1;
		ofstream test_file;

		// to clear the file if data exists beforhand.
		test_file.open(filename, ios::out | ios::trunc);
		test_file.close();

		test_file.open(filename, ios::out | ios::app);
		if(test_file.is_open())
			cout<<"File opened Successfully"<<endl;
		for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
		{
			cout << "Processing " << c_iter->first << endl;
			//CImg<double> class_vectors(size*size*3, filenames.size(), 1);
			// temp_img;
			// convert each image to be a row of this "model"
			for(int i=0; i<c_iter->second.size(); i++)
			{
				//class_vectors = class_vectors.draw_image(0, i, 0, 0, extract_features_baseline(c_iter->second[i].c_str()));
				CImg<double> temp_img = extract_features_baseline(c_iter->second[i].c_str());
				for(int k=0; k<temp_img.height(); k++)
				{
					test_file << class_num;
					for(int j=0; j<temp_img.width(); j++)
						test_file << " "<< j+1 << ":" << temp_img(j,k,0);
					test_file <<"\n";
				}
			}
			class_num++;
		}
		test_file.close();
	}

	static const int size=40;  // subsampled image resolution
	map<string, CImg<double> > models; // trained models
};
