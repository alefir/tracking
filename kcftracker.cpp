#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.2
    // NOTE : GOTURN implementation is buggy and does not work.
    string trackerTypes[6] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[2];

    Ptr<Tracker> tracker;

    #if (CV_MINOR_VERSION < 3)
    {
        tracker = Tracker::create(trackerType);
    }
    #else
    {
        if (trackerType == "BOOSTING")
            tracker = TrackerBoosting::create();
        if (trackerType == "MIL")
            tracker = TrackerMIL::create();
        if (trackerType == "KCF")
            tracker = TrackerKCF::create();
        if (trackerType == "TLD")
            tracker = TrackerTLD::create();
        if (trackerType == "MEDIANFLOW")
            tracker = TrackerMedianFlow::create();
        if (trackerType == "GOTURN")
            tracker = TrackerGOTURN::create();
    }
    #endif
    // Read video
    VideoCapture video("testfootage.mp4");
    
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl;
        return 1;
        
    }
    
    // Read first frame
    Mat frame;
    bool ok = video.read(frame);
    
    for(int i=0;i<600;i++){video.read(frame);}
    // Define initial boundibg box
    Rect2d bbox(287, 23, 86, 320);
    
    // Uncomment the line below to select a different bounding box
    bbox = selectROI(frame, false);

    // Display bounding box.
    rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
    tracker->init(frame, bbox);
    Mat thresh;
    destroyAllWindows();
    while(video.read(frame))
    {     
    	// Equalize Histogram (maximize image contrast)
        double timer = (double)getTickCount();
        ok = tracker->update(frame, bbox);
        float fps = getTickFrequency() / ((double)getTickCount() - timer);
        int xPos = 0, yPos = 0;
        if (ok)
        {
            // Tracking success : Draw the tracked object
            rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
            line(frame, Point(bbox.x+(bbox.width/2), bbox.y+(bbox.height/2)), Point(frame.size().width/2, frame.size().height/2), Scalar(0,255,0), 2, LINE_8, 0);
	    xPos = (bbox.x+(bbox.width/2)) - frame.size().width/2;
	    yPos = (bbox.y+(bbox.height/2)) - frame.size().height/2;
        }
        else
        {
            // Tracking failure detected.
            putText(frame, "Tracking failure detected - Revert to Threshold", Point(100,140), FONT_HERSHEY_DUPLEX, 0.75, Scalar(0,0,255),2);
            tracker->init(frame, bbox);

            Mat frame_thresh;
            inRange(frame, Scalar(240,240,240), Scalar(255,255,255), frame_thresh);
            blur(frame_thresh, frame_thresh, Size(5,5));
            Rect bounding_rect;
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;
            findContours(frame_thresh, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	    int largest_area = 0, largest_contour_index = 0;
            for(int i = 0; i < contours.size(); i++){
            	    double a = contourArea(contours[i], false);
            	    if(a > largest_area){
            	    	    largest_area = a;
            	    	    largest_contour_index = i;
            	    	    bounding_rect = boundingRect(contours[i]);
            	    }
            }
            xPos = (bounding_rect.x+(bounding_rect.width/2)) - frame.size().width/2;
            yPos = (bounding_rect.y+(bounding_rect.height/2)) - frame.size().height/2;
            rectangle(frame, bounding_rect, Scalar(255,0,0), 2, 1);
            line(frame, Point(bounding_rect.x+(bounding_rect.width/2), bounding_rect.y+(bounding_rect.height/2)), Point(frame.size().width/2, frame.size().height/2), Scalar(0,255,0), 2, LINE_8, 0);
            tracker->init(frame,bounding_rect);
        }
        
        // Display tracker type on frame
        putText(frame, trackerType + " Tracker", Point(100,20), FONT_HERSHEY_DUPLEX, 0.75, Scalar(50,170,50),2);
        
        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_DUPLEX, 0.75, Scalar(50,170,50), 2);

	// Display coordinates on frame
	putText(frame, "X : " + SSTR(xPos), Point(100,80), FONT_HERSHEY_DUPLEX, 0.75, Scalar(50,170,50), 2);
	putText(frame, "Y : " + SSTR(yPos), Point(100,110), FONT_HERSHEY_DUPLEX, 0.75, Scalar(50,170,50), 2);
	
	namedWindow("Tracking", CV_WINDOW_FREERATIO);
	imshow("Tracking", frame);
        // Exit if ESC pressed.
        int k = waitKey(1);
        if(k == 27)
        {
            break;
        }

    }
}
