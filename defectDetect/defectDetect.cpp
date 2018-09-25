#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
#define detectOK 1;

//����ÿ��С������Ԫ��С��ϵ����Ĭ��2 * 2��
int haarTrans(Mat haarBlock, Mat& haarMat)
{
	int rows = haarBlock.rows;
	int cols = haarBlock.cols;
	Mat rowTrans = Mat::zeros(Size(2, 2), CV_64FC1);   //�б任�Ľ��
	Mat haarTrans = Mat::zeros(Size(2, 2), CV_64FC1);  //С���任�����ս��
	for (int i = 0;i < rows / 2;i++)
	{
		for (int j = 0;j < cols;j++)
		{
			rowTrans.at<double>(i, j) = (haarBlock.at<double>(2 * i, j) +
				haarBlock.at<double>(2 * i + 1, j)) / 2.0;
			rowTrans.at<double>(i + rows / 2, j) = (haarBlock.at<double>(2 * i, j) -
				haarBlock.at<double>(2 * i + 1, j)) / 2.0;
		}
	}

	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < cols / 2;j++)
		{
			haarTrans.at<double>(i, j) = (rowTrans.at<double>(i, 2 * j) +
				rowTrans.at<double>(i, 2 * j + 1)) / 2.0;
			haarTrans.at<double>(i, j + cols / 2) = (rowTrans.at<double>(i, 2 * j) -
				rowTrans.at<double>(i, 2 * j + 1)) / 2.0;
		}
	}

	//��С������Ԫ��HaarС��ϵ������������
	haarMat.at<double>(0, 0) = haarTrans.at<double>(0, 0);
	haarMat.at<double>(0, 1) = (haarTrans.at<double>(1, 0) + haarTrans.at<double>(0, 1)
		+ haarTrans.at<double>(1, 1)) / 3.0;

	return detectOK;
}

/*����ÿ��ͳ�Ƶ�ԪС��ϵ���ľ�ֵ�ͷ���*/
int calcStaValue(Mat staUnit, Mat& staUnitMean, Mat& staUnitVar)
{
	Mat staUnitSum = Mat::zeros(Size(6, 1), CV_64FC1);  //ÿ��ͳ�Ƶ�Ԫ�ĸ�С��ϵ����
	Mat staUnitValue = Mat::zeros(Size(6, 4), CV_64FC1);  //�ĸ�С��ͳ�Ƶ�Ԫ��ϵ��
	Mat haarMat = Mat::zeros(Size(2, 1), CV_64FC1);  //���ÿ��С���任��Ԫ���ĸ�ϵ��
	//��ͳ�Ƶ�Ԫ��RGBͨ�����зֽ�
	vector<Mat> unitChannels;
	split(staUnit, unitChannels);
	Mat singleStaUnit(staUnit.size(), CV_64FC1);
	for (int n = 0;n < 3;n++)
	{
		singleStaUnit = unitChannels[n];
		for (int i = 0;i < 2;i++)
		{
			for (int j = 0;j < 2;j++)
			{
				Rect staUnitRect(2 * j, 2 * i, 2, 2);
				Mat haarBlock = singleStaUnit(staUnitRect);  //С���任��Ԫ
				haarTrans(haarBlock, haarMat);  //���С���任��Ԫ������ϵ��
				//��ÿ��С����Ԫ��ϵ���浽ͳ�Ƶ�Ԫ��ϵ��������
				staUnitValue.at<double>(2 * i + j, 2 * n) = haarMat.at<double>(0, 0);
				staUnitValue.at<double>(2 * i + j, 2 * n + 1) = haarMat.at<double>(0, 1);
				//���ͳ�Ƶ�Ԫ�ڸ�ϵ���ĺ�
				staUnitSum(Rect(2 * n, 0, 2, 1)) += haarMat.clone();
			}
		}
	}

	//����ͳ�Ƶ�Ԫ�ڵ�С��ϵ����ֵ
	staUnitMean = staUnitSum.mul(0.25);
	//����ͳ�Ƶ�Ԫ��С��ϵ���ķ���
	for (int i = 0;i < 6;i++)
	{
		for (int j = 0;j < 6;j++)
		{
			//�����i�͵�j��С��ϵ���ķ���
			double haarUnitVarSum = 0;
			for (int k = 0;k < 4;k++)
			{
				haarUnitVarSum += (staUnitValue.at<double>(k, i) - staUnitMean.at<double>(0, i))
					*(staUnitValue.at<double>(k, j) - staUnitMean.at<double>(0, j));
			}
			staUnitVar.at<double>(i, j) = haarUnitVarSum / 3.0;
		}
	}

	return detectOK;
}


double gamma(double xx)
{
	double coef_const[7];
	double step = 2.50662827465;
	double HALF = 0.5;
	double ONE = 1;
	double FPF = 5.5;
	double SER, temp, x, y;
	int j;

	coef_const[1] = 76.18009173;
	coef_const[2] = -86.50532033;
	coef_const[3] = 24.01409822;
	coef_const[4] = -1.231739516;
	coef_const[5] = 0.00120858003;
	coef_const[6] = -0.00000536382;

	x = xx - ONE;
	temp = x + FPF;
	temp = (x + HALF)*log(temp) - temp;
	SER = ONE;
	for (j = 1;j <= 6;j++)
	{
		x = x + ONE;
		SER = SER + coef_const[j] / x;
	}
	y = temp + log(step*SER);

	return exp(y);
}


double beta_cf(double a, double b, double x)
{
	int count, count_max = 100;
	double eps = 0.0000001;
	double AM = 1;
	double BM = 1;
	double AZ = 1;
	double QAB;
	double QAP;
	double QAM;
	double BZ, EM, TEM, D, AP, BP, AAP, BPP, AOLD;

	QAB = a + b;
	QAP = a + 1;
	QAM = a - 1;
	BZ = 1 - QAB*x / QAP;

	for (count = 1;count <= count_max;count++)
	{
		EM = count;
		TEM = EM + EM;
		D = EM*(b - count)*x / ((QAM + TEM)*(a + TEM));
		AP = AZ + D*AM;
		BP = BZ + D*BM;
		D = -(a + EM)*(QAB + EM)*x / ((a + TEM)*(QAP + TEM));
		AAP = AP + D*AZ;
		BPP = BP + D*BZ;
		AOLD = AZ;
		AM = AP / BPP;
		BM = BP / BPP;
		AZ = AAP / BPP;
		BZ = 1;
		if (fabs(AZ - AOLD)<eps*fabs(AZ)) 
			return(AZ);
	}
	return AZ;
}


/*F�ֲ������ܶȺ���*/
double FPdf(double F,double freeDegUp,double freeDegDown)
{
	double FResUp = gamma((freeDegUp + freeDegDown) / 2.0)*pow(freeDegUp, freeDegUp / 2.0)*
		pow(freeDegDown, freeDegDown / 2.0)*pow(F, freeDegUp / 2.0 - 1);
	double FResDown = gamma(freeDegUp / 2.0)*gamma(freeDegDown / 2.0)*
		pow(freeDegUp*F + freeDegDown, (freeDegUp + freeDegDown) / 2.0);
	return FResUp / FResDown;
}


/*F�ֲ����ۼƷֲ�����*/
double FCdf(double F, double freeDegUp, double freeDegDown)
{
	double delta = 0.1;

	double fCdfRes = 0;  //�õ���ۼƸ���
	for (double i = delta; i < F; i += delta)
	{
		fCdfRes += FPdf(i, freeDegUp, freeDegDown)*delta;
	}
	return fCdfRes;
}

/**********************************************/
double betainc(double x, double a, double b)/* ����ȫBeta���� */
{
	double y, BT, logProp;

	if (x == 0 || x == 1)
		BT = 0;
	else
	{
		logProp = log(gamma(a + b)) - log(gamma(a)) - log(gamma(b));
		BT = exp(logProp + a*log(x) + b*log(1 - x));
	}
	if (x < (a + 1) / (a + b + 2))
		y = BT*beta_cf(a, b, x) / a;
	else
		y = 1 - BT*beta_cf(b, a, 1 - x) / b;

	return y;
}

double FDist(double F, double m, double n)
{
	double xx, p;

	if (m <= 0 || n <= 0) p = -1;
	else if (F>0)
	{
		xx = F / (F + n / m);
		p = betainc(xx, m / 2, n / 2);
	}
	return p;
}
/**********************************************/

int main()
{
	Mat srcImg;
	srcImg = imread("defectPic\\timg.jpg",IMREAD_UNCHANGED);

	float scale = 300.0 / srcImg.cols;
	Mat resizeSrc;
	resize(srcImg, resizeSrc, Size(), scale, scale, 1);
	imshow("resizeSrc", resizeSrc);
	
	int rows = srcImg.rows;
	int cols = srcImg.cols;
	int channel = srcImg.channels();

	Mat imgData;
	srcImg.convertTo(imgData, CV_64FC3, 1);

	//����ͳ�Ƶ�Ԫ�С��з����ϵĸ���������ȡ����
	int rowSerial = rows / 4;
	int colSerial = cols / 4;
	Mat staUnit = Mat::zeros(Size(4, 4), CV_64FC1);  //ͳ�Ƶ�Ԫ
	Mat singleStaMean = Mat::zeros(Size(6, 1), CV_64FC1);  //����ͳ�Ƶ�Ԫ��ϵ����ֵ����
	Mat singleStaVar = Mat::zeros(Size(6, 6), CV_64FC1);  //����ͳ�Ƶ�Ԫ��ϵ���������
	Mat staUnitMean[6];  //����ͳ�Ƶ�Ԫ��ϵ����ֵ����
	Mat imgSumMean = Mat::zeros(Size(6, 1), CV_64FC1);  //����ͳ�Ƶ�Ԫ�ĸ�С��ϵ����ֵ֮��
	Mat imgSumVar = Mat::zeros(Size(6, 6), CV_64FC1);  //����ͳ�Ƶ�Ԫ�ĸ�С��ϵ��Э����֮��
	Mat imgMeanValue = Mat::zeros(Size(6, 1), CV_64FC1);  //����ͼ���С��ϵ����ֵ
	Mat imgVarValue = Mat::zeros(Size(6, 6), CV_64FC1);  //����ͼ���С��ϵ��Э�������
	//��ʼ����ֵ����
	for (int k = 0;k < 6;k++)
	{
		staUnitMean[k] = Mat::zeros(Size(colSerial, rowSerial), CV_64FC1);
	}

	/*��ÿ��С����Ԫ��ϵ��*/
	for (int i = 0;i < rowSerial;i++)
	{
		for (int j = 0;j < colSerial;j++)
		{
			//��ȡͳ�Ƶ�Ԫ���ĸ�����ֵ
			Rect staUnitRoi(4 * j, 4 * i, 4, 4);
			staUnit = imgData(staUnitRoi);
			calcStaValue(staUnit, singleStaMean, singleStaVar);
			for (int k = 0;k < 6;k++)
			{
				staUnitMean[k].at<double>(i, j) = singleStaMean.at<double>(0, k);
			}
			//��������ͳ�Ƶ�Ԫϵ����ֵ�ͷ���ĺ�
			imgSumMean += singleStaMean;
			imgSumVar += singleStaVar;
		}
	}

	//��������ͼ���С��ϵ����ֵ��Э����
	double unitScale = 1.0*rowSerial*colSerial;
	imgMeanValue = imgSumMean.mul(1.0 / unitScale);
	imgVarValue = imgSumVar.mul(1.0 / unitScale);
	Mat imgVarInv = Mat::ones(Size(6, 6), CV_64FC1)/imgVarValue;
	//cout << "imgMeanValue=" << imgMeanValue << endl;
	//cout << "imgVarValue=" << imgVarValue << endl;
	//����ͳ��������
	int freeDeg1 = 4;  //F�ֲ����������ɶ�
	int freeDeg2 = 4 * 4 - 4 - 4 + 1;
	//����ÿ��ͳ�Ƶ�Ԫ��HotellingT ^ 2ͳ����
	Mat meanDif = Mat::zeros(Size(6, 1), CV_64FC1);  //�����ֵ��ͼ���ֵ�Ĳ�
	Mat hotelRes;  //ÿ����Ԫ��Hotelling T^2ͳ��ֵ
	Mat defectImg = srcImg.clone();  //������ʾ覴õ�ͼ��
	for (int i = 0;i < rowSerial;i++)
	{
		for (int j = 0;j < colSerial;j++)
		{
			for (int k = 0;k < 6;k++)
			{
				meanDif.at<double>(0, k) = staUnitMean[k].at<double>(i, j) -
					imgMeanValue.at<double>(0, k);
				//cout << meanDif.at<double>(0, k) << " ";
			}
			hotelRes = meanDif*imgVarValue.inv()*(meanDif.t())*(2 * 2);
			//hotelRes = meanDif*imgVarInv*(meanDif.t())*(2 * 2);
			double res = hotelRes.at<double>(0, 0);
			double P1 = 1 - FCdf(res, freeDeg1, freeDeg2);
			double P2 = 1 - FDist(res, freeDeg1, freeDeg2);
			if (P2 < 1e-10)
			{
				Rect rect(4 * j, 4 * i, 4, 4);
				rectangle(defectImg, rect, Scalar(0, 0, 255), 1);
				//cout << "  *******DefectInfo!*******  ";
			}
			//cout << "Res=" << res << endl;
		}
	}

	Mat resizeDefect;
	resize(defectImg, resizeDefect, Size(), scale, scale, 1);
	imshow("resizeDefect", resizeDefect);

	waitKey(0);
	return 0;
}