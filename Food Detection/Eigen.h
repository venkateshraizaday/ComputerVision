class Eigen : public Classifier
{
	public:
	Eigen(const vector<string> &_class_list) : Classifier(_class_list) {}

	// Nearest neighbor training. All this does is read in all the images, resize
	// them to a common size, convert to greyscale, and dump them as vectors to a file
	virtual void train(const Dataset &filenames, string algo)
	{
		create_file(filenames, "eigenTrainData.dat");
		
		// train svm
		system("svm_multiclass/./svm_multiclass_learn -c 10 eigenTrainData.dat eigenModel");
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
		
		create_file(filenames, "eigenTestData.dat");
		system("svm_multiclass/./svm_multiclass_classify eigenTestData.dat eigenModel eigenPredictions");
	}
	
	protected:
	// extract features from an image, which in this case just involves resampling and 
	// rearranging into a vector of pixel data.
	CImg<double> extract_features_eigen(const string &filename)
	{
		CImg<double> input(filename.c_str());
		CImg<double> gray = input.get_RGBtoHSI().get_channel(2);
		return gray.resize(40,40,1).unroll('x');
	}
	
	void create_file(const Dataset &filenames, const char* filename)
	{
		int class_num = 1;
		ofstream test_file;


		// to clear the file if data exists beforehand.
		test_file.open(filename, ios::out | ios::trunc);
		test_file.close();

		test_file.open(filename, ios::out | ios::app);
		if(test_file.is_open())
			cout<<"File opened Successfully"<<endl;
		_DTwoDimArray<double> data(1250,6400);
		int count = 0;
		for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
		{
			cout << "Processing " << c_iter->first << endl;
			//CImg<double> class_vectors(size*size*3, filenames.size(), 1);
			// temp_img;
			// convert each image to be a row of this "model"
			//CImg<double> data(4800,c_iter->second.size(),1);
			double n = c_iter->second.size();


			for(int i=0; i<c_iter->second.size(); i++)
			{
				//class_vectors = class_vectors.draw_image(0, i, 0, 0, extract_features_baseline(c_iter->second[i].c_str()));
				CImg<double> temp_img = extract_features_eigen(c_iter->second[i].c_str());
//				cout<< "height: " << temp_img.height() << endl;
//				cout<< "width: " << temp_img.width() << endl;
//				cout<< "i: " << i << endl;

				for(int k=0; k<temp_img.height(); k++)
				{
					//test_file << class_num;
					for(int j=0; j<temp_img.width(); j++)
						data[count][j] = temp_img(j,k,0);
					//test_file <<"\n";
				}
				count++;
			}
			class_num++;
		}

		//cout << "Count :" <<count << endl;
		//Calculating the empirical mean

		_DTwoDimArray<double> u(1600,1);
		_DTwoDimArray<double> h(count,1);
		_DTwoDimArray<double> B(count,6400);
		_DTwoDimArray<double> Sigma(count,6400);
		//_DTwoDimArray<double> C(4800,4800);
		//CImg<double> covar(4800,4800,1,1,0);   //Covariance Matrix
		CImg<double> D(1600,1600,1,1,0);       //Eigen Values in Diagonal Matrix
		CImg<double> Vec(1600,1600,1,1,0);	   //Eigen Vectors
		CImg<double> topEigenVec(15,1600,1,1,0); //Top Eigen Vectors
		CImg<double> projectedData(15,count,1,1,0);  //Projected Image in k-dimensional space

		CImg<double> M(1600,count,1,1,0);
		CImg<double> U(count,count,1,1,0);
		CImg<double> S(1600,count,1,1,0);
		CImg<double> V(1600,1600,1,1,0);
		for(int i=0;i<count;i++)
		{
			h[i][0] = 1;
		}

		//Calculating the Empirical Mean
		cout<< "Phase 1" << endl;

		for(int i=0;i<count;i++){
			double sum = 0;
			for(int j=0;j<1600;j++){
				sum += data[i][j];
			}
			u[i][0] = sum/count;
		}

		cout<< "Phase 2" << endl;


		// Calculating Deviations from the empirical mean
		for(int i=0;i<count;i++){
			for(int j=0;j<1600;j++){
				B[i][j] = data[i][j] - h[i][0]*u[j][0];    // B = X - h.{u}^{T}
				M(j,i,0) = B[i][j];
			}
		}

		cout<< "Phase 3" << endl;

		/*

		//Calculating Covariance Matrix
		for(int i=0;i<4800;i++){
			for(int j=0;j<4800;j++){
				//Multiply the two matrix
				for(int k=0;k<n;k++){
					C[i][j] += B[k][i]*B[k][j];
				}
				C[i][j] = C[i][j]/(n-1);
				covar(j,i,0) = C[i][j];
			}
		}

		cout<< "Phase 4" << endl;

		covar.symmetric_eigen(D,V);

		cout<< "Phase 5" << endl;

		*/

		M.SVD(U,S,V);

		//The transpose of V is the Eigen Vector Matrix

		cout<< "Phase 4" << endl;

		Vec = V.get_transpose();

		// S^(T).S will give the diagonal Matrix of square root of Eigen Values.

		for(int i=0;i<1600;i++){
			for(int j=0;j<1600;j++){
				for(int k=0;k<count;k++){
					//C[i][j] += (double)S(i,k,0,0)*(double)S(j,k,0,0);
					D(j,i,0,0) += (double)S(i,k,0,0)*(double)S(j,k,0,0);
					//D(j,i,0,1) += (double)S(i,k,0,1)*(double)S(j,k,0,1);
					//D(j,i,0,2) += (double)S(i,k,0,2)*(double)S(j,k,0,2);
				}
				D(j,i,0,0) = (double)D(j,i,0,0)*(double)D(j,i,0,0);   //Squaring the values will give the Eigen Values
			}
		}

		//cout << "Printing out the eigen values. " << endl;

		/*cimg_forY(D,y){
			cimg_forX(D,x){
				if((double)D(x,y,0,0) != 0)
					cout << (double)D(x,y,0,0) << " ";
			}
		}*/

		topEigenVec = Vec.get_columns(0,15);
		cout<< "Phase 5" << endl;

		//Calculating each projected Image onto k-dimensional space
		class_num = 1;
		for(int i=0;i<count;i++){
			test_file << class_num;
			for(int j=0;j<15;j++){
				for(int k=0;k<1600;k++){
					projectedData(j,i,0,0) += data[i][k] * (double)topEigenVec(j,k,0,0);
				}
				test_file << " "<< j+1 << ":" << projectedData(j,i,0);
			}
			test_file <<"\n";
			if(((i+1)%50) == 0)
				class_num++;
		}

		cout<< "Phase 6" << endl;
		test_file.close();
	}

	//static const int size=40;  // subsampled image resolution
	map<string, CImg<double> > models; // trained models
};
