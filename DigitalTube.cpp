#include<opencv2/opencv.hpp>
#include<algorithm>
#include<iostream>
using namespace std;
using namespace cv;

#define W_BLUR "Blur"
#define W_BGR "BGR"
#define W_ROI "ROI"
#define W_GREEN "Green"
#define W_GREENTHRESHOLD "GreenThreshold"
#define W_MORPHOLOGY "Morphology"
#define W_CONTOURS "Contours"
#define W_NUMBER1 "Number1"
#define W_NUMBER2 "Number2"
#define W_NUMBER3 "Number3"


Mat g_srcImage;
Mat g_srcImageBlur;
Mat g_srcImageROI;
Mat g_dstImageGreen;
Mat g_dstImageGreenThreshold;
Mat g_dstImageMorphology;
Mat g_dstImageNumber[3];


int g_Blur=5;
int g_ROIOpen=5;
int g_dstImageGreenThresholdValue=200;
int g_dstImageOpenValue=1;
int g_dstImageCloseValue=5;
int g_timeInterval=10;

char g_number[4]={'z','e','r'};

bool g_ROITest=false;

vector<Rect> g_rect;

bool areacomp(const Rect &a, const Rect &b){
    return a.area()> b.area();
}

void myROI()
{
    Mat dstImageROI=Mat::zeros(g_srcImageBlur.size(),CV_8UC1);


    for(int i=0;i<dstImageROI.rows;i++)
    {
        for(int j=0;j<dstImageROI.cols;j++)
        {
            if(g_srcImageBlur.at<Vec3b>(i,j)[0]<100 && g_srcImageBlur.at<Vec3b>(i,j)[1]<100 && g_srcImageBlur.at<Vec3b>(i,j)[2]>150)
            {
                dstImageROI.at<uchar>(i,j)=255;
            }
        }
    }

    Mat kernel=getStructuringElement(MORPH_RECT,Size(g_ROIOpen,g_ROIOpen));
    morphologyEx(dstImageROI,dstImageROI,MORPH_OPEN,kernel);
    namedWindow(W_BGR,WINDOW_NORMAL);
    imshow(W_BGR,dstImageROI);

    vector<vector<Point> > contours;
    vector<Rect> rect;

    findContours(dstImageROI,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageROIContours=g_srcImage.clone();

    namedWindow(W_ROI,WINDOW_NORMAL);

    if(contours.size()>=1)
    {
        //画出可包围数字的最小矩形
        Rect t_rect = boundingRect(dstImageROI);
        rect.push_back(t_rect);
           
        g_ROITest=true;
    }
    //开始没有对准数码管自然没有轮廓
    else
    {
        g_ROITest=false;
        return;
    }
    
    

    rectangle(dstImageROIContours,rect[0],Scalar(193,0,0),10);

    imshow(W_ROI,dstImageROIContours);

    g_srcImageROI=g_srcImageBlur(Rect(rect[0]));

    // cout<<"MyROI:OK"<<endl;
}

void mySplit()
{
    vector<Mat> channels;
    split(g_srcImageROI,channels);
    g_dstImageGreen=channels.at(1);

    // namedWindow(W_GREEN,WINDOW_NORMAL);
    // imshow(W_GREEN,g_dstImageGreen);

    // cout<<"MySplit:OK"<<endl;
}

void myThreshold()
{
    //deep copy
    g_dstImageGreen.copyTo(g_dstImageGreenThreshold);
    
    //Threshold
    g_dstImageGreenThreshold=g_dstImageGreenThreshold>g_dstImageGreenThresholdValue;

    // namedWindow(W_GREENTHRESHOLD,WINDOW_NORMAL);
    // imshow(W_GREENTHRESHOLD,g_dstImageGreenThreshold);

    // cout<<"MyThreshold:OK"<<endl;

}

void myMorphology()
{
    Mat midImage;
	Mat kernelOpen=getStructuringElement(MORPH_RECT,Size(g_dstImageOpenValue,g_dstImageOpenValue));
	morphologyEx(g_dstImageGreenThreshold,midImage,MORPH_OPEN,kernelOpen);


	Mat kernelClose=getStructuringElement(MORPH_RECT,Size(g_dstImageCloseValue,g_dstImageCloseValue));
	morphologyEx(midImage,g_dstImageMorphology,MORPH_CLOSE,kernelClose);

    namedWindow(W_MORPHOLOGY,WINDOW_NORMAL);
	imshow(W_MORPHOLOGY,g_dstImageMorphology);   
    
    // cout<<"MyMorphology:OK"<<endl;
}


void myContours()
{
    vector<vector<Point> > contours;
    vector<Rect> t_rect;

    findContours(g_dstImageMorphology,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageContours=g_srcImageROI.clone();

	vector<vector<Point> >::iterator It;

    namedWindow(W_CONTOURS,WINDOW_NORMAL);

    if(contours.size()>=9)
    {
        int s=0;
        //只能预防最上层的数字出现断层，不能预防最下层的数字出现断层
        for(It = contours.begin()+3;s<3;It++,s++)
	    {
            //画出可包围数字的最小矩形
            Rect rect = boundingRect(*It);
            rectangle(dstImageContours,rect,Scalar(193,0,0),10);
            t_rect.push_back(rect);
	    }
        imshow(W_CONTOURS,dstImageContours);
    }

    if(t_rect.size()==3)
    {
        g_rect.assign(t_rect.begin(),t_rect.end());
    }

    // cout<<"MyContours:OK"<<endl;
}

bool comp(const Rect &a, const Rect &b){
    return a.x< b.x;
}

void myNumberSort()
{
    sort(g_rect.begin(),g_rect.end(),comp);
    

    for(int i=0;i<3;i++)
    {
        Mat ROI=g_dstImageMorphology(Rect(g_rect[i]));
        g_dstImageNumber[i]=ROI.clone();
    }

    namedWindow(W_NUMBER1,WINDOW_NORMAL);
    imshow(W_NUMBER1,g_dstImageNumber[0]);
    namedWindow(W_NUMBER2,WINDOW_NORMAL);
    imshow(W_NUMBER2,g_dstImageNumber[1]);
    namedWindow(W_NUMBER3,WINDOW_NORMAL);
    imshow(W_NUMBER3,g_dstImageNumber[2]);
    
    // cout<<"MyNumberSort:OK"<<endl;
}


char myDiscern(Mat n)
{
    if(3*n.cols<n.rows)
    {
        return '1';
    }
    int x_half=n.cols/2;
    int y_one_third=n.rows/3;
    int y_two_third=n.rows*2/3;
    int a=0,b=0,c=0,d=0,e=0,f=0,g=0;

    for(int i=0;i<n.rows;i++)
    {
        uchar *data=n.ptr<uchar>(i);
        if(i<y_one_third)
        {
            if(data[x_half]==255) a=1;
        }
        else if(i>y_one_third&&i<y_two_third)
        {
            if(data[x_half]==255) g=1;
        }
        else
        {
            if(data[x_half]==255) d=1;
        }
    }
    for(int j=0;j<n.cols;j++)
    {
        uchar *data=n.ptr<uchar>(y_one_third);
        if(j<x_half)
        {
            if(data[j]==255) f=1;
        }
        else
        {
            if(data[j]==255) b=1;
        }
    }
    for(int j=0;j<n.cols;j++)
    {
        uchar *data=n.ptr<uchar>(y_two_third);
        if(j<x_half)
        {
            if(data[j]==255) e=1;
        }
        else
        {
            if(data[j]==255) c=1;
        }
    }

    if(a==1 && b==1 && c==1 && d==1 && e==1 && f==1 && g==0)
    {
        return '0';
    }
    else if(a==1 && b==1 && c==0 && d==1 && e==1 && f==0 && g==1)
    {
        return '2';
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==0 && g==1)
    {
        return '3';
    }
    else if(a==0 && b==1 && c==1 && d==0 && e==0 && f==1 && g==1)
    {
        return '4';
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        return '5';
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        return '6';
    }
    else if(a==1 && b==1 && c==1 && d==0 && e==0 && f==0 && g==0)
    {
        return '7';
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        return '8';
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        return '9';
    }
    else
    {
        //printf("[error_%d_%d_%d_%d_%d_%d_%d]\n",a,b,c,d,e,f,g);
        return 'e';
    }

    // cout<<"MyDiscern:OK"<<endl;
}


int main()
{
    VideoCapture capture("C101.webm");

    double time0=static_cast<double>(getTickCount());
    bool timeTrigger=false;

    int numbertimes[10]={0,0,0,0,0,0,0,0,0,0};

    while(1)
    {

        capture>>g_srcImage;
        namedWindow("[Video]",WINDOW_NORMAL);
        imshow("[Video]",g_srcImage);
        

        g_srcImageBlur=g_srcImage.clone();
        GaussianBlur(g_srcImage,g_srcImageBlur,Size(g_Blur,g_Blur),0.0);
        // namedWindow(W_BLUR,WINDOW_NORMAL);
        // imshow(W_BLUR,g_srcImageBlur);

        //ROI
        myROI();

        if(g_ROITest==false)
        {
            //视频循环不能没有间隔，必须有waitKey
            waitKey(1);
            continue;
        }
        
        //split channels
        mySplit();

        //threshold=160
        myThreshold();

        //clear small white and connection breakpoint
        myMorphology();

        //draw contours
        myContours();

        


        int time=int((getTickCount()-time0)/getTickFrequency());

        //如果轮廓识别到三个数字
        if(g_rect.size()==3)
        {
            bool contoursTest=true;
            //如果ROI的数字超过了g_dstImageMorphology的范围
            for(int i=0;i<3;i++)
            {
                if((g_rect[i].x+g_rect[i].width)>g_dstImageMorphology.cols||(g_rect[i].y+g_rect[i].height)>g_dstImageMorphology.rows)
                {
                    contoursTest=false;
                }
            }


            if(!contoursTest) continue;


            //sort numbers
            myNumberSort();

            //discern number
            char t_number[3];

            //如果有一个数字识别不出来，那么就t_number[0]标记为'e'
            for(int i=0;i<3;i++)
            {
                t_number[i]=myDiscern(g_dstImageNumber[i]);

                if(t_number[i]=='e')
                {
                    t_number[0]='e';
                    break;
                }
            }

            //第一次的时候，将识别出的t_number赋给g_number
            if(g_number[0]=='z' && t_number[0]!='e')
            {
                for(int i=0;i<3;i++)
                {
                    g_number[i]=t_number[i];
                    cout<<g_number[i];
                }
                cout<<endl;
                
                //将g_timeInterval秒内的数字频率统计
                for(int i=0;i<3;i++)
                {
                    if(g_number[i]=='0')
                    {
                        numbertimes[0]++;
                    }
                    else if(g_number[i]=='1')
                    {
                        numbertimes[1]++;
                    }else if(g_number[i]=='2')
                    {
                        numbertimes[2]++;
                    }else if(g_number[i]=='3')
                    {
                        numbertimes[3]++;
                    }else if(g_number[i]=='4')
                    {
                        numbertimes[4]++;
                    }else if(g_number[i]=='5')
                    {
                        numbertimes[5]++;
                    }else if(g_number[i]=='6')
                    {
                        numbertimes[6]++;
                    }else if(g_number[i]=='7')
                    {
                        numbertimes[7]++;
                    }else if(g_number[i]=='8')
                    {
                        numbertimes[8]++;
                    }else if(g_number[i]=='9')
                    {
                        numbertimes[9]++;
                    }
                }
            }
            //g_number存在时，将非错误的且不同于上次的结果赋给并输出
            else if(g_number[0]!='z' &&t_number[0]!='e'&& (t_number[0]!=g_number[0] || t_number[1]!=g_number[1] || t_number[2]!=g_number[2]))
            {
                for(int i=0;i<3;i++)
                {
                    g_number[i]=t_number[i];
                    cout<<g_number[i];
                }
                cout<<endl;
                //将g_timeInterval秒内的数字频率统计
                for(int i=0;i<3;i++)
                {
                    if(g_number[i]=='0')
                    {
                        numbertimes[0]++;
                    }
                    else if(g_number[i]=='1')
                    {
                        numbertimes[1]++;
                    }else if(g_number[i]=='2')
                    {
                        numbertimes[2]++;
                    }else if(g_number[i]=='3')
                    {
                        numbertimes[3]++;
                    }else if(g_number[i]=='4')
                    {
                        numbertimes[4]++;
                    }else if(g_number[i]=='5')
                    {
                        numbertimes[5]++;
                    }else if(g_number[i]=='6')
                    {
                        numbertimes[6]++;
                    }else if(g_number[i]=='7')
                    {
                        numbertimes[7]++;
                    }else if(g_number[i]=='8')
                    {
                        numbertimes[8]++;
                    }else if(g_number[i]=='9')
                    {
                        numbertimes[9]++;
                    }
                }
            }
            //其他情况：第一次t_number没识别；g_number存在时，t_number没识别出和t_number和上次一样
            else;
        }


        //time是g_timeInterval的倍数（非0），且触发器没有被触发
        if(time!=0 && time%g_timeInterval==0 && timeTrigger==false) 
        {
            timeTrigger=true;
            
            //出现频率最少的三个数
            vector<int> result;

            //让等于0次的数输入到result中，让等于1次...直到result中统计到三个数
            for(int i=0;result.size()<3;i++)
            {
                for(int j=0;j<10;j++)
                {
                    if(numbertimes[j]==i)
                    {
                        result.push_back(j);
                    }
                    if(result.size()==3) break;
                }
            }
            cout<<"[result]="<<result[0]<<result[1]<<result[2]<<endl;
        }
        else if(time%g_timeInterval!=0 && timeTrigger==true)
        {
            timeTrigger=false;
            for(int i=0;i<10;i++)
            {
                numbertimes[i]=0;
            }
        }


        // if(waitKey(50)=='q') return 0;

        if(waitKey()=='g');
        else if(waitKey()=='q')return 0;
         
    }
	return 0;
}
