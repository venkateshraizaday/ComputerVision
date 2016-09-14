#include <fstream>

class Baseline : public Classifier
{
public:
  Baseline(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // SVM training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
	ofstream foodCategoryFile;
	foodCategoryFile.open("foodCategory.txt");
	
	// Generate the feature_info file which acts as input to svm_multiclass_learn
	ofstream outputFile;
	outputFile.open("baselineFeatures");
	int categoryID = 1;
	string categoryName;
	int featureCount ;
	
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	categoryName = c_iter->first;
	cout << "Processing " << categoryName << endl;
	foodCategoryFile << categoryID << " " << categoryName <<endl;
	
	CImg<double> class_vectors(size*size*3, c_iter->second.size(), 1);
	
	// convert each image to be a row of this "model" image
	for(int i = 0; i < c_iter->second.size(); i++){
	  
	  cout << "Image : "<<c_iter->second[i] << endl;
	  class_vectors = class_vectors.draw_image(0, i, 0, 0, extract_features(c_iter->second[i].c_str()));
	  featureCount = 0;
	  
	  //write the category ID 
	  outputFile << categoryID <<" ";
	  
	  //write feature vector for this image 
		for (int x = 0; x < size * size * 3; x++) {
			featureCount++;
			if(class_vectors(x, i, 0, 0) != 0){
				outputFile << featureCount << ":"<< class_vectors(x, i, 0, 0) << " ";
			}				
		}
		
		
	  //write image file name at the end for debugging purpose
	  outputFile << "# "<< c_iter->second[i] << endl;
	    
	}
	categoryID++;
    }
	foodCategoryFile.close();
	outputFile.close();
	
	//Feature_info file is generated and now we can call svm_classify_learn to create a model file -
	cout<<"Feature file generated for input to SVMClassifier."<<endl;
	
	//Prepare command in string form
	string command;
	command = "./svm_multiclass_learn -c 1.0 baselineFeatures baseline_svm_model";
	system(command.c_str());
	cout<<"SVM Model file generated as baseline_svm_model"<<endl;
  }
  
virtual string classify(const string &filename)
  {	  
	int endSlashPos = filename.find_last_of("/");
	string categoryName = filename.substr(5 , endSlashPos - 5);
	
	ofstream outputFile;
	outputFile.open("testImage.txt");
	int featureCount = 0;
	
	//Get the category ID from map
	string categoryID;
	for (map<string,string>::iterator it=foodIDNameMap.begin(); it!=foodIDNameMap.end(); ++it)
	{
		if(strcmp((it->second).c_str(), categoryName.c_str()) == 0){
			categoryID = it->first;
			break;
		}
	}
	
	//Write category ID to file
	outputFile << categoryID << " ";

    CImg<double> test_image = extract_features(filename);
	CImg<double> class_vectors(size*size*3, 1, 1);
    class_vectors = class_vectors.draw_image(0, 0, 0, 0, test_image);  
	//write feature vector for this image
	  for (int y = 0; y < 1; y++) {
			for (int x = 0; x < size * size * 3; x++) {
				featureCount++;
				if(class_vectors(x, y, 0, 0) != 0){
					outputFile << featureCount << ":"<< class_vectors(x, y, 0, 0) << " ";
				}				
			}
		}
		
	//write image file name at the end for debugging purpose
	outputFile << "# "<< filename << endl;
	
    freopen ("/dev/null", "w", stdout);	
	string command = "./svm_multiclass_classify testImage.txt baseline_svm_model predictions";
	system(command.c_str());
	freopen ("/dev/tty", "a", stdout);
	
	//Read the predictions file and return the class
	//Get the index of the class predicted
    string line;
	ifstream predictionsFile ("predictions");
	string estimatedCategoryID;
    if (predictionsFile.is_open())
    {
	while ( getline (predictionsFile,line) )
	{
	  int pos = line.find_first_of(" ");
	  estimatedCategoryID = line.substr(0, pos);
	}
	  predictionsFile.close();
	}
	
	//Get the estimated category  name from map
	string estimatedCategoryName;
	for (map<string,string>::iterator it=foodIDNameMap.begin(); it!=foodIDNameMap.end(); ++it)
	{
		if(strcmp((it->first).c_str(), estimatedCategoryID.c_str()) == 0){
			estimatedCategoryName = it->second;
			break;
		}
	}
	
	return estimatedCategoryName;
  }

  virtual void load_model()
  {
	string categoryID;
	string categoryName;
	string line;
	ifstream foodCategoryFile ("foodCategory.txt");
    if (foodCategoryFile.is_open())
    {
	while ( getline (foodCategoryFile,line) )
	{
	  int pos = line.find_first_of(" ");
	  categoryID = line.substr(0, pos);
	  categoryName = line.substr(pos + 1, line.length() - pos - 1 );
	  foodIDNameMap[categoryID] = categoryName;
	}
	  foodCategoryFile.close();
	}
   
  }
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  CImg<double> extract_features(const string &filename)
    {
      return (CImg<double>(filename.c_str())).resize(size,size,1,3).unroll('x');
    }

  static const int size = 20;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
  map<string, string > foodIDNameMap; 
};