// B657 assignment 2 skeleton code
//
// Compile with: "make"
//
// See assignment handout for command line and project specifications.


//Link to the header file
#include "CImg.h"
#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Sift.h>
#include <time.h>
#include <dirent.h>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;

class pointsRANSAC
{
	public:
	int x1;
	int x2;
	int y1;
	int y2;
};

//Part I Q4
//This functions takes the descriptors of a image and the value k.
//It reduces the dimensions of each descriptor point to k dimensions.
//We assume the value of 'w' and take intial value of x from 0-1 and reduce the dimensions.
vector<vector<int> > compute(vector<SiftDescriptor> &descriptorsorg,int k)
{
	double w=500.0;
	double x[128]; 
	int trails=10;
	vector<vector<int> > v1;
	for(int l=0;l<128;l++)
	{
		x[l]=(double) (rand() %100)/100;	
	}
	for(int i=0;i<descriptorsorg.size();i++)
	{
		vector<int> t;
		for(int j=0;j<k;j++)
		{
			double val=0.0;
			for(int l=0; l<128; l++)
			{

				val+=(descriptorsorg[i].descriptor[l]*x[l]);

			}
			int t1=(int)val/w;
			t.push_back(t1);
		}
		v1.push_back(t);
	}
	for(int nt=1;nt<=trails;nt++)
	{
		for(int l=0;l<128;l++)
		{
			x[l]=(double) (rand() %100)/100;	
		}
		for(int i=0;i<descriptorsorg.size();i++)
		{
			vector<int> t;
			for(int j=0;j<k;j++)
				{
					double val=0.0;
					for(int l=0; l<128; l++)
						{

							val+=(descriptorsorg[i].descriptor[l]*x[l]);

						}
					int t1=(int)val/w;
					if(t1<v1[i][j])
						v1[i][j]=t1;
				}
		}

	}

	return v1;
 }


void printimage(vector<SiftDescriptor> descriptors,CImg<double> &input_image)
{
	for(int i=0; i<descriptors.size(); i++)
	  {
	    cout << "Descriptor #" << i << ": x=" << descriptors[i].col << " y=" << descriptors[i].row << " descriptor=(";
	    for(int l=0; l<128; l++)
	      cout << descriptors[i].descriptor[l] << "," ;
	    cout << ")" << endl;

	    for(int j=0; j<5; j++)
	      for(int k=0; k<5; k++)
		if(j==2 || k==2)
		  for(int p=0; p<3; p++)
		  	if(descriptors[i].col+k < input_image.width() && descriptors[i].row+j < input_image.height())
                      input_image(descriptors[i].col+k, descriptors[i].row+j, 0, p)=0;
	  }
	
}
//Sort function rule to sort the descriptors based on their dist value which is the ratio of first smallest and second smallest distance.
bool Valuedist(const SiftDescriptor  &a, const SiftDescriptor  &b)
{
    return a.dist< b.dist;
}
//Distance function,we calculate the distance for each descriptor by comparing it with the input image's descriptor and find the ratio of least distance 
//and second leastdistance  and store it in 'dist' for each descriptor point.We sort the vector of descriptors and take  1/40th value as threshold and assume it as the optimal
//distance between the image.
double dist(vector<SiftDescriptor> &descriptors1,vector<SiftDescriptor> &descriptors2,vector<SiftDescriptor> &descriptors3)
{
	int count=0;
	for(int i=0;i<descriptors1.size();i++)
	{
		double min1=10000.0;
		double min2=100000.0;
		int ind;

		for(int j=0;j<descriptors2.size();j++)
		{
			double val=0;

			for(int l=0; l<128; l++)
			{
				val+=(descriptors1[i].descriptor[l]-descriptors2[j].descriptor[l])*(descriptors1[i].descriptor[l]-descriptors2[j].descriptor[l]);
				
			}
			val=sqrt(val);
			if(val<min1)
			{
				min2 = min1;
				min1 = val;
				ind = j;		
			}
			else if(val<min2)
			{
				min2 = val;
			}
		}
		
		
		descriptors1[i].dist=(min1/min2);
		descriptors3[i]=descriptors2[ind];
		descriptors3[i].x1 = descriptors1[i].col;
		descriptors3[i].y1 = descriptors1[i].row;
		descriptors3[i].x2 = descriptors2[i].col;
		descriptors3[i].y2 = descriptors2[i].row;
		descriptors3[i].dist = descriptors1[i].dist;

	}
	sort(descriptors1.begin(),descriptors1.end(),Valuedist);
	int thresh_ind = descriptors1.size()/40;
	return descriptors1[thresh_ind].dist;	
}

//We draw line based on the value of threshold,we check for each descriptor point's dist value if it is less that threshold we consider it as 
//sift match between the images and draw a line between them.
void drawline(CImg<double> &input_image,vector<SiftDescriptor> descriptors1,vector<SiftDescriptor> descriptors2,int width,double thres)
{
	const unsigned char color[] = { 255,128,64 };

	for(int i=0;i<descriptors2.size();i++)
	{

		if(descriptors1[i].dist<=thres)
			input_image.draw_line(descriptors1[i].col,descriptors1[i].row,width+descriptors2[i].col,descriptors2[i].row,color);

	}

}

//Data Structure to store image name and corresponding value we get on comparing it with query image.We use this value at later point
//When we rank the image,lower the value higher its match with the image compared.
class data
	{
	public:
		string str;
		double count;
		int ind;
	};
//Helper function to sort the data instances based on count value to give the image in matching order.
bool ValueCmp(const data  &a,const data  &b)
{
    return a.count< b.count;
}

CImg<double> getWarpedImg(CImg<double> matrix, CImg<double> input_image)
{
	cout<<"Creating Warped Image"<<"\n";
	CImg<double> tf_inv = matrix.get_invert();
	
    CImg<double> output_image(input_image.width(),input_image.height(),1,3,255);

    for (int i=0; i<input_image.width(); i++)
    {
        for (int j=0; j<input_image.height(); j++) 
        {
            double z = tf_inv(2,0)*i + tf_inv(2,1)*j + tf_inv(2,2)*1;
            int x = (tf_inv(0,0)*i + tf_inv(0,1)*j + tf_inv(0,2)*1)/z;
            int y = (tf_inv(1,0)*i + tf_inv(1,1)*j + tf_inv(1,2)*1)/z;
            if (x>=0 && x<input_image.width() && y>=0 && y<input_image.height())
				for (int k=0; k<3; k++)
					output_image(i,j,k) = input_image(x,y,k);
        }
    }
	return output_image;
}

CImg<double> getHomography(CImg<double> input_image1,CImg<double> input_image2)
{
	CImg<double> gray1 = input_image1.get_RGBtoHSI().get_channel(2);
	CImg<double> gray2 = input_image2.get_RGBtoHSI().get_channel(2);
	vector<SiftDescriptor> descriptors1 = Sift::compute_sift(gray1);
	vector<SiftDescriptor> descriptors2 = Sift::compute_sift(gray2);
	
	vector<SiftDescriptor> descriptors3(descriptors1.size());
	double threshold = dist(descriptors1,descriptors2,descriptors3);
	
	vector<pointsRANSAC> points;
	pointsRANSAC temp;
	for(int i=0;i<descriptors3.size();i++)
	{
		if(descriptors3[i].dist <= threshold)
		{
			temp.x1 = descriptors3[i].x1;
			temp.y1 = descriptors3[i].y1;
			temp.x2 = descriptors3[i].x2;
			temp.y2 = descriptors3[i].y2;
			points.push_back(temp);
		}
	}
	
	cout<<"Performing RANSAC"<<"\n";
	cout<<"Points:"<<points.size()<<"\n";
	int max_inliers = 0;
	CImg<double> best_matrix(3,3);
	
	int N=1000;
	for(int i=0; i<N;i++)
	{
		CImg<double> matrix1(8,8);
		CImg<double> matrix2(1,8);
		for(int j=0;j<4;j++)
		{
			int index = rand()%points.size();
			matrix1(0,(2*j)) = points[index].x1;
			matrix1(1,(2*j)) = points[index].y1;
			matrix1(2,(2*j)) = 1;
			matrix1(3,(2*j)) = 0;
			matrix1(4,(2*j)) = 0;
			matrix1(5,(2*j)) = 0;
			matrix1(6,(2*j)) = -1*(points[index].x1*points[index].x2);
			matrix1(7,(2*j)) = -1*(points[index].y1*points[index].x2);
			
			matrix1(0,(2*j)+1) = 0;
			matrix1(1,(2*j)+1) = 0;
			matrix1(2,(2*j)+1) = 0;
			matrix1(3,(2*j)+1) = points[index].x1;
			matrix1(4,(2*j)+1) = points[index].y1;
			matrix1(5,(2*j)+1) = 1;
			matrix1(6,(2*j)+1) = -1*(points[index].x1*points[index].y2);
			matrix1(7,(2*j)+1) = -1*(points[index].y1*points[index].y2);
			
			matrix2(0,(2*j)) = points[index].x2;
			matrix2(0,(2*j)+1) = points[index].y2;
		}
		
		//cout<<"Solving Matrix"<<"\n";
		CImg<double> temp(1,8);
		temp = matrix2.solve(matrix1);
		CImg<double> result(3,3);
		
		/*result(0,0) = temp(0,0); result(0,1) = temp(1,0); result(0,2) = temp(2,0);
		result(1,0) = temp(3,0); result(1,1) = temp(4,0); result(1,2) = temp(5,0);
		result(2,0) = temp(6,0); result(2,1) = temp(7,0); result(2,2) = 1;*/
		
		result(0,0) = temp(0,0); result(0,1) = temp(0,1); result(0,2) = temp(0,2);
		result(1,0) = temp(0,3); result(1,1) = temp(0,4); result(1,2) = temp(0,5);
		result(2,0) = temp(0,6); result(2,1) = temp(0,7); result(2,2) = 1;
		
		//cout<<"Counting Inliers"<<"\n"<<i<<"\n";
		int window = 20;
		int inlier = 0;
		for(int j=0;j<points.size();j++)
		{
			double z = result(2,0)*points[j].x1 + result(2,1)*points[j].y1 + result(2,2)*1;
			double new_x = (result(0,0)*points[j].x1 + result(0,1)*points[j].y1 + result(0,2))/z;
			double new_y = (result(1,0)*points[j].x1 + result(1,1)*points[j].y1 + result(1,2))/z;
			
			if(floor(new_x) < (points[j].x2 + window) && floor(new_x)> (points[j].x2 - window))
				if(floor(new_y) < (points[j].y2 + window) && floor(new_y) > (points[j].y2 - window))
					inlier++;
		}
		if(inlier>max_inliers)
		{
			max_inliers = inlier;
			best_matrix = result;
		}
	}
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
			cout<<best_matrix(i,j)<<" ";
		cout<<"\n";
	}
	cout<<"Max Inliers:"<<max_inliers<<"\n";
	return best_matrix;
}

//Main function
int main(const int argc, char **argv)
{
  try {

    if(argc < 2)
      {
	cout << "Insufficent number of arguments; correct usage:" << endl;
	cout << "    a2-p1 part_id ..." << endl;
	return -1;
      }

    string part = argv[1];
    string inputFile1 = argv[2];
    string inputFile2 =argv[3];

    if(part == "part1")
      {
	// This is just a bit of sample code to get you started, to
	// show how to use the SIFT library.
    vector<data> p1;
    data t1;
	CImg<double> input_image1(inputFile1.c_str());
	CImg<double> input_image2(inputFile2.c_str());
	vector<data> p;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Question 1
	//Draw line between matching sift points between two images
	
	CImg<double> gray1 = input_image1.get_RGBtoHSI().get_channel(2);
	CImg<double> gray2 = input_image2.get_RGBtoHSI().get_channel(2);
	vector<SiftDescriptor> descriptors1 = Sift::compute_sift(gray1);
	vector<SiftDescriptor> descriptors2 = Sift::compute_sift(gray2);
	int width=input_image1.width();
	vector<SiftDescriptor> descriptors3(descriptors1.size());
	t1.count=dist(descriptors1,descriptors2,descriptors3);
	t1.str=inputFile2;
	input_image1.append(input_image2);
	drawline(input_image1,descriptors1,descriptors3,width,t1.count);
	input_image1.get_normalize(0,255).save("sift.png");


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Question 2
	vector<data> p2;
	for(int i=3;i<argc;i++)
	{
		data t;
		string inputFiletemp =argv[i];
		CImg<double> input_imagetemp(inputFiletemp.c_str());
		
		CImg<double> temp_gray = input_imagetemp.get_RGBtoHSI().get_channel(2);
		vector<SiftDescriptor> descriptors1 = Sift::compute_sift(gray1);
		vector<SiftDescriptor> descrip2 = Sift::compute_sift(temp_gray);
		vector<SiftDescriptor> descrip3(descriptors1.size());
		
		t.count=dist(descriptors1,descrip2,descrip3);
		t.str=argv[i];
		p2.push_back(t);
	}
	std::sort(p2.begin(),p2.end(), ValueCmp);
	cout<<"\nComparing Image:"<<inputFile1;
	for(int i=0;i<p2.size();i++)
	{
		cout<<"\n"<<p2[i].str;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Question 3
	const string name[]={"londoneye_2.jpg","empirestate_14.jpg","colosseum_3.jpg","bigben_2.jpg","eiffel_1.jpg","empirestate_9.jpg","londoneye_2.jpg","louvre_3.jpg","notredame_1.jpg","sanmarco_1.jpg","tatemodern_2.jpg","trafalgarsquare_1.jpg"};
	DIR *dir;
	struct dirent *ent;
	clock_t time1,time2;
	time1=clock();
	for(int i=0;i<10;i++)
	{
	vector<data> p;
	string n="part1_images/";
	string temp=name[i].c_str();
	cout<<"\nComparing image: "<<name[i];
	temp=n+temp;
	CImg<double> input_imageorg(temp.c_str());
	CImg<double> grayorg = input_imageorg.get_RGBtoHSI().get_channel(2);
	vector<SiftDescriptor> descriptorsorg = Sift::compute_sift(grayorg);
	if ((dir = opendir ("part1_images")) != NULL)
	 {
	 	string s1="jpg";
	 	string s2="png";
  		while ((ent = readdir (dir)) != NULL)
  		 {
  		    string st=ent->d_name;
  		 	if(st.find(s2) != std::string::npos||st.find(s1) != std::string::npos)
  		 	{
  		 	data t;
  		 	string path="part1_images/";
  		 	string temp=path+ent->d_name;
    		CImg<double> input_imagetemp(temp.c_str());
			CImg<double> graytemp = input_imagetemp.get_RGBtoHSI().get_channel(2);
			vector<SiftDescriptor> descriptorstemp = Sift::compute_sift(graytemp);
			vector<SiftDescriptor> descripcmp(descriptorsorg.size());
			t.count=dist(descriptorsorg,descriptorstemp,descripcmp);
			t.str=ent->d_name;
			p.push_back(t);
		}
		else
			continue;
  		 }
  	closedir (dir);
    }

    std::sort(p.begin(),p.end(),ValueCmp);
    cout<<"\nTop 10 matches\n";
    for(int i=0;i<10;i++)
    {

    	cout<<p[i].str<<"\n";
    }
    
	}
	time2=clock();
    float diff =((float)time2-(float)time1);
	float seconds = diff / CLOCKS_PER_SEC;
    cout<<"Time for third Module:"<<seconds<<endl;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////Question 4
	time1=clock();
	for(int i=0;i<10;i++)
	{
	vector<data> p;
	string n="part1_images/";
	string temp=name[i].c_str();
	cout<<"\nComparing image: "<<name[i];
	temp=n+temp;
	CImg<double> input_imageorg(temp.c_str());
	CImg<double> grayorg = input_imageorg.get_RGBtoHSI().get_channel(2);
	vector<SiftDescriptor> descriptorsorg = Sift::compute_sift(grayorg);
	vector<vector<int> > v1=compute(descriptorsorg,20);
	vector<data>  p4;

	if ((dir = opendir ("part1_images")) != NULL)
	 {
		string ind;
	 	int k=0;
	 	string s1="jpg";
	 	string s2="png";
  		while ((ent = readdir (dir)) != NULL)
  		 {

  		 	string st=ent->d_name;
  		 	if(st.find(s2) != std::string::npos||st.find(s1) != std::string::npos)
  		 	{
  		 	data t;
  		 	string path="part1_images/";
  		 	string temp=path+ent->d_name;
  		 	vector<int> arr;
    		CImg<double> input_imagetemp(temp.c_str());
			CImg<double> graytemp = input_imagetemp.get_RGBtoHSI().get_channel(2);
			vector<SiftDescriptor> descriptorstemp = Sift::compute_sift(graytemp);
			vector<vector<int> > v2=compute(descriptorstemp,20);
			int count=0;
			data t4;
			vector<double> mindes;
			for(int i=0;i<v1.size();i++)
			{
				
				double min=1000000000000.0;
				string image_name;
				for(int j=0;j<v2.size();j++)
				{
					if(v1[i]==v2[j])
					{
						double sum=0.0;
						for(int l=0;l<128;l++)
							sum+=pow((descriptorsorg[i].descriptor[l]-descriptorstemp[j].descriptor[l]),2);
						sum=sqrt(sum);
						if(sum<min)
						{
							
							min=sum;
							ind=ent->d_name;
						}
					}	
				}
				mindes.push_back(min);
			}
			sort(mindes.begin(),mindes.end());
			int thres=mindes.size()/40;
			t4.count=mindes[thres];
			t4.str=ent->d_name;
			p4.push_back(t4);

			}
			else
			continue;
		}	

	 }
	 sort(p4.begin(),p4.end(),ValueCmp);
	 cout<<"\nTop 10 matches\n";
    for(int i=0;i<10;i++)
    {

    	cout<<p4[i].str<<"\n";
    }

	}
	time2=clock();
    diff =((float)time2-(float)time1);
	seconds = diff / CLOCKS_PER_SEC;
    cout<<"Time for fourth Module:"<<seconds<<endl;
	}

    else if(part == "part2")
    {
		
		string inputName = argv[2];
		CImg<double> input_image(inputName.c_str());
		//CImg<double> input_image("lincoln.png");

		// step 1
		/*CImg<double> tf1(3,3);
		tf1(0,0) = 0.907; tf1(0,1) = 0.258; tf1(0,2) = -182;
		tf1(1,0) = -0.153; tf1(1,1) = 1.44; tf1(1,2) = 58;
		tf1(2,0) = -0.000306; tf1(2,1) = 0.000731; tf1(2,2) = 1;

		CImg<double> output_image = getWarpedImg(tf1,input_image);
		output_image.save("transform.png");*/
		
		// step 2
		CImg<double> input_image1(inputFile1.c_str());
		CImg<double> input_image2(inputFile2.c_str());
		
		CImg<double> matrix = getHomography(input_image1,input_image2);
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
				cout<<matrix(i,j);
			cout<<"\n";
		}
		//output_image = getWarpedImg(matrix,input_image1);
		//output_image.save("part2.png");
		// step 3
		string inputFileMain = argv[2];
		CImg<double> image_main(inputFileMain.c_str());
		CImg<double> output_image;
		
		for(int i=3;i<argc;i++)
		{
			string nextImage = argv[i];
			CImg<double> image_new(nextImage.c_str());

			CImg<double> matrix = getHomography(image_new,image_main);
			/*CImg<double> matrix(3,3);
			matrix(0,0) = 1; matrix(0,1) = 1; matrix(0,2) = 0;
			matrix(1,0) = 0; matrix(1,1) = 1; matrix(1,2) = 0;
			matrix(2,0) = 0; matrix(2,1) = 0; matrix(2,2) = 1;*/
			output_image = getWarpedImg(matrix,image_new);
			output_image.append(image_new);
			
			nextImage = nextImage.substr(0,nextImage.length()-4);
			nextImage += "-Warped.png";
			output_image.save(nextImage.c_str());
		}
    }
    else
      throw std::string("unknown part!");

  }
  catch(const string &err) {
    cerr << "Error: " << err << endl;
  }
}