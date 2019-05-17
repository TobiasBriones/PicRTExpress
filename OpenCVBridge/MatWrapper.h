#pragma once
#include "pch.h"
#include "OpenCVHelper.h"

using namespace cv;

class MatWrapper
{

	private:
		Mat &image;
		const int width;
		const int height;
		int currentX;
		int currentY;
		bool isIterating;

	public:
		MatWrapper(Mat &image);
		~MatWrapper();

		struct Pixel
		{
			uchar r;
			uchar g;
			uchar b;
		};

		bool hasNext();
		void nextPixel(Pixel& pixel, int& x, int& y);
		void setPixel(Pixel pixel, int x, int y);
		uchar getChannelAt(int x, int y, int channel);

};

