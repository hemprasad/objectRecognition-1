/*
 * BOF.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: bhikadiy
 */

#include "BOF.h"

void BOF::trainvocab()
{
	extractor= ( new SiftDescriptorExtractor());

	std::vector<int> class_end_index;
	int ind=0;
	for(unsigned int i=0;i<set_list.size();i++)
	{
		ind += set_list.at(i).second;
		class_end_index.push_back(ind);
	}
	for(unsigned int i=0;i<class_end_index.size();i++)
	{
		std::cout<<"class_end_indexs are"<<class_end_index.at(i)<<std::endl;
	}

	/*
	 * SIFT sift;
	vector<KeyPoint> key_points;
	Mat descriptors;
	Mat training_desriptor;
	 * std::ofstream object_file;
    object_file.open("/s/chopin/l/grad/bhikadiy/Jazz/CS510/workspace/Assignment-4/data/SIFT_OUTPUT_FIXED/desciptor_sizes.txt");


    object_file<<std::setw(25)<<"Image Name"<<std::setw(10)<<"no. of descriptors\n";

    for(unsigned int i=0;i<set.size();i++)
    {
    	//std::cout<<"\nworking on "<<set.at(i).getname();
    	//calculating sift descriptors
    	Mat temp=set.at(i).getme().clone(), output_img;
    	sift(temp, Mat(), key_points, descriptors);
    	drawKeypoints(temp, key_points, output_img);

    	//writing files
    	std::stringstream filename,path;
    	filename<<set.at(i).getname();
    	object_file<<std::setw(25)<<set.at(i).getname()<<std::setw(10)<<descriptors.rows<<"x"<<descriptors.cols<<std::endl;
    	filename<<".jpg";
    	path<<"/s/chopin/l/grad/bhikadiy/Jazz/CS510/workspace/Assignment-4/data/SIFT_OUTPUT_FIXED/"<<filename.str();
    	//std::cout<<path.str()<<std::endl;
    	imwrite(path.str(),output_img);

    	//add descriptors to training set
    	training_desriptor.push_back(descriptors);
    	std::cout<<"\nno of desriptors "<<descriptors.rows<<"\n training set size is "<<training_desriptor.rows<<"x"<<training_desriptor.cols;
    }

    BOWKMeansTrainer bowtrainer(800); //num clusters
    bowtrainer.add(training_desriptor);
   	std::cout << "cluster BOW features" << std::endl;*/

	//culstering ... no need to do all time.. read it from xml file*/
	/*vocabulary = bowtrainer.cluster();


   	std::cout<<"vocab size"<<vocabulary.rows<<"x"<<vocabulary.cols;
   	FileStorage f;
   	f.open("tempstandardsift.xml", FileStorage::WRITE);
   	f <<"vocabulary"<< vocabulary;
   	f.release();*/

	//alternate to clustering
	FileStorage fs;
	fs.open("/Jatin/MS/cs510/Assignment-4/tempstandardsift.xml", FileStorage::READ);

	if (fs.isOpened())
	{
		std::cout<<"File is opened\n";
	}

	fs["vocabulary"] >> vocabulary;
	std::cout<<"\nvocabulary size is "<<vocabulary.rows <<" x "<<vocabulary.cols<<std::endl;



	//alternate done

	//Ptr<DescriptorMatcher > matcher  (new DescriptorMatcher:: BruteForceMatcher<L2<float> >());
	//(new BruteForceMatcher<L2<float> >());

	BOWImgDescriptorExtractor bowide(extractor,matcher);

	bowide.setVocabulary(vocabulary);

	vector<KeyPoint> keypoints;

	Mat response_hist;
	std::vector<std::vector<int> >pids;
	std::vector<std::vector<std::vector<int> > >pids_array;

	FileStorage fa;
	fa.open("pids.xml", FileStorage::WRITE);
	std::vector<std::pair<string,Mat> >class_data(set_list.size());

	for(unsigned int i=0;i<class_data.size();i++)
	{
		class_data.at(i).first = set_list.at(i).first;
	}
	for(unsigned int i=0;i<set.size();i++)
	{
		Mat temp=set.at(i).getme().clone();
		std::cout<<i<<std::endl;
		detector.detect(temp,keypoints);
		std::cout<<"\nno of keypoints "<<keypoints.size()<<std::endl;
		bowide.compute(temp,keypoints,response_hist,&pids);
		response_hist = response_hist*keypoints.size();
		if((int)i<class_end_index.at(0))
		{
			class_data.at(0).second.push_back(response_hist);
		}
		else
		{
			for(unsigned int j=0;j<set_list.size()-1;j++)
			{
				if((int) i<class_end_index.at(j+1) && (int)i>=class_end_index.at(j))
				{
					class_data.at(j+1).second.push_back(response_hist);
				}
			}
		}
		for(unsigned int k=0;k<pids.size();k++)
		{
			std::stringstream h;
			h<<"pid"<<i<<"a"<<k;
			fa<<h.str()<<pids.at(k);
		}
	}
	fa.release();



	FileStorage f;
	f.open("responses_new.xml", FileStorage::WRITE);
	for(unsigned int i=0;i<class_data.size();i++)
	{
		f <<class_data.at(i).first<< class_data.at(i).second;

	}


	f.release();
	std::map<std::string,Mat> classes_training_data;
	for(unsigned int i=0;i<class_data.size();i++)
	{
		classes_training_data[class_data.at(i).first]=class_data.at(i).second;
	}
	///

	//SVM part starts here. here 4 svm will be trained for 4 classes.
	CvSVMParams params;



	for(unsigned int i=0;i<classes_training_data.size();i++)
	{
		string class_name = set_list.at(i).first;
		std::cout<<"\nSVM is training class "<<class_name;
		Mat samples,labels;

		samples.push_back(classes_training_data[class_name]);
		Mat pos_labels = Mat::ones(classes_training_data[class_name].rows,1,CV_32FC1);
		labels.push_back(pos_labels);

		for(unsigned int j=0;j<classes_training_data.size();j++)
		{
			if(i!=j)
			{
				string not_class = set_list.at(j).first;
				std::cout<<"\nnegative class is "<<not_class;
				samples.push_back(classes_training_data[not_class]);
				Mat neg_label = Mat::zeros(classes_training_data[not_class].rows,1,CV_32FC1);
				labels.push_back(neg_label);
			}
		}
		std::cout<<"sample size is"<<samples.rows<<"x "<< samples.cols;
		samples.convertTo(samples,CV_32FC1);
		classifiers[class_name].train_auto(samples,labels,Mat(),Mat(),params,5);

		//classifiers[class_name].train(samples,labels);

		FileStorage p;
		std::stringstream filename,svmname;
		filename<<class_name<<".xml";
		p.open(filename.str(),FileStorage::WRITE);
		p<<"samples"<<samples;
		p<<"labels"<<labels;
		svmname<<class_name<<"svm.xml";

		classifiers[class_name].save(svmname.str().c_str());
	}
	std::cout<<"training is done\n";
	//testing the trained data
	for(unsigned int i=0;i<classes_training_data.size();i++)
	{
		string class_name = set_list.at(i).first;
		std::cout<<"\nSVM is testing class "<<class_name;
		Mat samples,labels;

		samples.push_back(classes_training_data[class_name]);
		samples.convertTo(samples,CV_32FC1);
		// classifiers[class_name].train_auto(samples,labels,Mat(),Mat(),params,3);

		for(unsigned int i=0;i<classifiers.size();i++)
		{	std::string classname = set_list.at(i).first;
		classifiers[classname].predict(samples,labels);
		std::cout<<"\nresults for class : "<<classname<<" : "<<labels<<std::endl;
		}


	}



}

void BOF::test(std::vector<Img>&test_set)
{
	std::cout<<"here\n";
	extractor = (new SiftDescriptorExtractor()); //  extractor;

	//Ptr<DescriptorMatcher > matcher  (new DescriptorMatcher:: BruteForceMatcher<L2<float> >());
	//(new BruteForceMatcher<L2<float> >());
	BOWImgDescriptorExtractor bowide(extractor,matcher);


	FileStorage fs;
	 	fs.open("/Jatin/MS/cs510/Assignment-4/tempstandardsift.xml", FileStorage::READ);

	   	if (fs.isOpened())
	   	{
	   		std::cout<<"File is opened\n";
	   	}

	   	fs["vocabulary"] >> vocabulary;
	   	std::cout<<"\nvocabulary size is "<<vocabulary.rows <<" x "<<vocabulary.cols<<std::endl;

	bowide.setVocabulary(vocabulary);

	std::vector<std::string> test_name, result_name;

	for(unsigned int i=0;i<test_set.size();i++)
	{
		Mat temp = test_set.at(i).getme().clone();

		Mat response_hist;
		vector<KeyPoint> keypoints;
		detector.detect(temp,keypoints);
		bowide.compute(temp, keypoints, response_hist);
		std::cout<<"response size is "<<response_hist.rows<<"x "<<response_hist.cols;
		response_hist = response_hist*keypoints.size();

		string test_image_name = test_set.at(i).getname();
		std::cout<<"\nTest Image name : "<< test_image_name;
		//float min=FLT_MAX;
		string detected_class="no class";
		std::vector<int >response_array;
		for(unsigned int j=0 ; j<classifiers.size();j++)
		{
			string class_name=set_list.at(j).first;
			response_hist.convertTo(response_hist,CV_32FC1);
			float resp = classifiers[class_name].predict(response_hist,0);
			std::cout << "\nclass : "<<class_name<<"response "<<resp;
			response_array.push_back(resp);
		}

		detected_class =  find_detected_class(response_array,response_hist);




		test_name.push_back(test_image_name);
		result_name.push_back(detected_class);
		std::cout<<"\n detected class is : "<<detected_class<<std::endl;
	}

	confusinmatrix(test_name,result_name);
}

std::string BOF:: find_detected_class(std::vector<int>response_list,Mat response_h)
{
	int no_of_ones=0;
	for(unsigned int i=0; i<response_list.size();i++)
	{
		if(response_list.at(i)==1)
		{
			no_of_ones++;
		}
	}

	if(no_of_ones==1)
	{
		for(unsigned int j=0 ; j<classifiers.size();j++)
		{
			string class_name=set_list.at(j).first;
			float resp = classifiers[class_name].predict(response_h,1);
			std::cout << "\nclass : "<<class_name<<"response "<<resp;

		}

		for(unsigned int i=0; i<response_list.size();i++)
		{
			if(response_list.at(i)==1)
			{
				return set_list.at(i).first;
			}
		}

	}
	else if(no_of_ones>1)
	{
		string detected_class;
		std::vector<int> detected_class_index;
		{
			for(unsigned int i=0; i<response_list.size();i++)
			{
				if(response_list.at(i)==1)
				{
					detected_class_index.push_back(i);
				}
			}
			std::cout<<"\n detected classes : "<<detected_class_index.size();
			float min=FLT_MAX;

			for(unsigned int j=0 ; j<detected_class_index.size();j++)
			{
				string class_name=set_list.at(detected_class_index.at(j)).first;
				float resp = classifiers[class_name].predict(response_h,1);
				std::cout << "\nclass : "<<class_name<<"response "<<resp;
				if(resp<min)
				{
					min=resp;
					detected_class=class_name;
				}
			}
		}
		return detected_class;
	}
	else
	{
		float min=FLT_MAX;
		string detected_class;
		for(unsigned int j=0 ; j<classifiers.size();j++)
		{
			string class_name=set_list.at(j).first;
			float resp = classifiers[class_name].predict(response_h,1);
			std::cout << "\nclass : "<<class_name<<"response "<<resp;
			if(resp<min)
			{
				min=resp;
				detected_class=class_name;
			}
		}
		return detected_class;
	}
}





void BOF:: confusinmatrix(std::vector<std::string>test_names,std::vector<std::string>result_names)
{
	int checkz,checkl,checkai,checkau, airplane_size=0, zebra_size=0, leopards_size=0, automobiles_size=0;
	std::map<string,int> airplane,leopards,zebra,automobile;
	for(unsigned int i=0 ;i<set_list.size();i++)
	{
		airplane[set_list.at(i).first]=0;
		automobile[set_list.at(i).first]=0;
		leopards[set_list.at(i).first]=0;
		zebra[set_list.at(i).first]=0;
	}
	for(unsigned int i=0;i<test_names.size();i++)
	{
		checkz=test_names.at(i).find("zebra");
		checkl=test_names.at(i).find("leopards");
		checkai=test_names.at(i).find("airplane");
		checkau=test_names.at(i).find("automobile");

		if(checkai>0)
		{
			airplane_size++;
			airplane[result_names.at(i)]++;
			if(result_names.at(i)!="airplane")
			{
				std::cout<<"\nTest Image : "<<test_names.at(i)<<"  Detected Class : "<<result_names.at(i);
			}
		}
		if(checkau>0)
		{
			automobiles_size++;
			automobile[result_names.at(i)]++;
			if(result_names.at(i)!="automobiles")
						{
							std::cout<<"\nTest Image : "<<test_names.at(i)<<"  Detected Class : "<<result_names.at(i);
						}
		}
		if(checkz>0)
		{
			zebra_size++;
			zebra[result_names.at(i)]++;
			if(result_names.at(i)!="zebra")
						{
							std::cout<<"\nTest Image : "<<test_names.at(i)<<"  Detected Class : "<<result_names.at(i);
						}
		}
		if(checkl>0)
		{
			leopards_size++;
			leopards[result_names.at(i)]++;
			if(result_names.at(i)!="leopards")
						{
							std::cout<<"\nTest Image : "<<test_names.at(i)<<"  Detected Class : "<<result_names.at(i);
						}
		}
	}
	std::cout<<"no of airplanes : "<<airplane_size<<std::endl;
	for(unsigned int i=0 ;i<set_list.size();i++)
	{	string classn = set_list.at(i).first;
	std::cout <<" no of "<<classn <<" : "<<airplane[classn]<<std::endl;
	}
	std::cout<<"no of automobiles : "<<automobiles_size<<std::endl;
	for(unsigned int i=0 ;i<set_list.size();i++)
	{	string classn = set_list.at(i).first;
	std::cout <<" no of "<<classn <<" : "<<automobile[classn]<<std::endl;
	}
	std::cout<<"no of leopards : "<<leopards_size<<std::endl;
	for(unsigned int i=0 ;i<set_list.size();i++)
	{	string classn = set_list.at(i).first;
	std::cout <<" no of "<<classn <<" : "<<leopards[classn]<<std::endl;
	}
	std::cout<<"no of zebra: "<<zebra_size<<std::endl;
	for(unsigned int i=0 ;i<set_list.size();i++)
	{	string classn = set_list.at(i).first;
	std::cout <<" no of "<<classn <<" : "<<zebra[classn]<<std::endl;
	}
}




BOF::~BOF() {
	// TODO Auto-generated destructor stub
}

