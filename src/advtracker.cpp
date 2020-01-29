#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

//convert to string
#define SSTR(x) static_cast<std::ostringstream &>(\
		(std::ostringstream() << std::dec << x)).str()

Rect2d findBounds(Mat matrix){

	//start binary threshold to identify exhaust plume
	Mat frame_thresh;

	inRange(matrix, Scalar(240,240,240), Scalar(255,255,255), frame_thresh);
	blur(frame_thresh, frame_thresh, Size(5,5));
	Rect2d bounding_rect;
	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hierarchy;
	findContours(frame_thresh, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	//find the largest threshold blob
	int largest_area = 0, largest_contour_index = 0;
	for (int i = 0; i < contours.size(); i++){
		double a = contourArea(contours[i], false);
		if (a > largest_area){
			largest_area = a;
			largest_contour_index = i;
			bounding_rect = boundingRect(contours[i]);
		}
	}

	return bounding_rect;
}

int main(int argc, char **argv){
	
	//create KCF tracker
	Ptr<Tracker> tracker;
	tracker = TrackerKCF::create();

	//read input video
	VideoCapture video("testfootage.mp4");

	//exit if video is not opened correctly
	if (!video.isOpened()){
		std::cout << "Could not read video file" << std::endl;
		return 1;
	}

	//read first frame
	Mat frame;
	bool ok = video.read(frame);
	
	//initialize tracker with bounding rectangle
	Rect2d bounds = findBounds(frame);
	tracker->init(frame, bounds);

	//calculate frames per second
	double fps = video.get(CV_CAP_PROP_FPS);

	while (video.read(frame)){

		ok = tracker->update(frame, bounds);
		if (ok){
			rectangle(frame, bounds, Scalar(255,0,0), 2, 1);
		}
		else{
			bounds = findBounds(frame);
			tracker = TrackerKCF::create();
			tracker->init(frame, bounds);
			rectangle(frame, bounds, Scalar(255,0,0), 2, 1);
			putText(frame, "KCF TRACKING FAILURE", Point(0,60), FONT_HERSHEY_DUPLEX, 0.75, Scalar(0,0,255), 2);
		}

		//display bounding coordinates
		int xPos = (bounds.x+(bounds.width/2)) - frame.size().width/2;
		int yPos = ((bounds.y+(bounds.height/2)) - frame.size().height/2) * -1;
		putText(frame, "X : " + SSTR(xPos), Point(0, 20), FONT_HERSHEY_DUPLEX, 0.75, Scalar(0,0,255), 2);
		putText(frame, "Y : " + SSTR(yPos), Point(0, 40), FONT_HERSHEY_DUPLEX, 0.75, Scalar(0,0,255), 2);

		//display center line
		line(frame, Point(bounds.x+(bounds.width/2), bounds.y+(bounds.height/2)), Point(frame.size().width/2, frame.size().height/2), Scalar(0,255,0), 2, LINE_8, 0);

		//display FPS counter
		putText(frame, "FPS : " + SSTR(int(fps)), Point(0,frame.size().height-20), FONT_HERSHEY_DUPLEX, 0.75, Scalar(0,0,255), 2);

		namedWindow("Tracking", CV_WINDOW_AUTOSIZE);
		imshow("Tracking", frame);
		
		//exit if ESC is pressed
		int k = waitKey(1000/fps);
		if (k == 27){ break; }
	}
}

