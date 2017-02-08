//-------------------------------------------------------------------- 
// EINDHOVEN UNIVERSITY OF TECHNOLGY
//-------------------------------------------------------------------- 
// EMBEDDED VISUAL CONTROL
// TEAM 18
// PROJECT SUNRISE
// AUTHOR:
//  * Luis Albert Zavala Mondragón, student ID: 0977512
//-------------------------------------------------------------------- 

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;
using namespace std;
RNG rng(12345);

//-------------------------------------------------------------------- 
// CONSTANTS
//-------------------------------------------------------------------- 
#define normSref 0.626271
#define normRref 0.610015
#define normLref 0.610139
#define normStRef 0.612975 
#define normUtRef 0.381269
#define MIN_LIKELIHOOD 0.7
#define MIN_LIKELIHOOD_UTURN 0.5
#define MIN_LIKELIHOOD_STOP  0.65
//#define DEBUG 1
//#define VIDEO_TEST 1
//#define VIDEO_COLORS
//#define GENERATE_TEMPLATE_YELLOW 1
//#define PROFILING 1
//#define SHOW_REFERENCES 1 
// Class used for the traffic signs
class traffic{
    public:
        char    color[10];
        char    shape[10];
        char    ishape[10];
        char    classification[10];     
        int     xpos;
        int     ypos;
        int     area;
    public:
        void classifyObject(){

            if( strcmp(color,"RED") == 0 ){
                if( strcmp(shape,"STOP")  == 0){
                    strcpy(classification,"STOP"); 
                }else{
                    strcpy(classification,"UNKNOWN"); 
                }
            }else if( strcmp(color,"BLUE") == 0 ){
                if( strcmp(shape,"CIRCLE")  == 0)
                    if(strcmp(ishape,"STRAIGHT")  == 0)
                        strcpy(classification,"STRAIGHT"); 
                    else if(strcmp(ishape,"RIGHT")  == 0)
                        strcpy(classification,"RIGHT"); 
                    else if(strcmp(ishape,"LEFT")  == 0)
                        strcpy(classification,"LEFT"); 
                    else
                        strcpy(classification,"UNKNOWN"); 
                else
                    strcpy(classification,"UNKNOWN"); 
            }else if( strcmp(color,"YELLOW") == 0 ){
                if( strcmp(shape,"UTURN")  == 0)
                    strcpy(classification,"UTURN"); 
                else
                    strcpy(classification,"UNKNOWN"); 
            }else{
                strcpy(classification,"UNKNOWN"); 
            }
        }
    public:
        void flush(){
            strcpy(color,"UNKNOWN");
            strcpy(shape,"UNKNOWN");
            strcpy(ishape,"UNKNOWN");
            strcpy(classification,"UNKNOWN");     
            xpos = 0;
            ypos = 0;
        }
 };

// Structure used to return back the pointer with the traffic signs and the number of elements
struct array_traffic{
    traffic *signs;
    int noSigns;
};

//-------------------------------------------------------------------
// Prototypes
//-------------------------------------------------------------------
//array_traffic detectObjects(char color,Mat in);
array_traffic detectObjects(Mat in);
char* likeLihood(Mat subPic);
void initializeReferences(void);
char* likeLihood_Uturn(Mat subPic);
char* likeLihood_Stop(Mat subPic);
array_traffic findTrafficSigns(Mat hsv_imag);


