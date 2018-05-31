#include "letter_detection.h"


using namespace std;
using namespace cv;

letter_detection::letter_detection()
{
    if (api.Init(NULL, "eng"))
    {
            std::cout << "Error Initilizing Tesseract" << std::endl;
            exit(1);
    }
    api.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
    api.SetVariable("tessedit_char_whitelist","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
}

letter_detection::~letter_detection()
{
}

Mat letter_detection::prepareRegion(Mat src)
{
    //Retrieving image contours, used to determine ROI's
    std::vector<std::vector<cv::Point>> contoursH;
    std::vector<cv::Vec4i> hierarchyH;  
    
    findContours(src, contoursH, hierarchyH, CV_RETR_TREE , CV_CHAIN_APPROX_SIMPLE);
    if(contoursH.size() <= 0) 
        return src;
    
    //Find Max
    int maxIndex = 0;
    for(int i=0; i<contoursH.size(); i++)
        if(contoursH.at(i).size() >= contoursH.at(maxIndex).size())
            maxIndex = i;
    Rect region = boundingRect(contoursH.at(maxIndex));
    
    src(region);
    
    return src;
}

Letter letter_detection::findLetter(cv::Mat src, int *accuracy)
{
    Letter letter;
    
    if(!src.data){ std::cout << "letter_detection: Invalid Input Data" << std::endl;
        return letter; }
    
    cv::Mat inputImg = prepareRegion(src);
    
    
    
    float maxPercent = 0;
    char maxChar = '~';
    int maxIndex = 0;
    for(int i=0; i<4; i++)
    {
            tessMatToPix(inputImg);
            api.Recognize(0);
            tesseract::ResultIterator *ri = api.GetIterator();
            tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
            if(ri != 0)
            {
                const char *tmpLetter = ri->GetUTF8Text(level);
                float conf = ri->Confidence(level);
                if(conf > maxPercent){ maxPercent = conf; maxIndex = i;  maxChar = tmpLetter[0]; }
                delete[] tmpLetter;
            }
            cv::rotate(inputImg, inputImg, cv::ROTATE_90_CLOCKWISE);
    }
    
    for(int i=0; i<maxIndex; i++)
        cv::rotate(inputImg, inputImg, cv::ROTATE_90_CLOCKWISE);
    
    letter.letter = maxChar;
    
    std::cout << "Letter: " << maxChar << endl;
    cv::imshow("Letter Detection", inputImg);
    cv::waitKey(1);
    
    return letter;
}

void letter_detection::tessMatToPix(Mat src)
{
        api.SetImage((uchar*)src.data, src.size().width, src.size().height, src.channels(), src.step1());
}