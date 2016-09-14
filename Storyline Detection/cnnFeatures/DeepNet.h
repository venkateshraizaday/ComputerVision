#include <stdio.h>
#include <fstream>
class DeepNet : public Classifier
{
public:
  DeepNet(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // SVM training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    
    ofstream ofs("deepFeatures");
    int CategoryID = 1;
    string categoryName;
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
    {
    categoryName = c_iter->first;
	cout << "Processing " << categoryName << endl;
	CategoryID = 1;
       
        for(int i=0;i<c_iter->second.size();i++)
	{
		cout << "Image : "<<c_iter->second[i] << endl;
		
		string imageName = c_iter->second[i];
		string className = imageName.substr(imageName.find_last_of("/") + 1, 1);
		//cout <<"Class Label : " << className << endl;
  		getNeurFile_File(c_iter->second[i]);
		ifstream nnOutputFile ("neural_features.txt");
		string line;
		if (nnOutputFile.is_open())
    		{
			int count=2;
			while (getline (nnOutputFile,line) )
			{
				count--;
				 if(count==0)
				{
	  
	 				 vector<string> stringValues;
	 				 split( line, stringValues, ' ' );
	         
              	     ofs<<className<<" ";
	  
	 				 //cout<< "String values  size : "<< stringValues.size() << endl;
	 			 	for(int i = 0 ; i < stringValues.size(); i++)
					{
						if(atoi(stringValues[i].c_str())!=0)
							ofs<<(i+1)<<":"<<stringValues[i]<<" ";
		  			}
				}
				
			}	

	 	 	nnOutputFile.close();
			remove("neural_features.txt");
			ofs<<"#"<<c_iter->second[i];
 			ofs<<endl;
			
		}
			
		CategoryID++;
   	 }
	 

    }
    ofs.close();
	cout<<"Feature file generated for input to SVMClassifier."<<endl;
	
	//Prepare command in string form
	string command;
	command = "./svm_multiclass_learn -c 1.0 deepFeatures deep_svm_model";
	system(command.c_str());
	
	cout<<"SVM Model file generated as deep_svm_model"<<endl;	
	
  }

  void getNeurFile_File(const string &file)
  {	
	string command;
	freopen("neural_features.txt","w",stdout);
	CImg<double> class_image(file.c_str());
        class_image=class_image.resize(231,231,1,3);
        class_image.save("image.png");
        command = "./overfeat/bin/linux_64/overfeat -L 12 image.png" ;
	system(command.c_str());
	//fclose(stdout);
	freopen ("/dev/tty", "a", stdout);
	
  }

virtual string classify(const string &filename)
  {
	  
	
	ofstream ofs;
	ofs.open("testImage.txt");
	
	//Get the category ID from map
	string categoryID  = "1";
	
	
	//Write category ID to file
	ofs << categoryID << " ";
     
    //Get the feature vector and write it to testImage.txt	 
	getNeurFile_File(filename);
	ifstream nnOutputFile ("neural_features.txt");
	string line;
		
	if (nnOutputFile.is_open())
	{
		int count = 2;
		while (getline (nnOutputFile,line) )
		{
			count--;
			if(count == 0)
			{
  
				vector<string> stringValues;
				split( line, stringValues, ' ' );
				for(int i = 0 ; i < stringValues.size(); i++)
				{
					if(atoi(stringValues[i].c_str())!=0){
						ofs<<(i+1)<<":"<<stringValues[i]<<" ";
					}		
				}
			}
			
		}	

		nnOutputFile.close();
		remove("neural_features.txt");
		ofs<<"# "<< filename << endl;		
	}
	ofs.close();
    
    freopen ("/dev/null", "w", stdout);	
	string command = "./svm_multiclass_classify testImage.txt deep_svm_model predictions";
	system(command.c_str());
	freopen ("/dev/tty", "a", stdout);
	
	//Read the predictions file and return the class
	//Get the index of the class predicted
    string line1;
	ifstream predictionsFile ("predictions");
	string estimatedCategoryID;
   	 if (predictionsFile.is_open())
    	{
		while ( getline (predictionsFile,line1) )
		{
			  int pos = line1.find_first_of(" ");
			  estimatedCategoryID = line1.substr(0, pos);
		}
		  predictionsFile.close();
	}
	
	remove("testImage.txt");
	remove("predictions");
	
	
	return estimatedCategoryID;
  }	


//http://stackoverflow.com/questions/5888022/split-string-by-single-spaces
void split(const string &txt, vector<std::string> &strs, char ch)
{
    int pos = txt.find( ch );
    int initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos + 1 ) );
        initialPos = pos + 1;
        pos = txt.find( ch, initialPos );
    }

	int minValue = 0;
	if(pos< txt.size()){
		minValue = pos;
	}else{
		minValue = txt.size();
	}
    // Add the last one
    strs.push_back( txt.substr( initialPos, minValue - initialPos + 1 ) );
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
  static const int size = 20;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
  map<string, string > foodIDNameMap; 
};
