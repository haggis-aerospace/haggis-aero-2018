#include "camLib.h"
#include "camLib.h"
#include <regex>

using namespace std;
using namespace cv;
using namespace cv::text;

camLib::camLib(bool disp)
{
    display = disp;
    cap = VideoCapture(1);  //Creating capture instance and initilizing OCRTesseract

    cap.set(CV_CAP_PROP_BUFFERSIZE, 2);
    tess = OCRTesseract::create(NULL, NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 3, 10);
    
    if(display){namedWindow("Output", WINDOW_NORMAL);}
                //resizeWindow("Output", 1600, 980);}
}

camLib::~camLib()
{
    delete tess;
}



void flush(VideoCapture& camera)
{
	for(int i=0; i<5; i++)
	{
	    camera.grab();
	}
}

Rect enlargeROI(Mat frm, Rect boundingBox, int padding) {
    Rect returnRect = Rect(boundingBox.x - padding, boundingBox.y - padding, boundingBox.width + (padding * 2), boundingBox.height + (padding * 2));
    if (returnRect.x < 0)returnRect.x = 0;
    if (returnRect.y < 0)returnRect.y = 0;
    if (returnRect.x+returnRect.width >= frm.cols)returnRect.width = frm.cols-returnRect.x;
    if (returnRect.y+returnRect.height >= frm.rows)returnRect.height = frm.rows-returnRect.y;
    return returnRect;
}


//Retrieving a new image from default webcam
Mat camLib::getImg()
{
//    flush(cap);
    cap.read(lastImg);
//    cv::resize(lastImg, lastImg, cv::Size(CAP_WIDTH, CAP_HEIGHT));
    waitKey(1);
    return lastImg;
}


vector<pair<cv::Mat, cv::Point>> camLib::getBounds(Mat* img){ return getBounds(*img); }

//Returns array of Mats containing possible letters 
std::mutex output_mutex;
vector<pair<cv::Mat, cv::Point>> camLib::getBounds(Mat img)
{
    Mat hsv;    //Conversion to HSV colour space
    cvtColor(img, hsv, CV_BGR2HSV);
//    GaussianBlur(hsv, hsv, Size(15,15), 0, 0);  //Smooth image with blur
        
    inRange(hsv, WHITE_MIN, WHITE_MAX, hsv);  //Filter out all colours except white/light grey

    std::vector<Mat> channels;
    split(hsv, channels);   //Splitting HSV space into 3 channels, using Hue for recognition

    Mat origHue = channels[0];
    Mat Hue;
    origHue.copyTo(Hue);

    Canny(Hue, Hue, 180, 0);    //Get lines, thicken x4 for better detection    
    for(int i=0; i<4; ++i) 
        dilate(Hue, Hue, Mat());

    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  //Retrieving image contours, used to determine ROI's
    findContours(Hue,contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);

    vector<pair<cv::Mat, cv::Point>> output;
    vector<thread> threads;
    int split = floor(contoursH.size()/4);
    if(contoursH.size() > 4){
        for(int t =0; t<4; t++){
            threads.push_back(thread(&camLib::checkRegion, this, 
                origHue, contoursH, t, hierarchyH, std::ref(output)));
        }
    }else{ threads.push_back(thread(&camLib::checkRegion, this,
                origHue, contoursH, 0, hierarchyH, std::ref(output)));}

    for(int i=0; i<threads.size(); i++)
        threads.at(i).join();

    lastFrameScan = ::currentTime();
    return output;
}



//Checks a region and determines if it could contain a letter.
bool camLib::checkRegion(cv::Mat input, std::vector<std::vector<cv::Point>> contours, int index, std::vector<cv::Vec4i> hierarchy, std::vector<std::pair<cv::Mat,cv::Point>> &output)
{
    int split = floor(contours.size()/4);
    int start = split*index;
    int end = split*(index+1);
    if(index == 3)
        end = contours.size();
    if(contours.size() <= 4)
    {
        start = 0;
        end = contours.size();
    }
    unsigned int operations = 0;
    for( int i = start; i< end; i++ ){
        operations++;
        std::vector<cv::Point> contour = contours.at(i); 
        if(contourArea(contour) < CONTOUR_SIZE_MIN) continue; //Filter small contours
        if(hierarchy[i][3] < 0) continue;  //Hollow out thick contours

        cv::Rect boundRect = boundingRect(contour);    
        float ratio = (float)boundRect.width/(float)boundRect.height;
        if(ratio < 1.0-RATIO_THRESHOLD || ratio > 1.0+RATIO_THRESHOLD) continue;    //Filter rects too wide or tall

        Mat cropped = input(boundRect); //Crop to rect

        std::vector<std::vector<cv::Point>> contoursOut; //Retrieve contour closest to centre, crop to it
        std::vector<cv::Vec4i> hierarchyOut;
        findContours(cropped,contoursOut, hierarchyOut, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
        if(contoursOut.size() <= 0) continue;
        vector<Rect> croppedRects;
        for(int y=0;y<contoursOut.size();y++) croppedRects.push_back(boundingRect(contoursOut.at(y)));
        double min=10000;
        Rect target = croppedRects.at(0);
        Point centre(boundRect.width/2, boundRect.height/2 + (boundRect.height*0.35));
        for(int y=0;y<croppedRects.size();y++)
        {
            double dist = cv::norm(Point(croppedRects.at(y).x + croppedRects.at(y).width/2
                ,croppedRects.at(y).y + croppedRects.at(y).height/2) - centre);
            if(dist < 0) dist = dist*-1;
            if(dist < min){ min = dist; target = croppedRects.at(y); }
        }
        //int offset = (double)target.width * .10;
        target = enlargeROI(cropped, target, 10);
        cropped = cropped(target);

        if(cropped.cols < LETTER_MIN_WIDTH || cropped.rows < LETTER_MIN_HEIGHT)
            continue;

        centre = Point(boundRect.x + boundRect.width/2, boundRect.y + boundRect.height/2);
        output_mutex.lock();
            output.push_back(make_pair(cropped, centre));
        output_mutex.unlock();
        continue;
    }
}


//Runs OCRTesseract on input image
vector<Letter> camLib::findLetter(Mat img, int min_confidence)
{
    vector<Letter> let;
   // if(!tryTrack(img)){
    if(true){
        //cout << "N\n";
        vector< pair<cv::Mat, cv::Point>> regions = getBounds(img);  //Retrieve vector of Mats possibly containing letters
        Mat scan;
        bool lastSet;
        pair<Mat, Point> lastImg;
        for(int i=0; i<regions.size(); i++) //Iterate through all retrieved Mats
        {
            if(regions.at(i).first.cols > 0){

		if(lastSet){
                        int offsetX = CAP_WIDTH * 0.95;
			int offsetY = CAP_HEIGHT * 0.95;
			if((regions.at(i).second.x-offsetX < lastImg.second.x && 
				regions.at(i).second.x+offsetX > lastImg.second.x) && 
				regions.at(i).second.y-offsetY < lastImg.second.y && 
                                regions.at(i).second.y+offsetY > lastImg.second.y)
			continue;
		}
                std::string res = "";
                regions.at(i).first.copyTo(scan);    //Apply white border area around Mat
                copyMakeBorder(scan, scan, 50,50,50,50,BORDER_CONSTANT,Scalar(0,0,0));
                scan = Scalar::all(255) - scan;     //Inverse colours to make letter black
                
                vector<Rect> *rects = new vector<Rect>();
                vector<std::string> *texts = new vector<std::string>();
                vector<float> *confidence = new vector<float>();
                tess->run(scan, res, rects, texts, confidence); //Runs Tesseract OCR on Mat
                
                //Ensure letter was found and filter low confidence
                if(res.length() == 3){ res = res.substr(0, 1); }
                if(!res.empty() && confidence->at(0) >= min_confidence) 
                {
                    let.push_back(Letter());    //Add letter to vector of type Letter
                    int pos = ((float)regions.at(i).second.x/(float)img.cols)*100;
                    let.at(let.size()-1).x = regions.at(i).second.x;
                    let.at(let.size()-1).y = regions.at(i).second.y;
                    let.at(let.size()-1).letter = char(res.at(0));
                    let.at(let.size()-1).pos = pos;
                    let.at(let.size()-1).width = regions.at(i).first.cols;
                    let.at(let.size()-1).height = regions.at(i).first.rows;
                    
                    lastImg = regions.at(i);
                    lastSet = true;
                    if(display)
                    rectangle(img, Rect(regions[i].second.x - regions[i].first.cols / 2,
                        regions[i].second.y - regions[i].first.rows / 2, regions[i].first.cols,
                        regions[i].first.rows), Scalar(0, 0, 255), 4);
                }
                delete rects;   //Free up memory pointers
                delete texts;
                delete confidence;
            }
        }

        updateHistory(let);
    }else{
        //cout << "Y\n";
        let.push_back(trackedLetter); }
    
    if(display){ imshow("Output", img);
    cv::waitKey(1);}
    return let;
}


//Adds letters to the history
void camLib::updateHistory(vector<Letter> letters)
{
    vector<pair<Letter,unsigned long>> pairs;
    for(int i=0; i<letters.size(); i++)
        pairs.push_back(make_pair(letters.at(i), currentTime()));
    history.push_front(pairs);
    if(history.size() > HISTORY_SIZE)
        history.pop_back();
}


//Returns most occouring letter
Letter camLib::mostOccouring()
{
    if(tracking) return trackedLetter;
    if(history.size() == 0)
        return Letter();
    
    map<char, vector<Letter>> elements;
    for(int d=0; d<history.size(); d++)
        for(int v=0; v<history.at(d).size(); v++)
        {
            if(currentTime()-history.at(d).at(v).second>MAX_HISTORY_AGE){
                continue;}
            char ltr = history.at(d).at(v).first.letter;
            map<char, vector<Letter>>::iterator it = elements.find(ltr);
            if(it != elements.end()){
                it->second.push_back(history.at(d).at(v).first);
            }else{
                vector<Letter> newVctr;
                newVctr.push_back(history.at(d).at(v).first);
                elements.insert(make_pair(ltr, newVctr));
            }
        }
    pair<Letter,int> max = make_pair(Letter(),0);
    for(std::map<char,vector<Letter>>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
    {
        vector<Letter> letters =  iter->second;
        if(letters.size() > max.second){
            max = make_pair(letters.at(0),letters.size());
            int avgSize = 0;
            for(int s=0; s<letters.size();s++)
                avgSize += letters.at(s).height;
            avgSize = avgSize/letters.size();
            max.first.avSize = avgSize;
        }
    }
    if((double)max.second/(double)history.size()*100.0>=CERTAINTY_PERCENTAGE){
        return max.first;}
    return Letter();
}



bool camLib::tryTrack(cv::Mat &img)
{
/*    if(history.size() < HISTORY_SIZE){
        return false;}
    
    if(currentTime()-lastFrameScan > MAX_SCAN_GAP){
        tracking = false;
        //history.clear();
        cout << "Detecting Again\n";
        return false;
    }

    bool initTrack = false;
    if(!isTracking())
    {
        map<char, vector<Letter>> elements;
        for(int d=0; d<history.size(); d++)
            for(int v=0; v<history.at(d).size(); v++)
            {
                if(currentTime()-history.at(d).at(v).second>MAX_HISTORY_AGE){
                    cout << "Letter too old\n"; continue;}
                char ltr = history.at(d).at(v).first.letter;
                map<char, vector<Letter>>::iterator it = elements.find(ltr);
                if(it != elements.end()){
                    it->second.push_back(history.at(d).at(v).first);
                }else{
                    vector<Letter> newVctr;
                    newVctr.push_back(history.at(d).at(v).first);
                    elements.insert(make_pair(ltr, newVctr));
                }
            }
        
        pair<Letter,int> max = make_pair(Letter(),0);
        for(std::map<char,vector<Letter>>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
        {
            vector<Letter> letters =  iter->second;
            if(letters.size() > max.second)
                max = make_pair(letters.at(0),letters.size());
        }
        if((double)max.second/(double)history.size()*100.0<CERTAINTY_PERCENTAGE)
        {
            tracking = false;
            return false;
        }
        
        trackedLetter = max.first;
        tracking = true;
        initTrack = true;
    }
    
    if(tracking)
    {   
        Rect2d bbox(0,0,CAP_WIDTH,CAP_HEIGHT);
        if(initTrack)
        {
            bbox = Rect2d(trackedLetter.x - trackedLetter.width/2 - (int)((double)trackedLetter.width*.25), 
                trackedLetter.y - trackedLetter.height/2 - (int)((double)trackedLetter.height*.25), 
                trackedLetter.width + (int)((double)trackedLetter.width*.50),
                trackedLetter.height + (int)((double)trackedLetter.height*.50));
            tracker = Tracker::create("KCF");
            tracker->init(img, bbox);
            if(display) rectangle(img, bbox, Scalar(0,255,0),4);
            cout << "Now tracking letter: " << trackedLetter.letter << "\n";
            return true;
        }
        bool success = tracker->update(img,bbox);
        if(success)
        {
            if(display)rectangle(img, bbox, Scalar(0,255,0),4);
            trackedLetter.x = bbox.x + bbox.width/2 + (int)((double)trackedLetter.width*.25);
            trackedLetter.y = bbox.y + bbox.height/2 + (int)((double)trackedLetter.height*.25);
            trackedLetter.width = (int)((double)bbox.width*.50);
            trackedLetter.height = (int)((double)bbox.height*.50);
            trackedLetter.pos = (int)((double)trackedLetter.x/(double)img.cols*100.0);
            return true;
        }
        tracking = false;
        history.clear();
        cout << "Lost lock on character\n";
        return false;
    }
    */
    return false;
}


//Python Bindings
extern "C"{
    camLib* py_camLib(bool disp) { return new camLib(disp); }
    Mat py_getImg(camLib* lib){ return lib->getImg(); }
    void py_findLetter(camLib* lib, int min_confidence){ lib->findLetter(min_confidence); }
    Letter py_mostOccouring(camLib* lib){ return lib->mostOccouring(); }
}
