#include <itkImage.h>
#include <itkTIFFImageIO.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkCastImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>
#include <itkRecursiveGaussianImageFilter.h>

/*This class is written for 2D 16-bit grayscale images but the behaviour remains true for
8-bit images*/
typedef float FloatPixelType;
typedef unsigned short USPixelType;
typedef unsigned char  UCPixelType;
typedef itk::Image<USPixelType, 2> InputImageType;
typedef itk::Image<UCPixelType, 2> UCImageType;
typedef itk::Image<FloatPixelType, 2> FloatImageType;
typedef itk::ImageRegionIteratorWithIndex<UCImageType> UCIter;
typedef itk::ImageRegionIteratorWithIndex<InputImageType> USIter;
typedef itk::ImageRegionConstIterator<UCImageType> UCConstIter;
typedef itk::ImageRegionConstIterator<InputImageType> USConstIter;
typedef itk::ImageRegionConstIteratorWithIndex<UCImageType> UCConstIterWIndex;
typedef itk::ImageRegionConstIteratorWithIndex<InputImageType> USConstIterWIndex;

class ImageManager
{
public:
	ImageManager( std::vector< std::string > & filenames );
	~ImageManager();
	
	//Only Passes The Images Set to On
	std::vector< UCConstIterWIndex > GetLowResDownSampIterators();
	std::vector< USConstIter > GetLowResIterators();
	std::vector< UCConstIter > GetMidResIterators(UCImageType::RegionType);
	std::vector< UCConstIter > GetMaxResIterators(UCImageType::RegionType);
	/*std::vector< InputImageType::Pointer > GetLowResImages();
	std::vector< UCImageType::Pointer > GetMidResImages();
	std::vector< UCImageType::Pointer > GetMaxResImages();*/
	double GetLowScaleFactor() { return lowScaleFactor; };
	double GetMidScaleFactor() { return midScaleFactor; };
	itk::SizeValueType GetmaxSizeX(){ return maxSizeX; };
	itk::SizeValueType GetmaxSizeY(){ return maxSizeY; };
	itk::SizeValueType GetmidSizeX(){ return midSizeX; };
	itk::SizeValueType GetmidSizeY(){ return midSizeY; };
	itk::SizeValueType GetlowSizeX(){ return lowSizeX; };
	itk::SizeValueType GetlowSizeY(){ return lowSizeY; };
	std::vector< bool > GetOnFlags();
	void SetOnFlags( std::vector< bool > flags );
	void WriteITKImage( UCImageType::Pointer inputImagePointer, std::string outputName );

private:
	std::vector< InputImageType::Pointer > inputImages;
	std::vector< InputImageType::Pointer > midResolution;
	std::vector< InputImageType::Pointer > lowResolution;
	std::vector< UCImageType::Pointer > lowResolutionDownSample;
	std::vector< UCImageType::Pointer > midResolutionDownSample;
	std::vector< UCImageType::Pointer > maxResolutionDownSample;
	std::vector< std::pair< USPixelType, USPixelType > > minMaxRages;
	std::vector< bool > onFlags;
	std::vector< std::string > fileNames;
	itk::SizeValueType maxDim, largestDim;
	double lowScaleFactor, midScaleFactor;
	itk::SizeValueType maxSizeX, maxSizeY, midSizeX, midSizeY, lowSizeX, lowSizeY;

	InputImageType::Pointer ReadImage( std::string &imagePath );
	InputImageType::Pointer GetDownSampledImage( InputImageType::Pointer inputImage, double scale );
	UCImageType::Pointer GetRescaledImage( InputImageType::Pointer inputImage, std::pair<USPixelType,USPixelType> &minMaxPair );
};