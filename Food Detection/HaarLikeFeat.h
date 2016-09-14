class HaarLikeFeat : public Classifier
{
	public:
	HaarLikeFeat(const vector<string> &_class_list) : Classifier(_class_list) {}

	// Nearest neighbor training. All this does is read in all the images, resize
	// them to a common size, convert to greyscale, and dump them as vectors to a file
	virtual void train(const Dataset &filenames, string algo)
	{
		int classNum = 1;
		ofstream train_file,myfile;
		
		// to clear the file if data exists beforhand.
		train_file.open("imgTrainData.dat", ios::out | ios::trunc);
		train_file.close();

		train_file.open("imgTrainData.dat", ios::out | ios::app);
		if(train_file.is_open())
			cout<<"Opened Succeslully"<<endl;
		
		CImg<int> haarFeatures = haarFeaturesGenerator(2000,size,size);
		myfile.open("haar_model", ios::out | ios::trunc);
		myfile.close();

		myfile.open("haar_model", ios::out | ios::app);
		if(myfile.is_open())
			cout<<"Opened Succeslully"<<endl;
		
		for (int i=0;i<haarFeatures.width();i++)
		{
			for(int j=0;j<haarFeatures.height();j++)
				myfile << haarFeatures(i,j)<<" ";
			myfile<<"\n";
		}
		myfile.close();
		
		for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
		{
			cout << "Processing " << c_iter->first << endl;
			for(int i=0; i<c_iter->second.size(); i++)
			{
				CImg<double> temp_img = extract_features_haar(c_iter->second[i].c_str());
				CImg<double> temp_integralImg = integralImg(temp_img);
				
				train_file << classNum;
				for(int j=0; j<haarFeatures.width(); j++)
				{
					double temp = getValue(temp_integralImg, haarFeatures(j,0),haarFeatures(j,1),haarFeatures(j,2),haarFeatures(j,3),haarFeatures(j,4));
					train_file << " "<< j+1 << ":" << temp;// write feature to file
				}
				train_file << "\n";
			}
			classNum++;
		}
		train_file.close();
		
		// train svm
		system("svm_multiclass/./svm_multiclass_learn -c 10.0 imgTrainData.dat model_haar");
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
		int classNum = 1;
		ofstream test_file;
		ifstream myfile;
		
		CImg<int> haarFeatures(2000,5);
		myfile.open("haar_model", ios::in);

		string line;
		int i=0;
		while ( getline (myfile,line) )
		{
			std::stringstream ss(line);
			ss >> haarFeatures(i,0) >> haarFeatures(i,1) >> haarFeatures(i,2)>> haarFeatures(i,3)>> haarFeatures(i,4);
			i++;
		}
		
		// to clear the file if data exists beforhand.
		test_file.open("imgTestData.dat", ios::out | ios::trunc);
		test_file.close();

		test_file.open("imgTestData.dat", ios::out | ios::app);
		if(test_file.is_open())
			cout<<"File Opened Succeslully"<<endl;
		
		for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
		{
			cout << "Processing " << c_iter->first << endl;
			for(int i=0; i<c_iter->second.size(); i++)
			{
				CImg<double> temp_img = extract_features_haar(c_iter->second[i].c_str());
				CImg<double> temp_integralImg = integralImg(temp_img);
				
				test_file << classNum;
				for(int j=0; j<haarFeatures.width(); j++)
				{
					double temp = getValue(temp_integralImg, haarFeatures(j,0),haarFeatures(j,1),haarFeatures(j,2),haarFeatures(j,3),haarFeatures(j,4));
					test_file << " "<< j+1 << ":" << temp;// write feature to file
				}
				test_file << "\n";
			}
			classNum++;
		}
		test_file.close();
		
		system("svm_multiclass/./svm_multiclass_classify imgTestData.dat model_haar predictions_haar");
	}
	
	protected:
	// extract features from an image, which in this case just involves resampling and 
	// rearranging into a vector of pixel data.
	CImg<double> extract_features_haar(const string &filename)
	{
		return (CImg<double>(filename.c_str())).get_RGBtoHSI().get_channel(2).resize(size,size,1);
	}
	
	// This function returns a list of haar features having five values,
	// 0 -> Type of viola jones feature
	// 1,2 -> x & y coordinates of the image where image integral will be applied.
	// 3,4 -> width & height of the haar like feature.
	CImg<int> haarFeaturesGenerator(int n,int width, int height)
	{
		CImg<int> haarFeatures(n,5);
		int type,x,y,w,h;
		
		srand(time(NULL));
		for(int i=0;i<n;i++)
		{
			type = rand()%5+1;
			if(type == 1)
			{
				x = rand()%(width-1);
				y = rand()%height;
				w = rand()%((width-x)/2)+1;
				h = rand()%(height-y)+1;
			}
			else if(type==2)
			{
				x = rand()%width;
				y = rand()%(height-1);
				w = rand()%(width-x)+1;
				h = rand()%((height-y)/2)+1;
			}
			else if(type==3)
			{
				x = rand()%(width-2);
				y = rand()%height;
				w = rand()%((width-x)/3)+1;
				h = rand()%(height-y)+1;
			}
			else if(type==4)
			{
				x = rand()%width;
				y = rand()%(height-2);
				w = rand()%(width-x)+1;
				h = rand()%((height-y)/3)+1;
			}
			else
			{
				x = rand()%(width-1);
				y = rand()%(height-1);
				w = rand()%((width-x)/2)+1;
				h = rand()%((height-y)/2)+1;
			}
			haarFeatures(i,0) = type;
			haarFeatures(i,1) = x;
			haarFeatures(i,2) = y;
			haarFeatures(i,3) = w;
			haarFeatures(i,4) = h;
		}
		return haarFeatures;
	}
	
	CImg<double> integralImg(const CImg<double> &img)
	{
		CImg<double> intImage(img.width(),img.height());
		intImage(0,0) = img(0,0);
		for(int i=0;i<intImage.width();i++)
			for(int j=0;j<intImage.height();j++)
				intImage(i,j) = img(i,j) + (j>0?intImage(i,j-1):0.0) + (i>0?intImage(i-1,j):0.0) - ((i>0&&j>0)?intImage(i-1,j-1):0.0);
		return intImage;
	}
	
	double getArea(const CImg<double> &intImg, int x1, int y1, int x2, int y2)
	{
		return ((intImg(x1,y1)+intImg(x2,y2)) - (intImg(x1,y2)+intImg(x2,y1)));
	}
	
	double getValue(const CImg<double> &intImg, int type, int x, int y, int width, int height)
	{
		if(type == 1)
			return getArea(intImg, x, y, x +width -1, y +height -1) - getArea(intImg, x+width, y, x +2*width -1, y +height -1);
		else if(type == 2)
			return getArea(intImg, x, y, x +width -1, y +height -1) - getArea(intImg, x, y +height, x +width -1, y +2*height -1);
		else if(type == 3)
			return getArea(intImg, x, y, x +width -1, y +height -1) - getArea(intImg, x +width, y, x +2*width -1, y +height -1) + getArea(intImg, x+2*width, y, x +3*width -1, y +height -1);
		else if(type == 4)
			return getArea(intImg, x, y, x +width -1, y +height -1) - getArea(intImg, x, y +height, x +width -1, y +2*height -1) + getArea(intImg, x, y +2*height, x +width -1, y +3*height -1);
		else
			return getArea(intImg, x, y, x +width -1, y +height -1) - getArea(intImg, x +width, y, x +2*width -1, y +height -1) - getArea(intImg, x, y +height, x +width -1, y +2*height -1) + getArea(intImg, x +width, y +height, x +2*width -1, y +2*height -1);
	}

	static const int size=100;  // subsampled image resolution
	//map<string, CImg<double> > models; // trained models
};
