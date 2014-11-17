#include "imageManager.h"

ImageManager::ImageManager( std::vector<std::string> &filenames )
{
	largestDim = 1000; //Constant that determines the longest dim in the low res image
	fileNames = filenames;
	for( itk::SizeValueType i=0; i<filenames.size(); ++i )
	{
		InputImageType::Pointer inputImage = ReadImage( filenames.at(i) );
		if( i )
		{
			if( maxSizeX != inputImage->GetLargestPossibleRegion().GetSize()[0] ||
				maxSizeY != inputImage->GetLargestPossibleRegion().GetSize()[1] )
			{
				std::cerr<<"Image"<<filenames.at(i)
					<<"is not the same size as the original image\n";
				continue;
			}
			onFlags.push_back( false );
		}
		else
		{
			maxSizeX = inputImage->GetLargestPossibleRegion().GetSize()[0];
			maxSizeY = inputImage->GetLargestPossibleRegion().GetSize()[1];
			maxDim = maxSizeX>maxSizeY ? maxSizeX : maxSizeY;
			lowScaleFactor = (((double)maxDim)/((double)largestDim))<1.0 ? 1.0 : (((double)maxDim)/((double)largestDim));
			midScaleFactor = lowScaleFactor/4;
			if( midScaleFactor<1.1 ) midScaleFactor = 1.0;
			onFlags.push_back( true );
		}
		fileNames.push_back( filenames.at(i) );
		inputImage->Register();
		inputImages.push_back( inputImage );
		std::pair<USPixelType,USPixelType> minMaxPair;

		InputImageType::Pointer midResImage = GetDownSampledImage( inputImage, midScaleFactor );
		midResImage->Register();
		midResolution.push_back( midResImage );

		InputImageType::Pointer lowResImage = GetDownSampledImage( inputImage, lowScaleFactor );
		lowResImage->Register();
		lowResolution.push_back( lowResImage );

		UCImageType::Pointer castLowResImage = GetRescaledImage( lowResImage, minMaxPair );
		castLowResImage->Register();
		lowResolutionDownSample.push_back( castLowResImage );

		UCImageType::Pointer castMidResImage = GetRescaledImage( midResImage, minMaxPair );
		castMidResImage->Register();
		midResolutionDownSample.push_back( castMidResImage );

		UCImageType::Pointer castMaxResImage = GetRescaledImage( inputImage, minMaxPair );
		castMaxResImage->Register();
		maxResolutionDownSample.push_back( castMaxResImage );

		minMaxRages.push_back( minMaxPair );
		if( !i )
		{
			midSizeX = midResImage->GetLargestPossibleRegion().GetSize()[0];
			midSizeY = midResImage->GetLargestPossibleRegion().GetSize()[1];
			lowSizeX = lowResImage->GetLargestPossibleRegion().GetSize()[0];
			lowSizeY = lowResImage->GetLargestPossibleRegion().GetSize()[1];
		}
		//Debugging
/*		itk::SizeValueType found1 = filenames.at(i).find_last_of("/\\");
		itk::SizeValueType found2 = filenames.at(i).find_last_of(".");
		std::string outputName = filenames.at(i).substr(found1+1);
		WriteITKImage( castInputImage, outputName );
		outputName = filenames.at(i).substr(found1+1,found2)+"_mid.tif";
		WriteITKImage( midResImage, outputName );
		outputName = filenames.at(i).substr(found1+1,found2)+"_low.tif";
		WriteITKImage( lowResImage, outputName );
*/		//End Debugging
	}
}

UCImageType::Pointer ImageManager::GetRescaledImage( InputImageType::Pointer inputImage, std::pair<USPixelType,USPixelType> &minMaxPair )
{
	typedef itk::RescaleIntensityImageFilter< InputImageType, UCImageType > RescaleIntensityType;
	RescaleIntensityType::Pointer rescale = RescaleIntensityType::New();
	rescale->SetInput( inputImage );
	rescale->SetOutputMaximum( itk::NumericTraits<UCPixelType>::max() );
	rescale->SetOutputMinimum( itk::NumericTraits<UCPixelType>::min() );
	try
	{
		rescale->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << excp << std::endl;
	}
	minMaxPair.first  = rescale->GetInputMinimum();
	minMaxPair.second = rescale->GetInputMaximum();
	UCImageType::Pointer returnPointer = rescale->GetOutput();
	return returnPointer;
}

InputImageType::Pointer ImageManager::GetDownSampledImage( InputImageType::Pointer inputImage, double scaleFactor )
{
	typedef itk::IdentityTransform< double, 2 >  TransformType;
	typedef itk::CastImageFilter< InputImageType, FloatImageType > CastFilterType;
	typedef itk::ResampleImageFilter< FloatImageType, InputImageType > ResampleFilterType;
	typedef itk::LinearInterpolateImageFunction< FloatImageType, double > InterpolatorType;
	typedef itk::RecursiveGaussianImageFilter< FloatImageType, FloatImageType > GaussianFilterType;

	const UCImageType::SpacingType& inputSpacing = inputImage->GetSpacing();

	CastFilterType::Pointer caster = CastFilterType::New();
	caster->SetInput( inputImage );

	GaussianFilterType::Pointer smoother = GaussianFilterType::New();
	smoother->SetInput( caster->GetOutput() );
	smoother->SetSigma( inputSpacing[0]*(scaleFactor/2) );
	smoother->SetNormalizeAcrossScale( true );

	ResampleFilterType::Pointer resampler = ResampleFilterType::New();

	TransformType::Pointer transform = TransformType::New();
	transform->SetIdentity();
	resampler->SetTransform( transform );

	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	resampler->SetInterpolator( interpolator );

	resampler->SetDefaultPixelValue( 0 );

	UCImageType::SpacingType spacing;
	spacing[0] = inputSpacing[0] * scaleFactor;
	spacing[1] = inputSpacing[1] * scaleFactor;

	resampler->SetOutputSpacing( spacing );
	resampler->SetOutputOrigin( inputImage->GetOrigin() );
	resampler->SetOutputDirection( inputImage->GetDirection() );

	UCImageType::SizeType size;
	size[0] = ((double)inputImage->GetLargestPossibleRegion().GetSize()[0])/scaleFactor;
	size[1] = ((double)inputImage->GetLargestPossibleRegion().GetSize()[1])/scaleFactor;
	resampler->SetSize( size );
	resampler->SetInput( smoother->GetOutput() );

	try
	{
		resampler->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << excp << std::endl;
	}
	InputImageType::Pointer returnPointer = resampler->GetOutput();
	return returnPointer;
}

ImageManager::~ImageManager( )
{
	for( itk::SizeValueType i=0; i<inputImages.size(); ++i )
		inputImages.at(i)->UnRegister();
	inputImages.clear();
	for( itk::SizeValueType i=0; i<maxResolutionDownSample.size(); ++i )
		maxResolutionDownSample.at(i)->UnRegister();
	maxResolutionDownSample.clear();
	for( itk::SizeValueType i=0; i<midResolution.size(); ++i )
		midResolution.at(i)->UnRegister();
	midResolution.clear();
	for( itk::SizeValueType i=0; i<midResolutionDownSample.size(); ++i )
		midResolutionDownSample.at(i)->UnRegister();
	midResolutionDownSample.clear();
	for( itk::SizeValueType i=0; i<lowResolution.size(); ++i )
		lowResolution.at(i)->UnRegister();
	lowResolution.clear();
	for( itk::SizeValueType i=0; i<lowResolutionDownSample.size(); ++i )
		lowResolutionDownSample.at(i)->UnRegister();
	lowResolutionDownSample.clear();
}

InputImageType::Pointer ImageManager::ReadImage( std::string &imagePath )
{
	typedef itk::ImageFileReader< InputImageType > ReaderType;
	typedef itk::TIFFImageIO TIFFIOType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName( imagePath.c_str() );
	TIFFIOType::Pointer tiffIO = TIFFIOType::New();
	reader->SetImageIO( tiffIO );
	try
	{
		reader->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << excp << std::endl;
	}
	InputImageType::Pointer returnPointer = reader->GetOutput();
	return returnPointer;
}

void ImageManager::WriteITKImage( UCImageType::Pointer inputImagePointer,
    std::string outputName )
{
  typedef itk::ImageFileWriter< UCImageType > WriterType;
  typedef itk::TIFFImageIO TIFFIOType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputName.c_str() );
  writer->SetInput( inputImagePointer );
  TIFFIOType::Pointer tiffIO = TIFFIOType::New();
  writer->SetImageIO( tiffIO );
  try
  {
    writer->Update();
  }
  catch(itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    exit( EXIT_FAILURE );
  }
  return;
}

std::vector< UCConstIterWIndex > ImageManager::GetLowResDownSampIterators()
{
	std::vector< UCConstIterWIndex > returnVector;
	for( itk::SizeValueType i=0; i<onFlags.size(); ++i )
		if( onFlags.at(i) )
		{
			UCConstIterWIndex currentIter( lowResolutionDownSample.at(i),
				lowResolutionDownSample.at(i)->GetLargestPossibleRegion() );
			returnVector.push_back(currentIter);
		}
	return returnVector;
}

std::vector< USConstIter > ImageManager::GetLowResIterators()
{
	std::vector< USConstIter > returnVector;
	for( itk::SizeValueType i=0; i<lowResolution.size(); ++i )
	{
			USConstIter currentIter( lowResolution.at(i),
				lowResolution.at(i)->GetLargestPossibleRegion() );
			returnVector.push_back(currentIter);
	}
	return returnVector;
}


std::vector< UCConstIter > ImageManager::GetMidResIterators( UCImageType::RegionType region )
{
	std::vector< UCConstIter > returnVector;
	for( itk::SizeValueType i=0; i<midResolutionDownSample.size(); ++i )
	{
			UCConstIter currentIter( midResolutionDownSample.at(i), region );
			returnVector.push_back(currentIter);
	}
	return returnVector;
}

std::vector< UCConstIter > ImageManager::GetMaxResIterators( UCImageType::RegionType region )
{
	std::vector< UCConstIter > returnVector;
	for( itk::SizeValueType i=0; i<maxResolutionDownSample.size(); ++i )
	{
			UCConstIter currentIter( maxResolutionDownSample.at(i), region );
			returnVector.push_back(currentIter);
	}
	return returnVector;
}

std::vector< bool > ImageManager::GetOnFlags()
{
	std::vector< bool > copy ( this->onFlags.begin(), this->onFlags.end() );
	return copy;
}

void ImageManager::SetOnFlags( std::vector< bool > inFlags )
{
	for( itk::SizeValueType i=0; (i<inFlags.size()) && (i<onFlags.size()); ++i )
		onFlags.at(i) = inFlags.at(i);
	return;
}

/*std::vector< UCImageType::Pointer > ImageManager::GetLowResImages()
{
	std::vector< UCImageType::Pointer > returnVector;
	for( itk::SizeValueType i=0; i<onFlags.size(); ++i )
		if( onFlags.at(i) )
			returnVector.push_back(lowResolution.at(i));
	return returnVector;
}

std::vector< UCImageType::Pointer > ImageManager::GetMidResImages()
{
	std::vector< UCImageType::Pointer > returnVector;
	for( itk::SizeValueType i=0; i<onFlags.size(); ++i )
		if( onFlags.at(i) )
			returnVector.push_back(midResolution.at(i));
	return returnVector;
}

std::vector< UCImageType::Pointer > ImageManager::GetMaxResImages()
{
	std::vector< UCImageType::Pointer > returnVector;
	for( itk::SizeValueType i=0; i<onFlags.size(); ++i )
		if( onFlags.at(i) )
			returnVector.push_back(maxResolution.at(i));
	return returnVector;
}*/