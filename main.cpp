#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>


#include "poest.h"
//#include "pointcloud.h"
using namespace cv;
using namespace std;

/*  Pose estimation
 *  s*m'=K*[R|t]*M
 *  Since K can be acquired by camera calibration with a chessboard
 *  A set of ways to estimate [R|t] are implemented.
 */
vector<Point3f> world_coord;
vector<Point2f> image_coord;
Mat camera_matrix=(Mat_<double>(3,3)<< 486.9, 0, 319.5, 0, 487.2, 239.5, 0, 0, 1);
vector<double> disto;
Mat R;
Mat t;

Mat image,imgL,imgR;
Pose_est poseEst;
ImageProcess imgprocess;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        //poseEst.PnPmethod(x,y);
        //poseEst.Featuremethod();
        poseEst.ORB_matching(imgL,imgR,10);
        //poseEst.stereo_test(imgL,imgR);
        //imgprocess.SliceImage(imgL,image);
    }
}

inline void ReadImage(const char *URL1,const char *URL2)
{
    imgL=imread(URL1,CV_LOAD_IMAGE_COLOR);
    imgR=imread(URL2,CV_LOAD_IMAGE_COLOR);
    //imgL=imread(URL1,CV_LOAD_IMAGE_GRAYSCALE);
    //imgR=imread(URL2,CV_LOAD_IMAGE_GRAYSCALE);
    imgprocess.SliceImage(imgL,imgL);
    imgprocess.SliceImage(imgR,imgR);

}

int main( int argc, char** argv)
{


    if( argc < 2)
    {
        cout <<" Usage: poest ImageToLoad" << endl;
        return -1;
    }
    world_coord.push_back(Point3f(0,0,0));
    world_coord.push_back(Point3f(0,29.8,0));
    world_coord.push_back(Point3f(0,29.8,4.6));
    world_coord.push_back(Point3f(0,0,4.6));
    world_coord.push_back(Point3f(10.8,0,0));
    world_coord.push_back(Point3f(10.8,29.8,4.6));


    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file
    ReadImage(argv[1],argv[2]);


    namedWindow( "PnP", WINDOW_AUTOSIZE );// Create a window for display.
    //set the callback function for any mouse event
    setMouseCallback("PnP", CallBackFunc, NULL);
    while(waitKey(50)<0)
    {
        imshow("PnP", image);                   // Show our image inside it.

    }



    return 0;
}