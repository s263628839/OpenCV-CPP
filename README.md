# 一、感想
## 1.图像预处理迭代
版本1：考虑获得给数字轮廓识别的图像时，是随便挑选阈值的，所以图像就很糟糕。
版本2：通过直接放大图像观察具体的RGB数值分布，就能得到一个特定的颜色通道和阈值。
## 2.图像形态学运算迭代
版本1：随便选择一种形态学运算，得到的效果并不是很好，要么有干扰白点，要么就是数字之间粘连，要么数码管之间的缝隙还存在
版本2：先开运算去除白点，再闭运算进行消除数码管之间的缝隙（为了整体轮廓识别）

## 3.图像轮廓识别迭代
> 版本1：

利用九个数字轮廓识别是从下层一层层往上的特性，识别中间三个
```cpp
vector<vector<Point> >::iterator lt;
for(lt=g_contours.begin()+3;lt<g_contours.end()-3;lt++)
```

>版本2：

发现如果a、f、g亮的这样的数码管之间没有连接的干扰数字会造成一个数字被认为是不同的轮廓。
从而九个数字有超过九个轮廓，g_rect就会多出3个。
同时我们的数码管板的特性是上层会出现错误的数字（即有断层的数字），中间是好的0-9（即没有断层），下层是好的数字的镜像（即没有断层）
所以轮廓的多出只会出现在上层
因而将轮廓for循环的终止条件改变一下，保证只循环3次。
```cpp
int s=0;
vector<vector<Point> >::iterator lt;
for(lt=g_contours.begin()+3;s<3;lt++,s++)
```

> 版本3：

我们发现有时候识别的结果不是很好，所以就无法满足触发条件。
但我们注意到我们的小车识别时大致静止的，所以可以采用上次识别的三个数字的位置。


## 4.图像数字排序迭代
> 版本1

因为轮廓识别时，虽然是从下层往上层，每层都识别完后才移动，但同层之间的识别没有顺序。
所以我们需要对提取的三个轮廓进行排序，排序根据左上角的坐标进行排序。

> 版本2

容器自定义排序：前期自己的排序有时会出现指针偏移现象而失败，后期采用sort()函数，自定义排序方式，效果好还简单。


## 5.处理视频迭代
> 版本1

只是修改了g_srcImage的读取方式，从图像转变为视频。

> 版本2

出现了如果一开始没有对准图像就报错的现象。原因是因为没对准，所以轮廓识别的结果为0个，所以g_rect为空，所以需要设置触发条件```if(contours.size()>=9)```
同理，以防万一，给排序也设置一下触发条件```if(g_rect.size()==3)```

> 版本3

数字不要太过频繁地输出，同样结果的话，那输出的数字就不变

# 二、最终版本
## 1.滤波
### （1）测试代码
https://blog.csdn.net/sandalphon4869/article/details/94725601
只要换一下图片就ok
### （2）结论
选择高斯滤波更好，Size值为5.
随便过滤一下，值不用太高，不要也行。
```GaussianBlur(g_srcImage,g_srcImage,Size(5,5),0.0);```

## 2.颜色分离和二值化

## （1）观察颜色
一直鼠标滑轮拉的话可以看到颜色RGB的分布，可以观察要关注的物体的颜色和其余区域的颜色。这样就能通过观察得到选择特定的颜色通道和阈值。
比如：这里数码管的绿色通道就很好，阈值200就很好区分。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707171854316.gif)

## （2）结论
我们的摄像头结果对绿色和蓝色通道的分离很好，我们选择绿色的，阈值为200.

```cpp
#include<opencv2/opencv.hpp>
#include<algorithm>
#include<iostream>
using namespace std;
using namespace cv;

#define W_BLUR "Blur"
#define W_GREEN "Green"
#define W_GREENTHRESHOLD "GreenThreshold"


Mat g_srcImage;
Mat g_srcImageBlur;
Mat g_dstImageGreen;
Mat g_dstImageGreenThreshold;


int g_Blur=11;
int g_dstImageGreenThresholdValue=200;

void mySplit()
{
    vector<Mat> channels;
    split(g_srcImageBlur,channels);
    //绿色通道
    g_dstImageGreen=channels.at(1);

    namedWindow(W_GREEN,WINDOW_NORMAL);
    imshow(W_GREEN,g_dstImageGreen);
}

void myThreshold()
{
    g_dstImageGreen.copyTo(g_dstImageGreenThreshold);
    
    //阈值
    g_dstImageGreenThreshold=g_dstImageGreenThreshold>g_dstImageGreenThresholdValue;

    namedWindow(W_GREENTHRESHOLD,WINDOW_NORMAL);
    imshow(W_GREENTHRESHOLD,g_dstImageGreenThreshold);
}
int main()
{
	
	g_srcImage=imread("N.jpg");
	namedWindow("[src]",WINDOW_NORMAL);
	imshow("[src]",g_srcImage);
	
	
	g_srcImageBlur=g_srcImage.clone();
	GaussianBlur(g_srcImage,g_srcImageBlur,Size(g_Blur,g_Blur),0.0);
	namedWindow(W_BLUR,WINDOW_NORMAL);
	imshow(W_BLUR,g_srcImageBlur);
	
	//split channels
	mySplit();
	
	//threshold=200
	myThreshold();

	waitKey();

	return 0;
}
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709114615108.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

## 3.形态学运算
开运算、闭运算的核的值Size()根据相机的分辨率来调，分辨率高的图片就大些，低的图片就小些。

```cpp
void myMorphology()
{
    Mat midImage;
    //开运算：处理白点
	Mat kernelOpen=getStructuringElement(MORPH_RECT,Size(g_dstImageOpenValue,g_dstImageOpenValue));
	morphologyEx(g_dstImageGreenThreshold,midImage,MORPH_OPEN,kernelOpen);

	//处理数码管之间的缝隙
	Mat kernelClose=getStructuringElement(MORPH_RECT,Size(g_dstImageCloseValue,g_dstImageCloseValue));
	morphologyEx(midImage,g_dstImageMorphology,MORPH_CLOSE,kernelClose);

    namedWindow(W_MORPHOLOGY,WINDOW_NORMAL);
	imshow(W_MORPHOLOGY,g_dstImageMorphology);   
    
}
```
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707194813809.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)
## 4.数字轮廓识别
### （1）初代代码
```cpp
void myContours()
{
    vector<vector<Point> > contours;
    vector<Rect> t_rect;

    findContours(g_dstImageMorphology,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageContours=g_srcImage.clone();

	vector<vector<Point> >::iterator It;

    namedWindow(W_CONTOURS,WINDOW_NORMAL);
    
    for(It = contours.begin();It<contours.end();It++)
    {
        //画出可包围数字的最小矩形
        Rect rect = boundingRect(*It);
        rectangle(dstImageContours,rect,Scalar(193,0,0),10);
        t_rect.push_back(rect);
        imshow(W_CONTOURS,dstImageContours);
        if(waitKey()=='g');
    }
    

    if(t_rect.size()==3)
    {
        g_rect.assign(t_rect.begin(),t_rect.end());
    }
}
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707200510841.gif)

### （2）结论
最终迭代版本

```cpp
void myContours()
{
    vector<vector<Point> > contours;
    vector<Rect> t_rect;

    findContours(g_dstImageMorphology,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageContours=g_srcImage.clone();

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
            rectangle(dstImageContours,rect,Scalar::all(255),20);
            t_rect.push_back(rect);
	    }
	    imshow(W_CONTOURS,dstImageContours);
    }
	//如果不是三个轮廓，那么就不会清除，容器内还是上次的数据
    if(t_rect.size()==3)
    {
        g_rect.assign(t_rect.begin(),t_rect.end());
    }
}
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707195624169.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

## 4.数字识别和截取
因为3中的轮廓识别是随机顺序，所以我们如果想过989的确定顺序，那么就得排序。
排序根据矩形的左上角横坐标来判断先后
```cpp
void myNumberSort()
{
    sort(g_rect.begin(),g_rect.end(),comp);
    

    for(int i=0;i<3;i++)
    {
        Mat ROI=g_dstImageMorphology(Rect(g_rect.at(i)));
        g_dstImageNumber[i]=ROI.clone();
    }

    namedWindow(W_NUMBER1,WINDOW_NORMAL);
    imshow(W_NUMBER1,g_dstImageNumber[0]);
    namedWindow(W_NUMBER2,WINDOW_NORMAL);
    imshow(W_NUMBER2,g_dstImageNumber[1]);
    namedWindow(W_NUMBER3,WINDOW_NORMAL);
    imshow(W_NUMBER3,g_dstImageNumber[2]);
  
}
```
![在这里插入图片描述](https://img-blog.csdnimg.cn/201907072008311.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)
## 5.数字识别（穿线法）
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707214239696.png)

```cpp
//识别中间行的单个数字，识别中间行只需要调用三次，传入不同的参数
void myDiscern(Mat n)
{
	//1的图像，使用穿线会是8。应该从它的尺寸入手，高远大于宽，这里我们选取3倍比.
    if(3*n.cols<n.rows)
    {
        cout<<'1';
        return;
    }
    //竖线
    int x_half=n.cols/2;
    //上横线
    int y_one_third=n.rows/3;
    //下横线
    int y_two_third=n.rows*2/3;
    //每段数码管，0灭，1亮
    int a=0,b=0,c=0,d=0,e=0,f=0,g=0;
	
	//竖线识别a,g,d段
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

	//上横线识别：
    for(int j=0;j<n.cols;j++)
    {
        uchar *data=n.ptr<uchar>(y_one_third);
        //f
        if(j<x_half)
        {
            if(data[j]==255) f=1;
        }
        //b
        else
        {
            if(data[j]==255) b=1;
        }
    }
	
	//下横线识别：
    for(int j=0;j<n.cols;j++)
    {
        uchar *data=n.ptr<uchar>(y_two_third);
		//e
        if(j<x_half)
        {
            if(data[j]==255) e=1;
        }
        //c
        else
        {
            if(data[j]==255) c=1;
        }
    }
	
	//七段管组成的数字
    if(a==1 && b==1 && c==1 && d==1 && e==1 && f==1 && g==0)
    {
        cout<<"0";
    }
    else if(a==0 && b==1 && c==1 && d==0 && e==0 && f==0 && g==0)
    {
        cout<<"1";
    }
    else if(a==1 && b==1 && c==0 && d==1 && e==1 && f==0 && g==1)
    {
        cout<<"2";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==0 && g==1)
    {
        cout<<"3";
    }
    else if(a==0 && b==1 && c==1 && d==0 && e==0 && f==1 && g==1)
    {
        cout<<"4";
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        cout<<"5";
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        cout<<"6";
    }
    else if(a==1 && b==1 && c==1 && d==0 && e==0 && f==0 && g==0)
    {
        cout<<"7";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        cout<<"8";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        cout<<"9";
    }
    else
    {
        printf("[error_%d_%d_%d_%d_%d_%d_%d]",a,b,c,d,e,f,g);
    }

}
```
## 6.图片形式识别release
代码下载：https://download.csdn.net/download/sandalphon4869/11310232
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707201225115.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

```cpp
#include<opencv2/opencv.hpp>
#include<algorithm>
#include<iostream>
using namespace std;
using namespace cv;

#define W_BLUR "Blur"
#define W_GREEN "Green"
#define W_GREENTHRESHOLD "GreenThreshold"
#define W_MORPHOLOGY "Morphology"
#define W_CONTOURS "Contours"
#define W_NUMBER1 "Number1"
#define W_NUMBER2 "Number2"
#define W_NUMBER3 "Number3"


Mat g_srcImage;
Mat g_srcImageBlur;
Mat g_dstImageGreen;
Mat g_dstImageGreenThreshold;
Mat g_dstImageMorphology;
Mat g_dstImageNumber[3];


int g_Blur=5;
int g_dstImageGreenThresholdValue=200;
int g_dstImageOpenValue=7;
int g_dstImageCloseValue=8;

vector<Rect> g_rect;

void mySplit()
{

    vector<Mat> channels;
    split(g_srcImageBlur,channels);
    g_dstImageGreen=channels.at(1);

    namedWindow(W_GREEN,WINDOW_NORMAL);
    imshow(W_GREEN,g_dstImageGreen);
}

void myThreshold()
{
    //deep copy
    g_dstImageGreen.copyTo(g_dstImageGreenThreshold);
    
    //Threshold
    g_dstImageGreenThreshold=g_dstImageGreenThreshold>g_dstImageGreenThresholdValue;

    namedWindow(W_GREENTHRESHOLD,WINDOW_NORMAL);
    imshow(W_GREENTHRESHOLD,g_dstImageGreenThreshold);

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
    
}


void myContours()
{
    vector<vector<Point> > contours;
    vector<Rect> t_rect;

    findContours(g_dstImageMorphology,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageContours=g_srcImage.clone();

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
}

bool comp(const Rect &a, const Rect &b){
    return a.x< b.x;
}

void myNumberSort()
{
    sort(g_rect.begin(),g_rect.end(),comp);
    

    for(int i=0;i<3;i++)
    {
        Mat ROI=g_dstImageMorphology(Rect(g_rect.at(i)));
        g_dstImageNumber[i]=ROI.clone();
    }

    namedWindow(W_NUMBER1,WINDOW_NORMAL);
    imshow(W_NUMBER1,g_dstImageNumber[0]);
    namedWindow(W_NUMBER2,WINDOW_NORMAL);
    imshow(W_NUMBER2,g_dstImageNumber[1]);
    namedWindow(W_NUMBER3,WINDOW_NORMAL);
    imshow(W_NUMBER3,g_dstImageNumber[2]);
    

}


void myDiscern(Mat n)
{
    if(3*n.cols<n.rows)
    {
        cout<<'1';
        return;
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
        cout<<"0";
    }
    else if(a==0 && b==1 && c==1 && d==0 && e==0 && f==0 && g==0)
    {
        cout<<"1";
    }
    else if(a==1 && b==1 && c==0 && d==1 && e==1 && f==0 && g==1)
    {
        cout<<"2";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==0 && g==1)
    {
        cout<<"3";
    }
    else if(a==0 && b==1 && c==1 && d==0 && e==0 && f==1 && g==1)
    {
        cout<<"4";
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        cout<<"5";
    }
    else if(a==1 && b==0 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        cout<<"6";
    }
    else if(a==1 && b==1 && c==1 && d==0 && e==0 && f==0 && g==0)
    {
        cout<<"7";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==1 && f==1 && g==1)
    {
        cout<<"8";
    }
    else if(a==1 && b==1 && c==1 && d==1 && e==0 && f==1 && g==1)
    {
        cout<<"9";
    }
    else
    {
        printf("[error_%d_%d_%d_%d_%d_%d_%d]",a,b,c,d,e,f,g);
    }

}


int main()
{
    g_srcImage=imread("N.jpg");
    namedWindow("[src]",WINDOW_NORMAL);
    imshow("[src]",g_srcImage);
    

    g_srcImageBlur=g_srcImage.clone();
    GaussianBlur(g_srcImage,g_srcImageBlur,Size(g_Blur,g_Blur),0.0);
    // namedWindow(W_BLUR,WINDOW_NORMAL);
    // imshow(W_BLUR,g_srcImageBlur);
    
    //split channels
    mySplit();

    //threshold=160
    myThreshold();

    //clear small white and connection breakpoint
    myMorphology();

    //draw contours
    myContours();

    if(g_rect.size()==3)
    {
        //sort numbers
        myNumberSort();

        //discern number
        cout<<"Number:";
        myDiscern(g_dstImageNumber[0]);
        myDiscern(g_dstImageNumber[1]);
        myDiscern(g_dstImageNumber[2]);
        cout<<endl;
    }

    waitKey();
         
    
	return 0;
}
```
## 7.在线视频识别release
```vector<Rect> g_rect;```需要清零，保证每次读帧都是新的中间的三个数字。
同样的话，那输出的数字就不变

代码下载：https://download.csdn.net/download/sandalphon4869/11310232
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190707212845350.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

```cpp
#include<opencv2/opencv.hpp>
#include<algorithm>
#include<iostream>
using namespace std;
using namespace cv;

#define W_BLUR "Blur"
#define W_GREEN "Green"
#define W_GREENTHRESHOLD "GreenThreshold"
#define W_MORPHOLOGY "Morphology"
#define W_CONTOURS "Contours"
#define W_NUMBER1 "Number1"
#define W_NUMBER2 "Number2"
#define W_NUMBER3 "Number3"


Mat g_srcImage;
Mat g_srcImageBlur;
Mat g_dstImageGreen;
Mat g_dstImageGreenThreshold;
Mat g_dstImageMorphology;
Mat g_dstImageNumber[3];


int g_Blur=5;
int g_dstImageGreenThresholdValue=200;
int g_dstImageOpenValue=7;
int g_dstImageCloseValue=8;

char g_number[4]={'z','e','r'};

vector<Rect> g_rect;

void mySplit()
{

    vector<Mat> channels;
    split(g_srcImageBlur,channels);
    g_dstImageGreen=channels.at(1);

    // namedWindow(W_GREEN,WINDOW_NORMAL);
    // imshow(W_GREEN,g_dstImageGreen);
}

void myThreshold()
{
    //deep copy
    g_dstImageGreen.copyTo(g_dstImageGreenThreshold);
    
    //Threshold
    g_dstImageGreenThreshold=g_dstImageGreenThreshold>g_dstImageGreenThresholdValue;

    // namedWindow(W_GREENTHRESHOLD,WINDOW_NORMAL);
    // imshow(W_GREENTHRESHOLD,g_dstImageGreenThreshold);

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
    
}


void myContours()
{
    vector<vector<Point> > contours;
    vector<Rect> t_rect;

    findContours(g_dstImageMorphology,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    
    Mat dstImageContours=g_srcImage.clone();

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
}

bool comp(const Rect &a, const Rect &b){
    return a.x< b.x;
}

void myNumberSort()
{
    sort(g_rect.begin(),g_rect.end(),comp);
    

    for(int i=0;i<3;i++)
    {
        Mat ROI=g_dstImageMorphology(Rect(g_rect.at(i)));
        g_dstImageNumber[i]=ROI.clone();
    }

    namedWindow(W_NUMBER1,WINDOW_NORMAL);
    imshow(W_NUMBER1,g_dstImageNumber[0]);
    namedWindow(W_NUMBER2,WINDOW_NORMAL);
    imshow(W_NUMBER2,g_dstImageNumber[1]);
    namedWindow(W_NUMBER3,WINDOW_NORMAL);
    imshow(W_NUMBER3,g_dstImageNumber[2]);
    

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

}


int main()
{
    VideoCapture capture("V1.webm");

    while(1)
    {
        capture>>g_srcImage;
        namedWindow("[Video]",WINDOW_NORMAL);
        imshow("[Video]",g_srcImage);
        

        g_srcImageBlur=g_srcImage.clone();
        GaussianBlur(g_srcImage,g_srcImageBlur,Size(g_Blur,g_Blur),0.0);
        // namedWindow(W_BLUR,WINDOW_NORMAL);
        // imshow(W_BLUR,g_srcImageBlur);
        
        //split channels
        mySplit();

        //threshold=160
        myThreshold();

        //clear small white and connection breakpoint
        myMorphology();

        //draw contours
        myContours();

        if(g_rect.size()==3)
        {
            //sort numbers
            myNumberSort();

            //discern number
            char t_number[3];

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
            }
            //其他情况：第一次t_number没识别；g_number存在时，t_number没识别出和t_number和上次一样
            else;
            
        }


        if(waitKey(100)=='q') return 0;

        // if(waitKey()=='g');
        // else if(waitKey()=='q')return 0;
         
    }
	return 0;
}
```

---


# 三、进阶一版
实战时，恶劣的环境因素需要我们作出更多的适应。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709230630386.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709233233246.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

可以看出白光及反射白光的物体造成了很大的干扰，这会让后续的轮廓识别失败。
其实我们不需要更改太多，只要圈出一个ROI矩形区域对应整个数码板就ok。

我的思路是这样的：
- 颜色分离改进：
白光和数码管光的RGB都是255，所以无法直接分离。
但数码管周围会发散出红光，我们可以通过红光分离出数码管板的区域，得到一副红光明显的图像。

- 形态学运算：
分离后的图像可能会出现一些白点甚至是白色光圈，这是因为白光在不断衰减，RGB会从255不断降低。我们通过闭运算就可以消除。

- 外轮廓识别圈出ROI
红光在数码板上分布很多，所以外轮廓识别就会得到数码板区域。

## 1.颜色分离改进
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709222525585.gif)
如：读取一副彩色图像，选择红光的区域。将其区域的分离出来，做成一张二值图
```cpp
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

int main()
{
    Mat srcImage=imread("S1.jpg");
    //dstImage必须创建，要不然无法访问。
    //这里的CV_8UC1表示单通道图
    Mat dstImage=Mat::zeros(srcImage.size(),CV_8UC1);

    //遍历方式是从上往下遍历每一行
    for(int i=0;i<dstImage.rows;i++)
    {
        for(int j=0;j<dstImage.cols;j++)
        {
            //srcImage是三通道图，所以用Vec3b
            //选择红光：R>200，G<150，B<150
            if(srcImage.at<Vec3b>(i,j)[0]<150 && srcImage.at<Vec3b>(i,j)[1]<150 && srcImage.at<Vec3b>(i,j)[2]>200)
            {
                //dstImage是单通道图，所以用uchar
                dstImage.at<uchar>(i,j)=255;
            }
        }
    }
    namedWindow("BGR",WINDOW_NORMAL);
    imshow("BGR",dstImage);

    waitKey();
    return 0;
}
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709230630386.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709230153320.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NhbmRhbHBob240ODY5,size_16,color_FFFFFF,t_70)

颜色分离与访问像素：https://blog.csdn.net/sandalphon4869/article/details/94713547

## 2.外轮廓识别圈出ROI
```if(contours.size()>=1)```是为了鲁棒性，在没有摄像头还没有对准数码管的时候。



```cpp
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
```


# 四、进阶二版
## 1.规则进阶
步兵识别场地内数码管后，裁判当场通过比较机器人身上的数码管显示的数字与场地数码管缺少的数字是否吻合来判断是否识别成功，比赛共有两版数码管，数码管每 1 场比赛共有 3 组缺失的数字，每组有 3 个数字缺失并持续 30s（90 秒一个大周期）。每版数码管最上面一行为干扰项，里面显示的数字没有意义，最下一行为颠倒的数字。
## 2.计算时间
https://blog.csdn.net/sandalphon4869/article/details/95374166
## 3.统计最少出现的数字
如果我们识别的正确率是100%，那么直接输出出现为0次的数字。
但由于我们读取视频帧的时候，如果这时帧恰好是数字切换的一瞬间，那么我们就读到了“错误的”数字，比如2→5，可能切换瞬间就是8，所以正确率就很难保证是100%。
所以我们就得输出最少出现的数字。

我想我们的正确率还是90%以上的，所以最少出现的数字一般就是0次、1次、2次（自信.jpg）。所以我们让等于0次的数输入到result中，让等于1次...直到result中统计到三个数就退出。

```cpp
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
```
## 4.代码Release

```cpp
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
    VideoCapture capture("R2.webm");

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
```
