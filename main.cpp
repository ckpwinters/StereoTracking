#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>

#include "poest.h"
//#include "pointcloud.h"
using namespace cv;
using namespace std;

/*  Pose estimation
 *  s*m'=K*[R|t]*M
 *  Since K can be acquired by camera calibration with a chessboard
 *  A set of ways to estimate [R|t] are implemented.
 */

Mat image;
vector<Point3d> world_coord;
//vector<Point2d> image_coord;
vector<Point3d> world_coord_2;
Mat R,t;
Mat R2,t2;


Mat imgL,imgR;
FeaturedImg image_L,image_R;
FeaturedImg image_L2,image_R2;
Mat imgL_2,imgR_2;
vector<Point2d> matches_L,matches_R;
vector<Point2d> matches_L2,matches_R2;
vector<Point3d> matches_P,matches_N;
PoseEst poseEst;


FeaturedImg first;
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        //poseEst.PnPmethod(x,y);
        //poseEst.Featuremethod();
        //poseEst.stereo_test(imgL,imgR);
        if(R.rows==0)
        {

            StereoImageProcess imgprocess;

            Mat img_matched;

            //Timer starts.
            auto start=std::chrono::system_clock::now();

            ChessboardGTruth gt;

            //gt.FindCorners(imgR,matches_R);
            gt.OneFrameTruth(imgL,imgR,R,t,matches_L,matches_R,world_coord);

            gt.OneFrameTruth(imgL_2,imgR_2,R2,t2,matches_L2,matches_R2,world_coord_2);
            ObjectTracker tk;

            tk.RansacMotion(world_coord,world_coord_2,R,t,30000,6,0.9);

            //tk.CalcMotions(a,b,R,t);
            //tk.CalcRTerror(R,t,world_coord,world_coord_2);


            //imgprocess.ImageInput(imgL,image_L.img,imgR,image_R.img);
            //imgprocess.FeaturesMatching(image_L,image_R,img_matched);
            //imgprocess.StereoConstruct(image_L,image_R,matches_L,world_coord);

            //poseEst.SolvePnP(matches_L, world_coord, R, t);
            //Timer ends.
            auto end=std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            cout<<"Pose estimation time:"<<elapsed_seconds.count()<<endl;


            /**For debugging.Input all data into a file**/
            fstream file;
            file.open("points.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
            file << "matched_points of the left image:" << endl;
            for (auto i:matches_L)
                file << i << " ";
            file << endl << "matched_points of the right image:" << endl;
            for (auto j:matches_R)
                file << j << " ";
            file << endl << "3D reconstruction:" << endl;
            for (auto k:world_coord)
                file << k << " ";
            file << endl << "3D reconstruction2:" << endl;
            for (auto k:world_coord_2)
                file << k << " ";
            file << "\n"<<endl ;
            file.close();
        }
        //For debug
        else
        {
            /*
            poseEst.MarkPtOnImg(image,Point2d(x,y));
            Point3d A=poseEst.CalcWldCoord(R,t,Point2d(x,y));
            cout<<"Let's see Point 3d:"<<A<<endl;
            */
            cout<<"can't run?"<<endl;
        }


    }
    if  ( event == EVENT_RBUTTONDOWN )
    {
        cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;

        StereoImageProcess imgprocess;
        Mat img_matched;
        /**For debugging.Input all data into a file**/
        //imgprocess.ImageInput(imgL_2,imgL_2,imgR_2,imgR_2);
        //imgprocess.PrintCorners();

        //imgprocess.FeaturesMatching(imgL, imgR, 5, matches_L, matches_R);

        //auto second=imgprocess.Matching(imgL_2,imgR_2,5,matches_L2,matches_R2);
        //imgprocess.StereoConstruct(matches_L2, matches_R2, world_coord_2, 120.0, 2.5);
        //poseEst.SolvePnP(matches_L2, world_coord_2, R, t);

        int feature_type=ORB_FREAK;
        auto start=std::chrono::system_clock::now();


        bool status=imgprocess.ImageInput(imgL,image_L.img,imgR,image_R.img);
        if(status==false)
        {
            imgprocess.ImageInput(imgL, image_L.img, imgR, image_R.img);
        }
        imgprocess.FeaturesMatching(image_L,image_R,img_matched,feature_type);
        //imshow("img_matched",img_matched);

        imgprocess.StereoConstruct(image_L,image_R,matches_L,world_coord);
        poseEst.PnPCheck(image_L,R,t);
        cout<<"Frame One OK."<<endl;


        imgprocess.ImageInput(imgL_2,image_L2.img,imgR_2,image_R2.img);
        imgprocess.FeaturesMatching(image_L2,image_R2,img_matched,feature_type);
        //imshow("img_matched",img_matched);

        imgprocess.StereoConstruct(image_L2,image_R2,matches_L2,world_coord_2);
        //poseEst.SolvePnP(matches_L2,world_coord_2,R2,t2);
        poseEst.PnPCheck(image_L2,R2,t2);

        cout<<"Frame Two OK."<<endl;



        ObjectTracker tk(image_L);
        vector<DMatch> a;
        tk.Track(image_L2,a,feature_type);

        world_coord.clear();
        world_coord_2.clear();
        for(int i=0;i<a.size();i++)
        {
            world_coord.push_back(image_L.matched_3d[a[i].queryIdx]);
            world_coord_2.push_back(image_L2.matched_3d[a[i].trainIdx]);

        }

        cout<<"Ransac Ready!"<<endl;


        tk.RansacMotion(world_coord,world_coord_2,R,t,200,9,0.6);


        auto end=std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        //Debug info

        Mat_<double> R_t;
        hconcat(R,t,R_t);
        auto mat_print=[](Mat &a){
            //cout<<"[";
            for(int i=0; i<a.rows; i++)
            {
                for (int j = 0; j < a.cols; j++)
                    cout << fixed<<setprecision(4) << a.at<double>(i, j) << ",";
                cout<<endl;
            }
        };


        cout<<" Best R|t:"<<endl;
        mat_print(R_t);
        cout<<"Pose estimation time:"<<elapsed_seconds.count()<<endl;


        /*
        vector<DMatch> matched;


        ObjectTracker tk(image_L);
        //cout<<"Debug info"<<endl;
        tk.Track(image_L2,matched);

        vector<Point3d> a;
        vector<Point3d> b;

        Mat rotation,translation;

        cout<<"debug info:matched_3d size is"<<image_L.matched_3d.size()<<endl;
        //for(auto i:image_L.matched_3d)
        //    cout<<"first matched 3d:"<<i<<endl;

        for(int i=0;i<3;i++)
        {
            a.push_back(image_L.matched_3d[matched[i].queryIdx]);
            b.push_back(image_L2.matched_3d[matched[i].trainIdx]);
        }
        for(auto i:a)
            cout<<"points from a:"<<i<<endl;
        for(auto j:b)
            cout<<"points from b:"<<j<<endl;

        tk.CalcMotions(a,b,rotation,translation);
        */


        /*
         * Debug info
         */

        fstream file;
        file.open("points.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
        file << "matched_points of the second left image:" << endl;
        for (auto i:matches_L2)
            file << i << " ";
        file << endl << "matched_points of the second right image:" << endl;
        for (auto j:matches_R2)
            file << j << " ";
        file << endl << "3D reconstruction:" << endl;
        for (auto k:world_coord_2)
            file << k << " ";
        file << endl;
        file.close();

    }
}

inline void ReadImage(const char *URL1,const char *URL2,Mat &img1,Mat &img2)
{
    img1=imread(URL1,CV_LOAD_IMAGE_COLOR);
    img2=imread(URL2,CV_LOAD_IMAGE_COLOR);

    //img1=imread(URL1,CV_LOAD_IMAGE_GRAYSCALE);
    //img2=imread(URL2,CV_LOAD_IMAGE_GRAYSCALE);

    //imgprocess.ImageInput(imgL,imgL,imgR,imgR);
    //imgprocess.PrintCorners();

    //imshow("imgL",imgL);
    //imshow("imgR",imgR);



}

void Test()
{
    StereoImageProcess imgprocess;

    Mat img_matched;
    int flag=ORB_FEATURE;

    //Timer starts.
    auto start=std::chrono::system_clock::now();

    StereoImageProcess pipeline;
    imgprocess.ImageInput(imgL,image_L.img,imgR,image_R.img);
    imgprocess.FeaturesMatching(image_L,image_R,img_matched,flag);
    //imshow("img_matched",img_matched);
    imwrite("../matched.jpg",img_matched);

    imgprocess.StereoConstruct(image_L,image_R,matches_L,world_coord);


    //ChessboardGTruth gt;

    //gt.FindCorners(imgR,matches_R);
    //gt.OneFrameTruth(imgL,imgR,R,t,matches_L,matches_R,world_coord);
    /*
    ObjectTracker a;
    Mat r,t;
    vector<Point3d> ref;
    ref.push_back(Point3d(8.1196, -13.7607, 459.5165));
    ref.push_back(Point3d(-23.4191, 27.1433, 460.9348));
    ref.push_back(Point3d(79.5689, -7.6724, 463.4779));

    vector<Point3d> tgt;
    tgt.push_back(Point3d(-60.0862, -14.9138, 463.4779));
    tgt.push_back(Point3d(-92.7118, 25.9826, 466.6965));
    tgt.push_back(Point3d(11.1966, -8.6325, 459.5166));
    a.CalcMotions(ref,tgt,r,t);
    Mat_<double> R_t;
    hconcat(r,t,R_t);
    auto mat_print=[](Mat &a){
        //cout<<"[";
        for(int i=0; i<a.rows; i++)
        {
            for (int j = 0; j < a.cols; j++)
                cout << fixed<<setprecision(4) << a.at<double>(i, j) << ",";
            cout<<endl;
        }
    };
    cout<<"R|t:";
    mat_print(R_t);
    Mat camera_matrix=(Mat_<float>(3,2)<< 537.6, 0., 400,
            0., 537.6,300);
    cout<<camera_matrix.size()<<endl;
     */


}
int main( int argc, char** argv)
{




    if( argc == 3)
    {
        ReadImage(argv[1],argv[2],imgL,imgR);
        Test();
        cout <<" Usage: poest ImageToLoadL ImageToLoadR" << endl;
        return -1;
    }
    else if( argc != 5)
        return -2;
    ReadImage(argv[1],argv[2],imgL,imgR);
    ReadImage(argv[3],argv[4],imgL_2,imgR_2);

    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file


    cout<<"image size:"<<image.rows<<"*"<<image.cols<<endl;
    namedWindow( "PnP", WINDOW_AUTOSIZE );// Create a window for display.
    //set the callback function for any mouse event
    setMouseCallback("PnP", CallBackFunc, NULL);
    while(waitKey(50)<0)
    {
        imshow("PnP", image);                   // Show our image inside it.

    }



    return 0;
}