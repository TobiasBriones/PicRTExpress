#include "pch.h"
#include "MatWrapper.h"
#include "OpenCVHelper.h"

using namespace cv;

MatWrapper::MatWrapper(Mat &image) : image(image), width(image.cols), height(image.rows)
{
	this->currentX = 0;
	this->currentY = 0;
}


MatWrapper::~MatWrapper()
{
}

bool MatWrapper::hasNext() {
	return currentY < height;

}

void MatWrapper::nextPixel(MatWrapper::Pixel &pixel, int &x, int &y) 
{
	pixel.b = image.ptr<uchar>(0)[(width * currentY + currentX) * 4 + 0];
	pixel.g = image.ptr<uchar>(0)[(width * currentY + currentX) * 4 + 1];
	pixel.r = image.ptr<uchar>(0)[(width * currentY + currentX) * 4 + 2];
	x = currentX;
	y = currentY;
	currentX++;

	if (currentX == width) {
		currentX = 0;
		currentY++;
	}
}

void MatWrapper::setPixel(Pixel pixel, int x, int y) {
	image.ptr<uchar>(0)[(width * y + x) * 4 + 0] = pixel.b;
	image.ptr<uchar>(0)[(width * y + x) * 4 + 1] = pixel.g;
	image.ptr<uchar>(0)[(width * y + x) * 4 + 2] = pixel.r;
}

uchar MatWrapper::getChannelAt(int x, int y, int channel) {
	return image.ptr<uchar>(0)[(width * y + x) * 4 + channel];

}
