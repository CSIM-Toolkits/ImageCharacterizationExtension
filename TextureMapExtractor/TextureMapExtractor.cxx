#include "itkImageFileWriter.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include "itkPluginUtilities.h"

#include "TextureMapExtractorCLP.h"

#include <itkImageRegionIterator.h>
#include <itkImageConstIterator.h>

#include <itkImageDuplicator.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>

#include "Texture/HaralickFeatures.h"
#include "Texture/HistogramFeatures.h"

#include <iostream>
#include <sstream>

#include <ctime>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <typename TPixel>
int DoIt( int argc, char * argv[], TPixel )
{
  PARSE_ARGS;

  typedef    TPixel InputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<double, 3>          ImageType;
  typedef itk::Image<unsigned char, 3>   UCharImageType;

  typedef itk::ImageFileReader<InputImageType>  InputReaderType;

  typename InputReaderType::Pointer reader1 = InputReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Origninal Volume",
                                        CLPProcessInformation);

  typename InputReaderType::Pointer reader2 = InputReaderType::New();
  itk::PluginFilterWatcher watchReader2(reader2, "Read Labeled Volume",
                                        CLPProcessInformation);
  reader1->SetFileName( inputVolume.c_str() );
  reader2->SetFileName( inputLabels.c_str() );
  reader2->ReleaseDataFlagOn();

  reader1->Update();
  reader2->Update();

  //Creates reader and writer for ImageType (double)
  typedef itk::ImageFileReader<ImageType>   ReaderType;
  typedef itk::ImageFileWriter<ImageType>   WriterType;

  typename ReaderType::Pointer reader = ReaderType::New();
  itk::PluginFilterWatcher watchReader(reader, "Read volume",
                                       CLPProcessInformation);
  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter1(writer, "Write Volume",
                                     CLPProcessInformation);

  //For time measurements
  clock_t begin, end;

  /*###################################################################################
   * Pre processing.
  */

  typedef itk::CastImageFilter<InputImageType, ImageType> CastFilterType;
  typedef itk::CastImageFilter<InputImageType, UCharImageType> UCharCastFilterType;
  typename CastFilterType::Pointer castImage = CastFilterType::New();
  typename UCharCastFilterType::Pointer castMask = UCharCastFilterType::New();
  typename CastFilterType::Pointer castMaskDouble = CastFilterType::New();

  //Casts and saves the input image
  castImage->SetInput(reader1->GetOutput());
  castImage->Update();

  //Casts and duplicate the input map
  castMask->SetInput(reader2->GetOutput());
  castMask->Update();

  //Casts and duplicate input map for blurring calculation
  castMaskDouble->SetInput(reader2->GetOutput());
  castMaskDouble->Update();

  cout<<endl;

  /*###################################################################################
   * 1 step: Computes the haralick features for the image. The result is returned as image maps
   * saved in the path directory.
  */
  if(doHaralick){
      cout<<"**************************"<<endl<<"Doing Haralick step"<<endl;

      begin = clock();

      HaralickFeatures* haralick = new HaralickFeatures(dir+"/"+prefix, castImage->GetOutput(), castMask->GetOutput(), window_size);
      haralick->Run();

      end = clock();
      double time = (double)(end-begin)/CLOCKS_PER_SEC;
      cout<<"Elapsed time = "<<time<<" seg = "<<time/60.0<<" min"<<endl;
  }

  /*###################################################################################
   * 2 step: Computes the histogram features for the image. The result is returned as image maps
   * saved in the path directory.
  */
  if(doHistogram){
      cout<<"**************************"<<endl<<"Doing histogram step"<<endl;

      begin = clock();

      HistogramFeatures* histogram = new HistogramFeatures(dir+"/"+prefix, castImage->GetOutput(), castMask->GetOutput(), window_size);
      histogram->Run();

      end = clock();
      double time = (double)(end-begin)/CLOCKS_PER_SEC;
      cout<<"Elapsed time = "<<time<<" seg = "<<time/60.0<<" min"<<endl;
  }

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<signed char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cerr << "Unknown input image pixel component type: ";
        std::cerr << itk::ImageIOBase::GetComponentTypeAsString( componentType );
        std::cerr << std::endl;
        return EXIT_FAILURE;
        break;
      }
    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
