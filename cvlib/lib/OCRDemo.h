//
// Created by David Helekal on 22/10/2017.
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <opencv2/text.hpp>

using namespace cv;
using namespace cv::text;
using namespace std;

#ifndef CVLIB_OCRDEMO_H
#define CVLIB_OCRDEMO_H
//ERStat extraction is done in parallel for different channels
class Parallel_extractCSER: public cv::ParallelLoopBody
{
private:
    std::vector<Mat> &channels;
    std::vector< vector<cv::text::ERStat> > &regions;
    std::vector< Ptr<ERFilter> > er_filter1;
    std::vector< Ptr<ERFilter> > er_filter2;

public:
    Parallel_extractCSER(vector<Mat> &_channels, vector< vector<ERStat> > &_regions,
                         vector<Ptr<ERFilter> >_er_filter1, vector<Ptr<ERFilter> >_er_filter2)
            : channels(_channels),regions(_regions),er_filter1(_er_filter1),er_filter2(_er_filter2){}

    virtual void operator()( const cv::Range &r ) const
    {
        for (int c=r.start; c < r.end; c++)
        {
            er_filter1[c]->run(channels[c], regions[c]);
            er_filter2[c]->run(channels[c], regions[c]);
        }
    }
    Parallel_extractCSER & operator=(const Parallel_extractCSER &a);
};

//OCR recognition is done in parallel for different detections
template <class T>
class Parallel_OCR: public ParallelLoopBody
{
private:
    vector<Mat> &detections;
    vector<string> &outputs;
    vector< vector<Rect> > &boxes;
    vector< vector<string> > &words;
    vector< vector<float> > &confidences;
    vector< Ptr<T> > &ocrs;

public:
    Parallel_OCR(vector<Mat> &_detections, vector<string> &_outputs, vector< vector<Rect> > &_boxes,
                 vector< vector<string> > &_words, vector< vector<float> > &_confidences,
                 vector< Ptr<T> > &_ocrs)
            : detections(_detections), outputs(_outputs), boxes(_boxes), words(_words),
              confidences(_confidences), ocrs(_ocrs)
    {}

    virtual void operator()( const cv::Range &r ) const
    {
        for (int c=r.start; c < r.end; c++)
        {
            ocrs[c%ocrs.size()]->run(detections[c], outputs[c], &boxes[c], &words[c], &confidences[c], OCR_LEVEL_WORD);
        }
    }
    Parallel_OCR & operator=(const Parallel_OCR &a);
};


// detect wrongly recognised strings
bool isRepetitive(const string& s){
    int count  = 0;
    int count2 = 0;
    int count3 = 0;
    int first=(int)s[0];
    int last=(int)s[(int)s.size()-1];
    for (int i=0; i<(int)s.size(); i++){
        if ((s[i] == 'i') || (s[i] == 'l') || (s[i] == 'I'))
            count++;
        if((int)s[i]==first)
            count2++;
        if((int)s[i]==last)
            count3++;
    }
    if ((count > ((int)s.size()+1)/2) || (count2 == (int)s.size()) || (count3 > ((int)s.size()*2)/3)) {
        return true;
    }


    return false;
}

//Draw ER's in an image via floodFill
void er_draw(vector<Mat> &channels, vector<vector<ERStat> > &regions, vector<Vec2i> group, Mat& segmentation){
    for (int r=0; r<(int)group.size(); r++){
        ERStat er = regions[group[r][0]][group[r][1]];
        if (er.parent != NULL) {
            int newMaskVal = 255;
            int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
            floodFill(channels[group[r][0]],segmentation,Point(er.pixel%channels[group[r][0]].cols,er.pixel/channels[group[r][0]].cols),
                      Scalar(255),0,Scalar(er.level),Scalar(0),flags);
        }
    }
}

void imgRecTest() {

    namedWindow("recognition",WINDOW_NORMAL);
    Mat frame,grey,orig_grey,out_img;
    vector<Mat> channels;
    vector< vector<ERStat> > regions(2);

    // create ERFilter objects with default classifiers
    vector< Ptr<ERFilter> > er_filters1;
    vector< Ptr<ERFilter> > er_filters2;
    for (int i = 0; i<2; i++) {
        Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1("trained_classifierNM1.xml"),8,0.00015f,0.13f,0.2f,true,0.1f);
        Ptr<ERFilter> er_filter2 = createERFilterNM2(loadClassifierNM2("trained_classifierNM2.xml"),0.5);
        er_filters1.push_back(er_filter1);
        er_filters2.push_back(er_filter2);
    }

    // initialize OCR tesseract objects
    int num_ocrs = 10;
    vector< Ptr<OCRTesseract> > ocrs;
    for (int i = 0; i < num_ocrs; i++) {
        ocrs.push_back(OCRTesseract::create());
    }

    // get image from webcam
    int cam_index = 0;
    VideoCapture cam(cam_index);

    while (waitKey(30) != 27){

        cam.read(frame);

        // text detection
        cvtColor(frame,grey,COLOR_RGB2GRAY);
        grey.copyTo(orig_grey);

        channels.clear();
        channels.push_back(grey);
        channels.push_back(255-grey);

        regions[0].clear();
        regions[1].clear();

        parallel_for_(cv::Range(0,(int)channels.size()), Parallel_extractCSER(channels,regions,er_filters1,er_filters2));

        // group detection

        vector< vector<Vec2i> > nm_region_groups;
        vector<Rect> nm_boxes;

        erGrouping(frame, channels, regions, nm_region_groups, nm_boxes, ERGROUPING_ORIENTATION_HORIZ);

        // text recognition

        frame.copyTo(out_img);
        float scale_img  = (float)(600.f/frame.rows);
        float scale_font = (float)(2-scale_img)/1.4f;
        vector<string> words_detection;
        float min_confidence1 = 60.f, min_confidence2 = 70.f;
        vector<Mat> detections;


        for (int i=0; i<(int)nm_boxes.size(); i++) {
            rectangle(out_img, nm_boxes[i].tl(), nm_boxes[i].br(), Scalar(255,255,0),3);


            Mat group_img = Mat::zeros(frame.rows+2, frame.cols+2, CV_8UC1);
            er_draw(channels, regions, nm_region_groups[i], group_img);
            group_img(nm_boxes[i]).copyTo(group_img);
            copyMakeBorder(group_img,group_img,15,15,15,15,BORDER_CONSTANT,Scalar(0));
            detections.push_back(group_img);
        }

        vector<string> outputs((int)detections.size());
        vector< vector<Rect> > boxes((int)detections.size());
        vector< vector<string> > words((int)detections.size());
        vector< vector<float> > confidences((int)detections.size());

        for (int i=0; i<(int)detections.size(); i=i+(int)num_ocrs){
            Range r;
            if (i+(int)num_ocrs <= (int)detections.size())
                r = Range(i,i+(int)num_ocrs);
            else
                r = Range(i,(int)detections.size());

            parallel_for_(r, Parallel_OCR<OCRTesseract>(detections, outputs, boxes, words, confidences, ocrs));

        }



        for (int i=0; i<(int)detections.size(); i++){

            outputs[i].erase(remove(outputs[i].begin(), outputs[i].end(), '\n'), outputs[i].end());

            if (outputs[i].size() < 3) {
                continue;
            }

            for (int j=0; j<(int)boxes[i].size(); j++){

                boxes[i][j].x += nm_boxes[i].x-15;
                boxes[i][j].y += nm_boxes[i].y-15;


                if ((words[i][j].size() < 2) || (confidences[i][j] < min_confidence1) || ((words[i][j].size()==2) && (words[i][j][0] == words[i][j][1])) || ((words[i][j].size()< 4) && (confidences[i][j] < min_confidence2)) || isRepetitive(words[i][j])) {
                    continue;
                }


                words_detection.push_back(words[i][j]);
                rectangle(out_img, boxes[i][j].tl(), boxes[i][j].br(), Scalar(255,0,255),3);
                Size word_size = getTextSize(words[i][j], FONT_HERSHEY_SIMPLEX, (double)scale_font, (int)(3*scale_font), NULL);
                rectangle(out_img, boxes[i][j].tl()-Point(3,word_size.height+3), boxes[i][j].tl()+Point(word_size.width,0), Scalar(255,0,255),-1);
                putText(out_img, words[i][j], boxes[i][j].tl()-Point(1,1), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255,255,255),(int)(3*scale_font));

                if ((boxes[i][j].tl().x + boxes[i][j].br().x)/2 > (0.5 * out_img.cols))
                    cout << words[i][j] << ":right->" << endl;
                else
                    cout << words[i][j] << ":left<-" << endl;
            }

        }
        imshow("recognition",out_img);
    }
}



#endif //CVLIB_OCRDEMO_H
