//------------------------------------------------------------------------------------------------- 
// EINDHOVEN UNIVERSITY OF TECHNOLGY
//------------------------------------------------------------------------------------------------- 
// EMBEDDED VISUAL CONTROL
// TEAM 18
// PROJECT SUNRISE
// AUTHOR:
//  * Luis Albert Zavala Mondragón, student ID: 0977512
// Description:
//      This program estimate the traffc signs based on tempate match
//      ing. Since it is meant to be used within an embedded platform, in oder to reduce compu
//      tations the matching is done in a 1:1 comparison on a a scale of 32x32 picels. To 
//      identify a match the total proportion of white pixels is compared between the two
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
// Includes
//------------------------------------------------------------------------------------------------- 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "trafficSignsDetection.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
using namespace cv;
using namespace std;
//------------------------------------------------------------------------------------------------- 
// Global variables
//------------------------------------------------------------------------------------------------- 
Mat grayArrowStraight   = imread("32x32_MonochromeTemplates/binary_arrowStraight_1.bmp" ,CV_LOAD_IMAGE_GRAYSCALE);
Mat grayArrowRight      = imread("32x32_MonochromeTemplates/binary_arrowRight_1.bmp"    ,CV_LOAD_IMAGE_GRAYSCALE);   
Mat grayArrowLeft       = imread("32x32_MonochromeTemplates/binary_arrowLeft_1.bmp"     ,CV_LOAD_IMAGE_GRAYSCALE);    
Mat grayStop            = imread("32x32_MonochromeTemplates/binary_stop.bmp"            ,CV_LOAD_IMAGE_GRAYSCALE);
Mat grayUturn           = imread("32x32_MonochromeTemplates/binary_uturn.bmp"           ,CV_LOAD_IMAGE_GRAYSCALE);   


//-------------------------------------------------------------------------------------------------
// MAIN FUNCITON
//-------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    #ifdef SHOW_REFERENCES
    imshow("grayArrowStraight   ",grayArrowStraight   );
    imshow("grayArrowRight      ",grayArrowRight      );
    imshow("grayArrowLeft       ",grayArrowLeft       );
    imshow("GrayStop",grayStop);
    imshow("GrayUturn",grayUturn);
    #endif

    string filename  = argc >= 2 ? argv[1] : "../videos/video_new.h264";//"../videos/video_new.h264";
    VideoCapture capture(filename);
    if( !capture.isOpened() )
        throw "Error when reading video";
    namedWindow( "w", 1);
    for( ; ; ) {
        Mat frame;
        Mat src ;
        Mat shortWindow;
        capture >> frame;
        resize(frame, src, Size(), 0.25, 0.25, INTER_LINEAR);

        #ifdef PROFILING 
        clock_t begin, end;
        double time_spent;   
        begin = clock();
        #endif
        // **** Start of the processing stage 
        traffic *trafficSignals;


        array_traffic r = detectObjects(src);
        trafficSignals        = r.signs;
        int noS   = r.noSigns;
        for(int p=0;p<noS;p++){
            putText(src,trafficSignals[p].classification , cvPoint(trafficSignals[p].xpos,trafficSignals[p].ypos),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
        }
        #ifdef PROFILING
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Execution time: %f seconds\n",time_spent);
        #endif
   

        imshow("w", src);
        waitKey(20); // waits to display frame

        free(trafficSignals);

    }
    waitKey(0); 
    return 0; 
}


//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
// DETECT OBJECTS 
//-------------------------------------------------------------------------------------------------
// INPUTS:
//  * char color: Character that specifies the color of the objetcs 
//      to be found
//  * Mat in:   Input image in HSV format
//-------------------------------------------------------------------------------------------------
// OPERATION 
//  The code works by performing the following steps:
//      1)  It thrsholds an HSV input image depending on the color
//      2)  It applies a gausian blur to th thresholded image
//      3)  The thrsholded imag is segmented using canny edge detector
//      4)  Approximates the outermost curve by usinng polygons
//      5)  Counting the number of sides we can determine the outer structure
//-------------------------------------------------------------------------------------------------
// Based in the code posted in:
//  * https://solarianprogrammer.com/2015/05/08/detect-red-circles-image-using-opencv/
//  * http://answers.opencv.org/question/28899/correct-hsv-inrange-values-for-red-objects/
//  * http://docs.opencv.org/3.1.0/d9/d8b/tutorial_py_contours_hierarchy.html#gsc.tab=0
//  * http://stackoverflow.com/questions/15751940/opencv-converting-canny-edges-to-contours
//  * http://stackoverflow.com/questions/10238765/contours-opencv-how-to-eliminate-small-contours-in-a-binary-image 
//  * http://stackoverflow.com/questions/22876351/opencv-2-4-8-python-determining-orientation-of-an-arrow
//-------------------------------------------------------------------------------------------------
// Other observations:
//  * In opencv H goes only form 0 to 180
//  * Red goes from 0 to 10
//-------------------------------------------------------------------------------------------------
//array_traffic detectObjects(char color, Mat in){
array_traffic detectObjects(Mat in){
    double area     = 0;
    // **  BEGIN SEGMENTATION & EDGE DETECTION  ** //  
        // Declaring Auxiliary variables
        Mat lower_hue_range;
        Mat upper_hue_range;
        Mat image_r,image_b,image_y;
        Mat cloneIn; 
        //#ifdef GENERATE_TEMPLATE
        Mat outputPrint;
        //#endif
        // In the case of red we have two ranges for HSV images


        // Converting input image to HSV format
        Mat hsv_imag;
        cvtColor(in, hsv_imag, CV_BGR2HSV);

    // Erosion used for Yellow Channel
    int erosion_size = 1;
    int erosion_type = MORPH_RECT;
    Mat element = getStructuringElement( erosion_type,
                                       Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                       Point( erosion_size, erosion_size ) );
    inRange(hsv_imag.clone(), Scalar(0, 60, 50), Scalar(10, 255, 255), lower_hue_range);         // segmentation of lower range of red
    inRange(hsv_imag.clone(), Scalar(170, 120, 50), Scalar(179, 255, 255), upper_hue_range);      // Segmentation of upper range of red
    addWeighted(lower_hue_range, 1.0, upper_hue_range, 1.0, 0.0, image_r);            // Adding the two ranges
    inRange(hsv_imag.clone(), Scalar(13, 100, 20), Scalar(25, 255, 255), image_y);
    erode( image_y.clone(), image_y, element );
    inRange(hsv_imag.clone(), Scalar(101, 50, 20), Scalar(128, 255, 255), image_b);

    #ifdef VIDEO_COLORS
    imshow("Red channel",image_r);
    imshow("Yellow channel",image_y);
    imshow("Blue channel",image_b);
    #endif

    
    // **  END SEGMENTATION & EDGE DETECTION    ** // 
    const double thresholdArea    = 0.002*in.size().width*in.size().height;
    

    // ** BEGIN FINDING THE CONTOURS ** //
        vector<vector<Point> >  contours_r,contours_b,contours_y; 
        vector<Vec4i>           r_hierarchy;
        findContours(image_r.clone(),contours_r,r_hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
        findContours(image_y.clone(),contours_y,r_hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
        findContours(image_b.clone(),contours_b,r_hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
    // ** END FINDING THE CONTOURS ** //
        

        // Bounding Box
        Rect                    boundRect;
        Point2f mc;                    
        Moments mu;
    // ** BEGIN SHAPE CLASSIFICAION ** //
        int p = 0; // q is the index for contours, p is the one for the valid items 
        traffic *signs= (traffic *)calloc((contours_r.size()+contours_y.size()+contours_b.size()), sizeof(traffic));   // Allocating memory for the elements of the aray
        //#pragma op parallel
        for( int i = 0; i< contours_r.size(); i++ ){
            mu = moments( contours_r[i], false );
            mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
            area = mu.m00;  // We find the area to filter undesired stuff
            if(area>thresholdArea){                                     // Filtering small onjects that doesn't match
                strcpy(signs[p].color,"RED");
                boundRect = boundingRect( Mat(contours_r[i]) );
                strcpy(signs[p].shape, likeLihood_Stop(image_r(boundRect)));        // Finding wether matches with an arrow or not      
                signs[p].classifyObject();
                if(strcmp(signs[p].classification,"UNKNOWN") != 0){
                    signs[p].xpos = (int)mc.x; 
                    signs[p].ypos = (int)mc.y; 
                    signs[p].area = (int)area;
                    p++;
                }else{
                   signs[p].flush();
                } 
            }
        }
        //#pragma op parallel
        for( int i = 0; i< contours_y.size(); i++ ){
            mu = moments( contours_y[i], false );
            mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
            area = mu.m00;  // We find the area to filter undesired stuff
            if(area>thresholdArea){                                     // Filtering small onjects that doesn't match
                strcpy(signs[p].color,"YELLOW");
                boundRect = boundingRect( Mat(contours_y[i]) );
                strcpy(signs[p].shape, likeLihood_Uturn(image_y(boundRect)));        // Finding wether matches with an arrow or not      
                signs[p].classifyObject();
                if(strcmp(signs[p].classification,"UNKNOWN") != 0){
                    signs[p].xpos = (int)mc.x; 
                    signs[p].ypos = (int)mc.y;
                    signs[p].area = (int)area;
                    p++;
                
                }else{
                   signs[p].flush();
                } 
            }
        }
        //#pragma op parallel
        for( int i = 0; i< contours_b.size(); i++ ){
            Moments mu = moments( contours_b[i], false );
            mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
            area = mu.m00;  // We find the area to filter undesired stuff
            if(area>thresholdArea){                                     // Filtering small onjects that doesn't match
                strcpy(signs[p].color,"BLUE");
                boundRect = boundingRect( Mat(contours_b[i]) );
                strcpy(signs[p].shape,"CIRCLE");
                strcpy(signs[p].ishape, likeLihood(image_b(boundRect)));        // Finding wether matches with an arrow or not      
                signs[p].classifyObject();
                if(strcmp(signs[p].classification,"UNKNOWN") != 0){
                    signs[p].xpos = (int)mc.x; 
                    signs[p].ypos = (int)mc.y; 
                    signs[p].area = (int)area;
                    p++;
                }else{
                    signs[p].flush(); 
                }
            }
        }
    
        // Creating structure for outputting values
        array_traffic w;
        w.signs     = signs;
        w.noSigns   = p;
        return(w);
        
    // ** END SHAPE CLASSIFICAION ** //
        
}

//-------------------------------------------------------------------------------------------------- 
// Likelihood function 
//--------------------------------------------------------------------------------------------------
// DESCRIPTION: 
// Detects which is the most likely sign detected on a given frame
// between an input image and our 3 blue arrows (binarized)
//-------------------------------------------------------------------------------------------------- 
// INPUTS:
//  * Mat   subPic: This is the extracted binarized blob from the image
// OUTPUT:
//  * (*char)  string which indicates the traffic sign detected
//-------------------------------------------------------------------------------------------------- 
char* likeLihood(Mat subPic){

    // Processing Data    
    Mat scaledArrow;
    Mat scaledGray;
    Mat subPicGray;
    Mat masked;

    // ** FEATURE EXTRACTION ** //
    // STRAIGHT ARROW
    resize(subPic ,scaledArrow,grayArrowStraight.size());
    bitwise_xor(grayArrowStraight, scaledArrow,masked);     // Applying Mask
    float sIn = (float)sum( masked )[0]/(255*(scaledArrow.size().height*scaledArrow.size().width));
    float P_straight = (normSref - sIn)/normSref; 
    
    // RIGHT ARROW
    resize(subPic ,scaledArrow,grayArrowRight.size());
    bitwise_xor(grayArrowRight, scaledArrow,masked);     // Applying Mask
    sIn = (float)sum( masked )[0]/(255*(scaledArrow.size().height*scaledArrow.size().width));
    float P_right = (normRref - sIn)/normRref; 

    // RIGHT ARROW
    resize(subPic ,scaledArrow,grayArrowLeft.size());
    bitwise_xor(grayArrowLeft, scaledArrow,masked);     // Applying Mask
    sIn = (float)sum( masked )[0]/(255*(scaledArrow.size().height*scaledArrow.size().width));
    float P_left = (normLref - sIn)/normLref; 

    #ifdef DEBUG
        printf("* DEBUG: Probability of Straight Arrow= %f\n",P_straight);
        printf("* DEBUG: Probability Right Arrow= %f\n",P_right);
        printf("* DEBUG: Probability Left Arrow= %f\n",P_left);
        imshow("subPic blue",scaledArrow);
    #endif
 
    // ** EXECUTING CLASSIFICATION ** // 

    if(P_straight <= MIN_LIKELIHOOD && P_right <= MIN_LIKELIHOOD& P_left <=MIN_LIKELIHOOD){
        return((char*)"UNKNOWN");
    }else{
        if(P_straight>P_right) 
            if(P_straight>P_left){
                return((char*)"STRAIGHT");
            }else{ 
                return((char*)"LEFT");
            }
        else  if(P_right>P_left){ 
                return((char*)"RIGHT");
            }else{ 
                return((char*)"LEFT");
            }
    }
}


//-------------------------------------------------------------------- 
// Likelihood function RED and YELLOW 
//--------------------------------------------------------------------
// DESCRIPTION: 
// Detects which is the most likely sign detected on a given frame
// between an input image and our 3 blue arrows (binarized)
//-------------------------------------------------------------------- 
// INPUTS:
//  * Mat   subPic: This is the extracted binarized blob from the image
// OUTPUT:
//  * (*char)  string which indicates the traffic sign detected
//-------------------------------------------------------------------- 
char* likeLihood_Stop(Mat subPic){

    Mat scaledArrow;
    Mat scaledGray;
    Mat subPicGray;
    Mat masked;

    // ** FEATURE EXTRACTION ** //
    // STRAIGHT ARROW
    resize(subPic ,scaledArrow,grayStop.size());
    bitwise_xor(grayStop , scaledArrow,masked);     // Applying Mask
    //masked = abs(grayStop - scaledArrow);     // Applying Mask
    float sIn = (float)sum( masked )[0]/(255*(scaledArrow.size().height*scaledArrow.size().width));
    float P_stop = abs(normStRef - sIn)/normStRef; 

    #ifdef DEBUG
        printf("Reference %f, Lilekihood %f \n",normStRef,P_stop);
        imshow("subPic_red",scaledArrow);
        //waitKey();
    #endif
 
    // ** EXECUTING CLASSIFICATION ** // 
    if(P_stop<= MIN_LIKELIHOOD_STOP)
        return((char*)"UNKNOWN");
    else
        return((char*)"STOP");
                        
    
}

char* likeLihood_Uturn(Mat subPic){

    Mat scaledArrow;
    Mat scaledGray;
    Mat subPicGray;
    Mat masked;

    // ** FEATURE EXTRACTION ** //
    // STRAIGHT ARROW
    resize(subPic ,scaledArrow,grayUturn.size());
    bitwise_xor(grayUturn , scaledArrow,masked);     // Applying Mask
    //masked = abs(grayUturn - scaledArrow);     // Applying Mask
    float sIn = (float)sum( masked )[0]/(255*(scaledArrow.size().height*scaledArrow.size().width));
    float P_Uturn = abs(normUtRef - sIn)/normUtRef; 
   
    #ifdef DEBUG
        printf("Refrence %f, Lilekihood %f \n",normUtRef,P_Uturn);
        imshow("difference",masked);
        imshow("subPic_red",scaledArrow);
        //waitKey();
    #endif 
 
    // ** EXECUTING CLASSIFICATION ** // 
    if(P_Uturn<= MIN_LIKELIHOOD_UTURN)
        return((char*)"UNKNOWN");
   else
        return((char*)"UTURN");
    
}
