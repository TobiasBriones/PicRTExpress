#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

namespace OpenCVBridge
{
	public ref class OpenCVHelper sealed
	{

		public:
			OpenCVHelper();
			
			void Brightness(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, double brightness);
			void Convolution(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, int convolution);
			void Flip(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output);
			void Gray(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output);
			void RandomLines(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, int hs, int vs);
			void RGBComponents(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, double r, double g, double b);
			void Rotate(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, double angle);
			void Sepia(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output);
			void Pixelate(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, int radius);
			void Plot(Windows::Graphics::Imaging::SoftwareBitmap^ input, Windows::Graphics::Imaging::SoftwareBitmap^ output, int function, int precision, double scaleX, double scaleY);

		private:
			bool TryConvert(Windows::Graphics::Imaging::SoftwareBitmap^ from, cv::Mat& convertedMat);
			bool GetPointerToPixelData(Windows::Graphics::Imaging::SoftwareBitmap^ bitmap, unsigned char** pPixelData, unsigned int* capacity);

	};
}
