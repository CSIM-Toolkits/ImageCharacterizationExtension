#include "HistogramFeatures.h"

#include <itkImageFileReader.h>
#include <itkImageRegionIterator.h>

#include <itkImageToHistogramFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

#include <itkImageDuplicator.h>
#include <itkImageFileWriter.h>

#include <iostream>
#include <sstream>
//#include <ofstream.h>
#include <sys/stat.h>

HistogramFeatures::HistogramFeatures(string prefix, ImageType::Pointer img, UCharImageType::Pointer mask, std::vector<int> size){
    this->prefix = prefix;
    this->img = img;
    this->mask = mask;
    this->size = size;
}

HistogramFeatures::~HistogramFeatures(){

}

void HistogramFeatures::Run(){
    //Defines types used
    typedef itk::RescaleIntensityImageFilter<ImageType>                              RescaleFilterType;
    typedef itk::RegionOfInterestImageFilter<ImageType, ImageType>                   ROIType;
    typedef itk::Statistics::ImageToHistogramFilter<UCharImageType>                  HistogramFilterType;
    typedef itk::ImageRegionIterator<ImageType>                                      IteratorType;
    typedef itk::ImageRegionIterator<UCharImageType>                                 UCharIteratorType;

    //typedef itk::ImageDuplicator<ImageType> DuplicatorType;

    //Rescales image for values between 0 and 255
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(this->img);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    //Creates output images
    ImageType::Pointer intImage = ImageType::New();
    intImage->CopyInformation(this->img);
    intImage->SetRegions(this->img->GetRequestedRegion());
    intImage->Allocate();
    intImage->FillBuffer(0);

    ImageType::Pointer meanImage = ImageType::New();
    meanImage->CopyInformation(this->img);
    meanImage->SetRegions(this->img->GetRequestedRegion());
    meanImage->Allocate();
    meanImage->FillBuffer(0);

    ImageType::Pointer stdImage = ImageType::New();
    stdImage->CopyInformation(this->img);
    stdImage->SetRegions(this->img->GetRequestedRegion());
    stdImage->Allocate();
    stdImage->FillBuffer(0);

    ImageType::Pointer kurtImage = ImageType::New();
    kurtImage->CopyInformation(this->img);
    kurtImage->SetRegions(this->img->GetRequestedRegion());
    kurtImage->Allocate();
    kurtImage->FillBuffer(0);

    ImageType::Pointer skewImage = ImageType::New();
    skewImage->CopyInformation(this->img);
    skewImage->SetRegions(this->img->GetRequestedRegion());
    skewImage->Allocate();
    skewImage->FillBuffer(0);

    //Creates histogram filter
    typename HistogramFilterType::Pointer histFilter = HistogramFilterType::New();
    int measurementVectorSize = 1;
    int binsPerDimension = 16;
    HistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    lowerBound.Fill(0);
    HistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    upperBound.Fill(255) ;
    HistogramFilterType::HistogramType::SizeType histSize(measurementVectorSize);
    histSize.Fill(binsPerDimension);

    histFilter->SetHistogramBinMinimum( lowerBound );
    histFilter->SetHistogramBinMaximum( upperBound );
    histFilter->SetHistogramSize( histSize );

    //Creates roi to run through image
    ROIType::Pointer roiFilter = ROIType::New();
    roiFilter->SetInput(rescaleFilter->GetOutput());

    //Creates window to be used in roi
    ImageType::RegionType window;
    ImageType::RegionType::SizeType size;
    for(int i=0; i<this->size.size(); i++)
        size[i]=this->size[i];
    //size[2]=1; //Specific use for 2D analysis *COMMENT THIS LINE IF YOU WANT ISOMETRIC OPERATION
    window.SetSize(size);

    //Create iterators
    IteratorType imgIt (this->img, this->img->GetRequestedRegion());
    UCharIteratorType mapIt (this->mask, this->mask->GetRequestedRegion());
    IteratorType itIt (intImage, intImage->GetRequestedRegion());
    IteratorType mnIt (meanImage, meanImage->GetRequestedRegion());
    IteratorType stIt (stdImage, stdImage->GetRequestedRegion());
    IteratorType ktIt (kurtImage, kurtImage->GetRequestedRegion());
    IteratorType skIt (skewImage, skewImage->GetRequestedRegion());

    //this->img->GetImageDimension()

    imgIt.GoToBegin();
    while(!imgIt.IsAtEnd()){
        mapIt.SetIndex(imgIt.GetIndex());
        if(mapIt.Get()>0){
            window.SetIndex(imgIt.GetIndex());
            ImageType::RegionType region = this->img->GetRequestedRegion();

            if(region.IsInside(window)){
                roiFilter->SetRegionOfInterest(window);
                roiFilter->Update();

                IteratorType roiIt(roiFilter->GetOutput(), roiFilter->GetOutput()->GetRequestedRegion());

                double it, mn, st, sk, kt;

                //Intensity and first moment
                it=0;
                mn=0;
                int count=0;

                roiIt.GoToBegin();
                while(!roiIt.IsAtEnd()){
                    mn+=roiIt.Get();
                    it+=pow((double)roiIt.Get(),2.0);
                    ++count;
                    ++roiIt;
                }

                mn=mn/(double)count;
                it=it/(double)count;

                //Second moment
                st=0;

                roiIt.GoToBegin();
                while(!roiIt.IsAtEnd()){
                    st+=pow(((double)roiIt.Get()-mn),2.0);

                    ++roiIt;
                }
                st=(double)st/(double)count;

                //Third and fourth moment
                sk=0;
                kt=0;

                roiIt.GoToBegin();
                while(!roiIt.IsAtEnd()){
                    if(roiIt.Get()>0){
                        sk+=pow(((double)roiIt.Get()-mn),3.0)/((double)count*pow(st,3.0));
                        kt+=pow(((double)roiIt.Get()-mn),4.0)/((double)count*pow(st,4.0));
                    }
                    ++roiIt;
                }
                //std::cout<<"sk="<<sk<<" kt="<<kt<<std::endl;

                //Attribute values
                itIt.SetIndex(imgIt.GetIndex());
                itIt.Set(it);
                mnIt.SetIndex(imgIt.GetIndex());
                mnIt.Set(mn);
                stIt.SetIndex(imgIt.GetIndex());
                stIt.Set(st);
                skIt.SetIndex(imgIt.GetIndex());
                skIt.Set(sk);
                ktIt.SetIndex(imgIt.GetIndex());
                ktIt.Set(kt);

            }
        }
        ++imgIt;
    }
    typedef itk::ImageFileWriter<ImageType>   WriterType;
    typename WriterType::Pointer writer = WriterType::New();

    typedef itk::RescaleIntensityImageFilter<ImageType> RescaleFilterType;
    typename RescaleFilterType::Pointer rescale = RescaleFilterType::New();

    rescale->SetOutputMinimum(0);
    rescale->SetOutputMaximum(255);

    rescale->SetInput(intImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_intensity.nii.gz" );
    writer->Update();

    rescale->SetInput(meanImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_mean.nii.gz" );
    writer->Update();

    rescale->SetInput(stdImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_std.nii.gz" );
    writer->Update();

    rescale->SetInput(skewImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_skewness.nii.gz" );
    writer->Update();

    rescale->SetInput(kurtImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_kurtosis.nii.gz" );
    writer->Update();
    writer->Update();
}
