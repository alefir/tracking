/* 
Copyright (c) 2020 Zachary Mudd

Permission is  hereby granted,  free of charge,  to any  person obtaining
a  copy  of  this  software   and  associated  documentation  files  (the
"Software"),  to  deal in  the  Software  without restriction,  including
without  limitation the  rights  to use,  copy,  modify, merge,  publish,
distribute, sublicense, and/or sell copies of the Software, and to permit
persons  to whom  the Software  is  furnished to  do so,  subject to  the
following conditions:

The above copyright  notice and this permission notice  shall be included
in all copies or substantial portions of the Software.

THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "thermaltracker.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int cflag = 0, vflag = 0;
	char *vvalue = NULL;
	int cvalue;
	int c;

	while ((c = getopt(argc, argv, "c:v:")) != -1) {
		switch (c) {
			case 'c':
				cflag = 1;
				cvalue = atoi(optarg);
				break;
			case 'v':
				vflag = 1;
				vvalue = optarg;
				break;
			case '?':
				if (optopt == 'v') {
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				}
				return 1;
			default:
				abort();
		}
	}

	cv::VideoCapture video;
	if (cflag) {
		video.open(cvalue);
	} else if (vflag) {
		video.open(vvalue);
	}

	if (!video.isOpened()) {
		fprintf(stderr, "Failed to open video stream\n");
		return 1;
	}

	cv::Mat frame;
	video.read(frame);

	cv::Rect2d bounds = findBounds(frame);

	cv::Ptr<cv::Tracker> tracker = cv::TrackerKCF::create();
	tracker->init(frame, bounds);

	int ok;

	while (video.read(frame)) {
		if (tracker->update(frame, bounds)) {
			cv::rectangle(frame, bounds, cv::Scalar(255,0,0), 2, 1);
		} else {
			bounds = findBounds(frame);
			tracker = cv::TrackerKCF::create();
			tracker->init(frame, bounds);
			cv::rectangle(frame, bounds, cv::Scalar(255,0,0), 2, 1);
			cv::putText(frame, "TRACKING FAILURE", cv::Point(0,60), cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(0,0,255), 2);
		}

		struct trackerOutput output;
		output.xpos = (bounds.x + (bounds.width/2)) - frame.size().width/2;
		output.ypos = ((bounds.y + (bounds.height/2)) - frame.size().height/2) * -1;

		cv::namedWindow("Tracking", cv::WINDOW_AUTOSIZE);
		cv::imshow("Tracking", frame);
		
		//exit if ESC is pressed
		int k = cv::waitKey(10);
		if (k == 27){ break; }
	}


	return 0;
}

cv::Rect2d findBounds(cv::Mat matrix) {
	cv::Mat frame_thresh;
	cv::inRange(matrix, cv::Scalar(240,240,240), cv::Scalar(255,255,255), frame_thresh);
	cv::blur(frame_thresh, frame_thresh, cv::Size(5,5));
	cv::Rect2d bounding_rect;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(frame_thresh, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);


	int largest_area = 0, largest_contour_index = 0;
	for (int i = 0; i < contours.size(); i++) {
		double a = cv::contourArea(contours[i], false);
		if (a > largest_area) {
			largest_area = a;
			largest_contour_index = i;
		}
	}

	bounding_rect = cv::boundingRect(contours[largest_contour_index]);
	return bounding_rect;
}

