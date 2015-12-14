#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <iostream>
#include <cmath>
#include <ctime>
#include <emmintrin.h>
#include <smmintrin.h>
#include "stdint.h"
#include "Timer.h"

#include "classifier.h"
#include "EasyBMP.h"
#include "linear.h"
#include "argvparser.h"
#include "matrix.h"
#include "greyscale.h"
#include "sobel.h"
#include "calculatesobel.h"
#include "color_extractor.h"
#include "hog.h"
#include "pyramide.h"
#include "kernel.h"

/**
@mainpage Computer graphics task 2.
@author This project was created by Vadim Navrazhnykh.
*/

/**
@file task2.cpp
*/
#define PARAM_C 0.1
#define CELL_CNT 8

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::tie;
using std::tuple;
using std::make_pair;
using std::make_tuple;
using std::cout;
using std::cerr;
using std::endl;

using CommandLineProcessing::ArgvParser;

typedef unsigned char uchar;
typedef vector<pair<BMP*, int> > TDataSet;
typedef vector<pair<string, int> > TFileList;
typedef vector<pair<vector<float>, int> > TFeatures;
typedef Matrix<int> IMatrix;
typedef Matrix<uchar> CMatrix;
typedef Matrix<short> SMatrix;
typedef Matrix<float> FMatrix;
typedef tuple<float, float, float> Vector3;

bool is_sse;

/**
@function LoadFileList
loads list of files and its labels from 'data_file' and stores it in 'file_list'
@param data_file is a path to data file.
@param file_list is an output list of files.
*/
void LoadFileList(const string& data_file, TFileList* file_list) {
    ifstream stream(data_file.c_str());

    string filename;
    int label;
    
    int char_idx = data_file.size() - 1;
    for (; char_idx >= 0; --char_idx)
        if (data_file[char_idx] == '/' || data_file[char_idx] == '\\')
            break;
    string data_path = data_file.substr(0,char_idx+1);
    
    while(!stream.eof() && !stream.fail()) {
        stream >> filename >> label;
        if (filename.size())
            file_list->push_back(make_pair(data_path + filename, label));
    }

    stream.close();
}

/**
@function LoadImages
loads images by list of files 'file_list' and stores them in 'data_set'
@param file_list is an input list of files.
@param data_set is an output list of loaded images and labels.
*/
void LoadImages(const TFileList& file_list, TDataSet* data_set) {
    for (size_t img_idx = 0; img_idx < file_list.size(); ++img_idx) {
            // Create image
        BMP* image = new BMP();
            // Read image from file
        image->ReadFromFile(file_list[img_idx].first.c_str());
            // Add image and it's label to dataset
        data_set->push_back(make_pair(image, file_list[img_idx].second));
    }
}

/**
@function SavePredictions
saves result of prediction to file
@param file_list is an input list of files
@param labels is an input list of labels.
@param prediction_file is a path to output file.
*/
void SavePredictions(const TFileList& file_list,
                     const TLabels& labels, 
                     const string& prediction_file) 
{
        // Check that list of files and list of labels has equal size 
    assert(file_list.size() == labels.size());
        // Open 'prediction_file' for writing
    ofstream stream(prediction_file.c_str());

        // Write file names and labels to stream
    for (size_t image_idx = 0; image_idx < file_list.size(); ++image_idx)
        stream << file_list[image_idx].first << " " << labels[image_idx] << endl;
    stream.close();
}

/**
@function ExtractFeatures
exatracts features from dataset.
@param data_set is an input list of images and labels.
@param features is an output list of features.
*/
void ExtractFeatures(const TDataSet& data_set, TFeatures* features) {
    
    BMP *img;
    int label;

    Timer timer;
    timer.start();

    for (auto sample: data_set) 
    {
        vector<float> descryptor;

        tie(img, label) = sample;

        //Extracting color features
        ExtractColor(img, descryptor);

        // Grayscaling image
        CMatrix grey_scaled(img->TellHeight(), img->TellWidth());

        if (is_sse)
        {
            toGrayScaleSSE(*img, grey_scaled);
            HOG_SSE(grey_scaled, descryptor, CELL_CNT);
            PyramideSSE(grey_scaled, descryptor);
            X2KernelSSE(descryptor);
        }
        else
        {
            toGrayScale(*img, grey_scaled);
            HOG(grey_scaled, descryptor, CELL_CNT);
            Pyramide(grey_scaled, descryptor);
            X2Kernel(descryptor);
        }

        // Adding new feature
        features->push_back(make_pair(descryptor, label));
    }

    if (is_sse)
        timer.check("SSE: ");
    else
        timer.check("NO SSE: ");
}

/**
@function ClearDataset
clears dataset structure
@param data_set is an input list of images and labels to clear.
*/
void ClearDataset(TDataSet* data_set) {
        // Delete all images from dataset
    for (size_t image_idx = 0; image_idx < data_set->size(); ++image_idx)
        delete (*data_set)[image_idx].first;
        // Clear dataset
    data_set->clear();
}

/**
@function TrainClassifier
trains SVM classifier using data from 'data_file' and saves trained model to 'model_file'
@param data_file is a path to input file.
@param model_file is a path to output file.
*/
void TrainClassifier(const string& data_file, const string& model_file) {
        // List of image file names and its labels
    TFileList file_list;
        // Structure of images and its labels
    TDataSet data_set;
        // Structure of features of images and its labels
    TFeatures features;
        // Model which would be trained
    TModel model;
        // Parameters of classifier
    TClassifierParams params;
    
        // Load list of image file names and its labels
    LoadFileList(data_file, &file_list);
        // Load images
    LoadImages(file_list, &data_set);
        // Extract features from images
    try {
        ExtractFeatures(data_set, &features);
    }
    catch (string s) {
        cout << s << endl;
    }

        // Clear dataset structure
    ClearDataset(&data_set);

    params.C = PARAM_C;

    TClassifier classifier(params);
        // Train classifier
    classifier.Train(features, &model);
        // Save model to file
    model.Save(model_file);
}

/**
@function PredictData
predicts data from 'data_file' using model from 'model_file' and saves predictions to 'prediction_file'
@param data_file is a path to input data file.
@param model_file is a path to input model file.
@param prediction_file is a path to output file.
*/
void PredictData(const string& data_file,
                 const string& model_file,
                 const string& prediction_file) {
        // List of image file names and its labels
    TFileList file_list;
        // Structure of images and its labels
    TDataSet data_set;
        // Structure of features of images and its labels
    TFeatures features;
        // List of image labels
    TLabels labels;

        // Load list of image file names and its labels
    LoadFileList(data_file, &file_list);
        // Load images
    LoadImages(file_list, &data_set);
        // Extract features from images
    ExtractFeatures(data_set, &features);

        // Classifier 
    TClassifier classifier = TClassifier(TClassifierParams());
        // Trained model
    TModel model;
        // Load model from file
    model.Load(model_file);
        // Predict images by its features using 'model' and store predictions
        // to 'labels'
    classifier.Predict(features, model, &labels);

        // Save predictions
    SavePredictions(file_list, labels, prediction_file);
        // Clear dataset structure
    ClearDataset(&data_set);
}

int main(int argc, char** argv) {
    // Command line options parser
    ArgvParser cmd;
        // Description of program
    cmd.setIntroductoryDescription("Machine graphics course, task 2. CMC MSU, 2014.");
        // Add help option
    cmd.setHelpOption("h", "help", "Print this help message");
        // Add other options
    cmd.defineOption("data_set", "File with dataset",
        ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);
    cmd.defineOption("model", "Path to file to save or load model",
        ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired);
    cmd.defineOption("predicted_labels", "Path to file to save prediction results",
        ArgvParser::OptionRequiresValue);
    cmd.defineOption("train", "Train classifier");
    cmd.defineOption("predict", "Predict dataset");
    cmd.defineOption("sse", "sse");
        
        // Add options aliases
    cmd.defineOptionAlternative("data_set", "d");
    cmd.defineOptionAlternative("model", "m");
    cmd.defineOptionAlternative("predicted_labels", "l");
    cmd.defineOptionAlternative("train", "t");
    cmd.defineOptionAlternative("predict", "p");

        // Parse options
    int result = cmd.parse(argc, argv);

        // Check for errors or help option
    if (result) {
        cout << cmd.parseErrorDescription(result) << endl;
        return result;
    }

        // Get values 
    string data_file = cmd.optionValue("data_set");
    string model_file = cmd.optionValue("model");
    bool train = cmd.foundOption("train");
    bool predict = cmd.foundOption("predict");
    is_sse = cmd.foundOption("sse");

        // If we need to train classifier
    if (train)
        TrainClassifier(data_file, model_file);
        // If we need to predict data
    if (predict) {
            // You must declare file to save images
        if (!cmd.foundOption("predicted_labels")) {
            cerr << "Error! Option --predicted_labels not found!" << endl;
            return 1;
        }
            // File to save predictions
        string prediction_file = cmd.optionValue("predicted_labels");
            // Predict data
        PredictData(data_file, model_file, prediction_file);
    }
}
