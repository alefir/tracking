#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

int main(int argc, char** argv)
{
	VideoCapture cap(0);
	if(!cap.isOpened())
		return -1;
	namedWindow("window", CV_WINDOW_FREERATIO);
	Mat frame;
	for(;;){
		cap.read(frame);
		if(frame.empty()) return -2;

		imshow("window", frame);
		cvWaitKey(20);
		if(waitKey(5) >= 0) break;
	}
	return 0;
}
