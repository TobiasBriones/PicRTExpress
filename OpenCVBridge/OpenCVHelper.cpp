#include "pch.h"
#include "OpenCVHelper.h"
#include "MemoryBuffer.h"
#include <stdlib.h>
#include <time.h> 
#include <math.h> 
#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <MatWrapper.h>

using namespace OpenCVBridge;
using namespace Platform;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;
using namespace cv;

OpenCVHelper::OpenCVHelper()
{
	srand((uint) time(NULL));
}

bool OpenCVHelper::GetPointerToPixelData(SoftwareBitmap^ bitmap, unsigned char** pPixelData, unsigned int* capacity)
{
	BitmapBuffer^ bmpBuffer = bitmap->LockBuffer(BitmapBufferAccessMode::ReadWrite);
	IMemoryBufferReference^ reference = bmpBuffer->CreateReference();

	ComPtr<IMemoryBufferByteAccess> pBufferByteAccess;
	if ((reinterpret_cast<IInspectable*>(reference)->QueryInterface(IID_PPV_ARGS(&pBufferByteAccess))) != S_OK)
	{
		return false;
	}

	if (pBufferByteAccess->GetBuffer(pPixelData, capacity) != S_OK)
	{
		return false;
	}
	return true;
}

bool OpenCVHelper::TryConvert(SoftwareBitmap^ from, Mat& convertedMat)
{
	unsigned char* pPixels = nullptr;
	unsigned int capacity = 0;

	if (!GetPointerToPixelData(from, &pPixels, &capacity))
	{
		return false;
	}

	Mat mat(from->PixelHeight,
			from->PixelWidth,
			CV_8UC4,			// assume input SoftwareBitmap is BGRA8
			(void*) pPixels);

	// shallow copy because we want convertedMat.data = pPixels
	// don't use .copyTo or .clone
	convertedMat = mat;
	return true;
}

void OpenCVHelper::Brightness(SoftwareBitmap^ input, SoftwareBitmap^ output, double brightness)
{
	struct Helper {
		static uchar getComponent(uchar original, double brightness) {
			if (brightness == 0) {
				return original;
			}
			if (brightness > 0) {
				uchar complement = 255 - original;
				return original + (uchar)(brightness * complement);
			}
			return original + (uchar)(brightness * original);
		}
	};
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	/*if (!outputMat.isContinuous()) return;
	MatWrapper wrapper(inputMat);
	MatWrapper wrapper2(outputMat);
	int x, y;
	MatWrapper::Pixel pixel;
	MatWrapper::Pixel pixel2;

	pixel2.r = pixel2.g = pixel2.b = 100;

	while (wrapper.hasNext()) {
		wrapper.nextPixel(pixel, x, y);

		pixel2.b=Helper::getComponent(pixel.b, brightness);
		pixel2.g = Helper::getComponent(pixel.g, brightness);
		pixel2.r = Helper::getComponent(pixel.r, brightness);
		wrapper2.setPixel(pixel2, x, y);
	}
	*/

	// accept only char type matrices
	CV_Assert(outputMat.depth() == CV_8U);

	int channels = outputMat.channels();
	int nRows = outputMat.rows;
	int nCols = outputMat.cols * channels;
	int x = 0;
	int y = 0;
	int channelCount = 0;
	bool isContinuous = outputMat.isContinuous();
	uchar * p;
	double amount = 0;

	// We have two storage stuctures, as a matrix or one row and one whole column
	if (isContinuous)
	{
		nCols *= nRows;
		nRows = 1;
	}
	// _______________________

	for (int i = 0; i < nRows; ++i)
	{
		// Get the pointer to the current row
		p = outputMat.ptr<uchar>(i);

		for (int j = 0; j < nCols; ++j)
		{
			// Apply changes to pixel components
			p[j] = Helper::getComponent(inputMat.ptr<uchar>(i)[j], brightness);
			// ______________________________________

			channelCount++;

			// Update the coordinate x if needed
			if (channelCount == channels)
			{
				x++;
				channelCount = 0;

				if (isContinuous && x == outputMat.cols)
				{
					x = 0;
					y++;
				}
			}
		}
		x = 0;
		y++;
	}

}

void OpenCVHelper::Convolution(SoftwareBitmap^ input, SoftwareBitmap^ output, int convolution)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	Mat kernel;

	switch (convolution) {
		// Blur
		case 0:
			blur(inputMat, outputMat, cv::Size(11, 11), cv::Point(-1, -1), BORDER_DEFAULT);
			return;

		// Gaussian Blur
		case 1:
			GaussianBlur(inputMat, outputMat, cv::Size(7, 7), 0, 0);
			return;

		// Line Detection
		case 2:
			kernel = (Mat_<float>(3, 3) <<  -1, -1, -1,
											 2,  2,  2,
											-1, -1, -1);
			break;

		// Edge Detection
		case 3:
			kernel = (Mat_<float>(3, 3) <<  -1, -1, -1,
											-1,  8, -1,
											-1, -1, -1);
			break;
	}
	filter2D(inputMat, outputMat, -1, kernel, cv::Point(-1, -1), 0, BORDER_DEFAULT);
	
}

void OpenCVHelper::Flip(SoftwareBitmap^ input, SoftwareBitmap^ output)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	MatWrapper inputWrapper(inputMat);
	MatWrapper outputWrapper(outputMat);
	MatWrapper::Pixel pixel1;
	int w = outputMat.cols;
	int x;
	int y;

	while (inputWrapper.hasNext()) {
		inputWrapper.nextPixel(pixel1, x, y);
		int fx = w - x - 1;

		outputWrapper.setPixel(pixel1, fx, y);
	}
}

void OpenCVHelper::Gray(SoftwareBitmap^ input, SoftwareBitmap^ output)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;

	MatWrapper inputWrapper(inputMat);
	MatWrapper outputWrapper(outputMat);
	MatWrapper::Pixel pixel1;
	MatWrapper::Pixel pixel2;
	int x;
	int y;

	while (inputWrapper.hasNext()) {
		inputWrapper.nextPixel(pixel1, x, y);
		uchar val = (uchar) (0.299 * pixel1.r + 0.587 * pixel1.g + 0.114 * pixel1.b);
		pixel2.b = val;
		pixel2.g = val;
		pixel2.r = val;

		outputWrapper.setPixel(pixel2, x, y);
	}
}

void OpenCVHelper::RandomLines(SoftwareBitmap^ input, SoftwareBitmap^ output, int hs, int vs)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;

	// accept only char type matrices
	CV_Assert(outputMat.depth() == CV_8U);

	int channels = outputMat.channels();
	int nRows = outputMat.rows;
	int nCols = outputMat.cols * channels;
	int x = 0;
	int y = 0;
	int channelCount = 0;
	bool isContinuous = outputMat.isContinuous();
	uchar * p;
	uchar * currentColorH;
	uchar * currentColorV;

	if (3 < channels) {
		currentColorH = new uchar[5];
		currentColorV = new uchar[5];
	}
	else {
		currentColorH = new uchar[3];
		currentColorV = new uchar[3];
	}

	// Init first random color
	for (int c = 0; c < channels; c++) {
		currentColorH[c] = rand() % 256;
		currentColorV[c] = 0;
	}
	// __________________________________

	// We have two storage stuctures, as a matrix or one row and one whole column
	if (isContinuous)
	{
		nCols *= nRows;
		nRows = 1;
	}
	// _______________________

	for (int i = 0; i < nRows; ++i)
	{
		// Get the pointer to the current row
		p = outputMat.ptr<uchar>(i);

		// For example: |1 pixel|	|1 pixel|			|1 pixel|
		//				  [bgr]		   [bgr]	...		  [bgr]
		for (int j = 0; j < nCols; ++j)
		{
			// Apply changes to pixel components
			if (hs != 0 && x % hs == 0)
			{
				p[j] = currentColorV[channelCount];
			}
			else if (vs != 0 && y % vs == 0)
			{
				p[j] = currentColorH[channelCount];
			}
			else {
				p[j] = inputMat.ptr<uchar>(i)[j];
			}

			// ______________________________________
			channelCount++;

			// Update the coordinate x if needed
			if (channelCount == channels)
			{
				x++;
				channelCount = 0;

				if (isContinuous && x == outputMat.cols)
				{
					x = 0;
					y++;

					// Change the random color for each row
					// Set next random color when matrix is continuously stored
					for (int c = 0; c < channels; c++) {
						currentColorH[c] = rand() % 256;
					}
					// __________________________________

				}
			}
		}
		x = 0;
		y++;

		// Change the random color for each row
		// Set next random color when matrix normally stored
		for (int c = 0; c < channels; c++) {
			currentColorH[c] = rand() % 256;
		}
		// __________________________________
	}
	delete[] currentColorH;
	delete[] currentColorV;
}

void OpenCVHelper::RGBComponents(SoftwareBitmap^ input, SoftwareBitmap^ output, double r, double g, double b)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	MatWrapper inputWrapper(inputMat);
	MatWrapper outputWrapper(outputMat);
	MatWrapper::Pixel pixel1;
	MatWrapper::Pixel pixel2;
	int x;
	int y;

	while (inputWrapper.hasNext()) {
		inputWrapper.nextPixel(pixel1, x, y);

		pixel2.b = (uchar)(pixel1.b * b);
		pixel2.g = (uchar)(pixel1.g * g);
		pixel2.r = (uchar)(pixel1.r * r);
		outputWrapper.setPixel(pixel2, x, y);
	}
}

void OpenCVHelper::Rotate(SoftwareBitmap^ input, SoftwareBitmap^ output, double angle)
{
	struct Helper {
		static void rotate(int x, int y, int& rx, int& ry, int cx, int cy, double angle) {
			rx = (int) (((x - cx) * cos(angle) - (y - cy) * sin(angle)) + cx);
			ry = (int) (((x - cx) * sin(angle) + (y - cy) * cos(angle)) + cy);
		}

		static bool inBounds(int x, int y, Mat& image) {
			return (0 <= x && x < image.cols) && (0 <= y && y < image.rows);
		}
	};
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	MatWrapper inputWrapper(inputMat);
	MatWrapper outputWrapper(outputMat);
	MatWrapper::Pixel pixel1;
	int cx = outputMat.cols / 2;
	int cy = outputMat.rows / 2;
	int x;
	int y;
	int rx;
	int ry;

	while (inputWrapper.hasNext()) {
		inputWrapper.nextPixel(pixel1, x, y);

		Helper::rotate(x, y, rx, ry, cx, cy, angle);
		if (Helper::inBounds(rx, ry, outputMat)) {
			outputWrapper.setPixel(pixel1, rx, ry);
		}
	}
}

void OpenCVHelper::Sepia(SoftwareBitmap^ input, SoftwareBitmap^ output)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	MatWrapper inputWrapper(inputMat);
	MatWrapper outputWrapper(outputMat);
	MatWrapper::Pixel pixel1;
	MatWrapper::Pixel pixel2;
	int x, y, tr, tg, tb;

	while (inputWrapper.hasNext()) {
		inputWrapper.nextPixel(pixel1, x, y);

		tr = (int) (0.393 * pixel1.r + 0.769 * pixel1.g + 0.189 * pixel1.b);
		tg = (int) (0.349 * pixel1.r + 0.686 * pixel1.g + 0.168 * pixel1.b);
		tb = (int) (0.272 * pixel1.r + 0.534 * pixel1.g + 0.131 * pixel1.b);

		if (tr > 255) tr = 255;
		if (tg > 255) tg = 255;
		if (tb > 255) tb = 255;

		pixel2.b = tb;
		pixel2.g = tg;
		pixel2.r = tr;

		outputWrapper.setPixel(pixel2, x, y);
	}
}

void OpenCVHelper::Pixelate(SoftwareBitmap^ input, SoftwareBitmap^ output, int radius)
{
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	// accept only char type matrices
	CV_Assert(outputMat.depth() == CV_8U);

	int channels = outputMat.channels();
	int nRows = outputMat.rows;
	int nCols = outputMat.cols * channels;
	int x = 0;
	int y = 0;
	int channelCount = 0;
	bool isContinuous = outputMat.isContinuous();
	uchar * p;
	int cx = 0;
	int cy = 0;

	// We have two storage stuctures, as a matrix or one row and one whole column
	if (isContinuous)
	{
		nCols *= nRows;
		nRows = 1;
	}
	// _______________________

	for (int i = 0; i < nRows; ++i)
	{
		// Get the pointer to the current row
		p = outputMat.ptr<uchar>(i);

		for (int j = 0; j < nCols; ++j)
		{
			// Apply changes to pixel components
			cx = x;
			cx %= 2 * radius + 1;
			cx -= radius;
			cx = x - cx;
			cy = y;
			cy %= 2 * radius + 1;
			cy -= radius;
			cy = y - cy;

			if (cx < 0) cx = 0;
			if (cx >= input->PixelWidth) cx = input->PixelWidth - 1;
			if (cy < 0) cy = 0;
			if (cy >= input->PixelHeight) cy = input->PixelHeight - 1;

			switch (channelCount) {
				// B
			case 0:
				p[j] = inputMat.ptr<uchar>(i)[(inputMat.cols * cy + cx) * 4];
				break;

				// G
			case 1:
				p[j] = inputMat.ptr<uchar>(i)[(inputMat.cols * cy + cx) * 4 + 1];
				break;

				// R
			case 2:
				p[j] = inputMat.ptr<uchar>(i)[(inputMat.cols * cy + cx) * 4 + 2];
				break;
			}
			// ______________________________________
			channelCount++;

			// Update the coordinate x if needed
			if (channelCount == channels)
			{
				x++;
				channelCount = 0;

				if (isContinuous && x == outputMat.cols)
				{
					x = 0;
					y++;
				}
			}
		}
		x = 0;
		y++;
	}
}

void OpenCVHelper::Plot(SoftwareBitmap^ input, SoftwareBitmap^ output, int function, int precision, double scaleX, double scaleY)
{
	struct Helper {
		static int getValue(int value, double scale) {
			return (int) ((double) value * scale);
		}

		static void setChannel(uchar *p, int j, int channelCount, uchar currentB, uchar currentG, uchar currentR) {
			switch (channelCount) {
				// B
				case 0:
					p[j] = currentB;
					break;

				// G
				case 1:
					p[j] = currentG;
					break;

				// R
				case 2:
					p[j] = currentR;
					break;
			}
		}
	};
	Mat inputMat, outputMat;
	if (!(TryConvert(input, inputMat) && TryConvert(output, outputMat))) return;
	// accept only char type matrices
	CV_Assert(outputMat.depth() == CV_8U);

	int channels = outputMat.channels();
	int nRows = outputMat.rows;
	int nCols = outputMat.cols * channels;
	int x = 0;
	int y = 0;
	int _y = 0;
	int channelCount = 0;
	bool isContinuous = outputMat.isContinuous();
	uchar * p;
	char currentB = 0;
	char currentG = 0;
	char currentR = 0;

	// We have two storage stuctures, as a matrix or one row and one whole column
	if (isContinuous)
	{
		nCols *= nRows;
		nRows = 1;
	}
	// _______________________

	for (int i = 0; i < nRows; ++i)
	{
		// Get the pointer to the current row
		p = outputMat.ptr<uchar>(i);

		for (int j = 0; j < nCols; ++j)
		{
			// Apply changes to pixel components
			// Copy original image data
			p[j] = inputMat.ptr<uchar>(i)[j];
			int scaledX = Helper::getValue(x, scaleX);
			int scaledY = Helper::getValue(_y, scaleY);

			// Draw the function if <x, y> belongs to its plot
			switch (function) {
				case 0:
					if (_y >= x - precision && _y <= x + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 1:
					if (scaledY >= pow(scaledX, 2) - precision && scaledY <= pow(scaledX, 2) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 2:
					if (scaledY >= pow(scaledX, 3) - precision && scaledY <= pow(scaledX, 3) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 3:
					if (scaledY >= pow(scaledX, 0.5) - precision && scaledY <= pow(scaledX, 0.5) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 4:
					if (scaledY >= sin(scaledX) - precision && scaledY <= sin(scaledX) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 5:
					if (scaledY >= cos(scaledX) - precision && scaledY <= cos(scaledX) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 6:
					if (scaledY >= tan(scaledX) - precision && scaledY <= tan(scaledX) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;

				case 7:
					if (scaledY >= log(scaledX) - precision && scaledY <= log(scaledX) + precision)
					{
						Helper::setChannel(p, j, channelCount, currentB, currentG, currentR);
					}
					break;
			}
			// ______________________________________
			channelCount++;

			// Update the coordinate x if needed
			if (channelCount == channels)
			{
				x++;
				channelCount = 0;

				if (isContinuous && x == outputMat.cols)
				{
					x = 0;
					y++;
					_y = outputMat.rows - y;
				}
			}
		}
	}
}
