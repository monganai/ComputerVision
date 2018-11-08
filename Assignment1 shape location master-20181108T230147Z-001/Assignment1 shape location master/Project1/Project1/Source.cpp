#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <list>


using namespace std;
using namespace cv;





int PointDistance(CvPoint point1, CvPoint point2);                                                        // Function to calculate the distance between two CvPoints  
bool ValidRatioRectangle(CvPoint point1, CvPoint point2, CvPoint point3);                                          //  function to check if countors found represent a traffic light ;
Mat CreateStandaloneTrafficImage(CvPoint point1, CvPoint point2, Mat images[], int index);                //  using a retcangle mask extract the traffic light from src image 
list<Mat> findContourslegacy(Mat image, Mat binaryMat, Mat image_array[], int index);                     //  using contours with 4 or 6 vertices detect/draw around traffic lights  old cv functions
void whatState(list<Mat> TrafficMats, Mat lights[]);
int whatThreshold(int index);




	/** @function main */
int main(int argc, char** argv)
{

	Mat image_array[14];                                                                              // create an array of images and load in the images
	image_array[0] = imread("CamVidImages/CamVidLights01.png");
	image_array[1] = imread("CamVidImages/CamVidLights02.png");
	image_array[2] = imread("CamVidImages/CamVidLights03.png");
	image_array[3] = imread("CamVidImages/CamVidLights04.png");
	image_array[4] = imread("CamVidImages/CamVidLights05.png");
	image_array[5] = imread("CamVidImages/CamVidLights06.png");
	image_array[6] = imread("CamVidImages/CamVidLights07.png");
	image_array[7] = imread("CamVidImages/CamVidLights08.png");
	image_array[8] = imread("CamVidImages/CamVidLights09.png");
	image_array[9] = imread("CamVidImages/CamVidLights10.png");
	image_array[10] = imread("CamVidImages/CamVidLights11.png");
	image_array[11] = imread("CamVidImages/CamVidLights12.png");
	image_array[12] = imread("CamVidImages/CamVidLights13.png");
	image_array[13] = imread("CamVidImages/CamVidLights14.png");

	Mat lights[4];
	lights[0] = imread("lights/green.png");
	lights[1] = imread("lights/red.png");
	lights[2] = imread("lights/orange.png");
	lights[3] = imread("lights/redorg.png");



	bool finished = false;          // basic user interface 
	int index = 0;
	while (finished != true) {
		cout << "image index ";
		cin >> index;

		Mat image= image_array[index];											  //load image from the array
		int thresVal = whatThreshold(index);
		cv::Mat grayscaleMat(image.size(), CV_8U);							      // greyscale matrix
		cv::cvtColor(image, grayscaleMat, CV_BGR2GRAY);                           // convert image to greyscale 
		cv::Mat binaryMat(grayscaleMat.size(), grayscaleMat.type());              // binary matrix
		cv::threshold(grayscaleMat, binaryMat, thresVal, 255, cv::THRESH_BINARY);       // convert the greyscale mat to binary
		//imshow("binary", binaryMat);

		list<Mat> TrafficMats = findContourslegacy(image, binaryMat, image_array, index);
		whatState(TrafficMats,lights);
		cvWaitKey(0);
		cout << "run with another image ?  \n  enter index : ";              // basic user interface 

	}

	return 0;
}

int PointDistance(CvPoint point1, CvPoint point2) {                                           //simple geometry distance formula

	int x1 = point1.x;
	int x2 = point2.x;
	int y1 = point1.y;
	int y2 = point2.y;
	int x = x2 - x1;
	int y = y2 - y1;
	int xsq = x * x;
	int ysq = y * y;

	return sqrt(xsq + ysq);                                                                   // complains of loss of acuracy but decimals not needed for this application

}


bool ValidRatioRectangle(CvPoint point1, CvPoint point2, CvPoint point3) {                             //  checks if found rectangles are in the right proportion

	int Top_Length = PointDistance(point1, point2);
	int Right_Side = PointDistance(point2, point3);
	int hyp = PointDistance(point3, point1);

	double topsq = Top_Length * Top_Length;  //a2
	double rightsq = Right_Side * Right_Side; // b2
	double hypsq = hyp * hyp;                 //c2
	double cosC = (topsq + rightsq - hypsq) / (2 * Top_Length * Right_Side);
	double angle = fabs(acos(cosC));        // positive value of the inverse of cosC

	if (Right_Side > (2 * Top_Length)  && angle < 1.8 && angle > 1.45 && Right_Side > 20 && Top_Length > 9 ) {
		return true;
	}
	return false;

}


bool ValidRatioFilterLeft(CvPoint point1, CvPoint point2, CvPoint point3) {
	int Top_Length = PointDistance(point1, point2);
	int Right_Side = PointDistance(point2, point3);
	if (Right_Side > 2 * Top_Length && Top_Length > 8 && Right_Side > 30) {
		return true;
	}
	return false;
}

Mat CreateStandaloneTrafficImage(CvPoint point1, CvPoint point2, Mat images[], int index) {
	Rect R(point1, point2);																	     //   create a rectangle mask for the traffic light,  top left bottom right coodinates
	Mat TrafficLight = images[index](R);                                                         //   apply mask to original colour image 
	return TrafficLight;
}


list<Mat> findContourslegacy(Mat image, Mat binaryMat, Mat image_array[], int index) {
	IplImage*  binaryImage = cvCloneImage(&(IplImage)binaryMat);                            // converting mat to ilpimage     cvFindContours & cvApprox poly incompatable with mat 
	IplImage* sourceImage = cvCloneImage(&(IplImage)image);                                 // converting mat to ilpimage
	list<Mat> matList;                                                                      // store found trafficlight images
	CvSeq* contours;															            //pointer for contours
	CvSeq* contour_Count;															     	//hold sequence of points of a contour
	CvMemStorage *contour_Storage = cvCreateMemStorage(0);								    //memory allocation not done automatically
	cvFindContours(binaryImage, contour_Storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));  // find all contours in the binary image

	while (contours->h_next != NULL)       // linked list style iteration go while next is not null
	{
	contour_Count = cvApproxPoly(contours, sizeof(CvContour), contour_Storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);  // count vertices on discovered contours
		if (contour_Count->total == 4)                                                                             // case for Standard Traffic Lights ( drawn in red )
		{
			CvPoint *CornerPoints[4];
			for (int i = 0; i < 4; i++) {
				CornerPoints[i] = (CvPoint*)cvGetSeqElem(contour_Count, i);
			}
			Point topLeft = *CornerPoints[0];
			Point topRight = *CornerPoints[1];
			Point bottomRight = *CornerPoints[2];
			Point bottomLeft = *CornerPoints[3];
			int diaglr = PointDistance(topLeft, bottomRight);
			int diagrl = PointDistance(topRight, bottomLeft);
			
			if (ValidRatioRectangle(topLeft, topRight, bottomRight) == true  ) {                    // if the found rectangle matches my critera draw around the edges.

				cvLine(sourceImage, topLeft, topRight, cvScalar(0, 0, 255), 2);                                // draw from point to point 
				cvLine(sourceImage, topRight, bottomRight, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, bottomRight, bottomLeft, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, bottomLeft, topLeft, cvScalar(0, 0, 255), 2);
				cout << "topLeft point :";
				cout << topLeft;                                                                               // points printed to console for ground truith
				cout << "\n";
				cout << "bottomRight point :";
				cout << bottomRight;
				cout << "\n";
				
				Mat light = CreateStandaloneTrafficImage(*CornerPoints[0], *CornerPoints[2], image_array, index);     // using a mask create an image of the found traffic light
				matList.push_front(light);

			}
		}


		if (contour_Count->total == 6)
		{
			CvPoint *CornerPoints[6];                                                                                                      // Traffic lights with filter lights
			for (int i = 0; i < 6; i++) {
				CornerPoints[i] = (CvPoint*)cvGetSeqElem(contour_Count, i);
			}

			int topLength = PointDistance(*CornerPoints[0], *CornerPoints[1]);                                   // getting the distance between two points as the border lengths
			int bottomLength = PointDistance(*CornerPoints[4], *CornerPoints[5]);
			int leftLength = PointDistance(*CornerPoints[0], *CornerPoints[5]);
			int rightLength = PointDistance(*CornerPoints[1], *CornerPoints[2]) + PointDistance(*CornerPoints[3], *CornerPoints[4]);

			Point topLeft = *CornerPoints[0];
			Point topRight = *CornerPoints[1];
			Point middleLeft = *CornerPoints[2];
			Point middleRight = *CornerPoints[3];
			Point bottomRight = *CornerPoints[4];
			Point bottomLeft = *CornerPoints[5];

			if (leftLength > rightLength && topLength < bottomLength && ValidRatioFilterLeft(topLeft, topRight, bottomLeft) == true) {
				
				cvLine(sourceImage, topLeft, topRight, cvScalar(0,0,255), 2);               // draw from point to point 
				cvLine(sourceImage, topRight, middleLeft, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, middleLeft, middleRight, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, middleRight, bottomRight, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, bottomRight, bottomLeft, cvScalar(0, 0, 255), 2);
				cvLine(sourceImage, bottomLeft, topLeft, cvScalar(0, 0, 255),2);

				Mat light = CreateStandaloneTrafficImage(*CornerPoints[0], *CornerPoints[4], image_array, index);
				matList.push_front(light);

				cout << "topLeft point :";
				cout << topLeft;                                                                               // points printed to console for ground truith
				cout << "\n";
				cout << "bottomRight point :";
				cout << bottomRight;
				cout << "\n";
			}

			/*if (PointDistance(*CornerPoints[2], *CornerPoints[3])  > PointDistance(*CornerPoints[5], *CornerPoints[4]) && PointDistance(*CornerPoints[3], *CornerPoints[4]) > 10) {   // && ValidRatio(*CornerPoints[2], *CornerPoints[3], *CornerPoints[4]) == true

				cvLine(sourceImage, topLeft, topRight, cvScalar(255, 0, 0), 2);               // draw from point to point
				cvLine(sourceImage, topRight, middleLeft, cvScalar(255, 0, 0), 2);
				cvLine(sourceImage, middleLeft, middleRight, cvScalar(255, 0, 0), 2);
				cvLine(sourceImage, middleRight, bottomRight, cvScalar(255, 0, 0), 2);
				cvLine(sourceImage, bottomRight, bottomLeft, cvScalar(255, 0, 0), 2);
				cvLine(sourceImage, bottomLeft, topLeft, cvScalar(255, 0, 0), 2);

				cout << "\n";                                        // points printed to console for ground truith
				cout << CornerPoints[5]->x;
				cout << CornerPoints[5]->y;
				cout << "\n";

				Mat light = CreateStandaloneTrafficImage(*CornerPoints[0], *CornerPoints[4], image_array, index);
				matList.push_front(light);
			}*/

		}

        contours = contours->h_next;																 // next contour in the list 
		cvShowImage("image with lights highlighted", sourceImage);                                  // show original image with drawn contours 
	}
	return matList;
}


void whatState(list<Mat> TrafficMats, Mat lights[]) {

	double score = 0;
	int whati = 0;
	int bins = 25;
	Mat hsvLight;
	Mat hueLight;
	Mat hsvTemplate;
	Mat hueTemplate;
	Mat hist1;
	Mat hist2;
	int ch[] = { 0, 0 };
	int histSize = MAX(bins, 2);
	float hue_range[] = { 0, 180 };
	const float* ranges = { hue_range };

	while (!TrafficMats.empty()) {                                            // iterate and dispaly found sub images

		Mat foundLight = TrafficMats.front();
		//imshow("Found light", foundLight);
		cvtColor(foundLight, hsvLight, COLOR_BGR2HSV);
		hueLight.create(hsvLight.size(), hsvLight.depth());
		mixChannels(&hsvLight, 1, &hueLight, 1, ch, 1);
		calcHist(&hueLight, 1, 0, Mat(), hist1, 1, &histSize, &ranges, true, false);
		normalize(hist1, hist1, 0, 255, NORM_MINMAX, -1, Mat());

		double maxVal = 0.0;
		int bestMatchi = -1;
		for (int i = 0; i < 4; i++) {
	
			cvtColor(lights[i], hsvTemplate, COLOR_BGR2HSV);
			hueTemplate.create(hsvTemplate.size(), hsvTemplate.depth());
			mixChannels(&hsvTemplate, 1, &hueTemplate, 1, ch, 1);
			calcHist(&hueTemplate, 1, 0, Mat(), hist2, 1, &histSize, &ranges, true, false);
			normalize(hist2, hist2, 0, 255, NORM_MINMAX, -1, Mat());

			
			double percentage = compareHist(hist1, hist2, 0);
			if (percentage > maxVal ) {
				if (percentage > 0.1) {
					maxVal = percentage;
					bestMatchi = i;
				}
			}
			
		}

		cout << maxVal;
		switch (bestMatchi) {
		case 0:
			cout << "     green    " << endl;
			break;
		case 1:
			cout << "     orange     " << endl;
			break;
		case 2:
			cout << "    red     " << endl;
			break;
		case 3:
			cout << "      red and orange     " << endl;
			break;
		
		default:
			cout << "error" << endl;
		}


		TrafficMats.pop_front();
	}

}


int whatThreshold(int index) {

	switch (index) {
	case 1:
	case 6:
	case 7:
		return 60;
		break;
	
	case 8:
	case 11:
	case 10:
		return 50;
		break;
	
	
	
	default:
		return 55;
	}



}


