#include "HaralickFeatures.h"

#include <itkImageFileReader.h>
#include <itkImageRegionIterator.h>
#include <itkScalarImageToTextureFeaturesFilter.h>

#include <itkScalarImageToCooccurrenceMatrixFilter.h>
#include <itkHistogramToTextureFeaturesFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>

#include <itkExtractImageFilter.h>

#include <itkImageDuplicator.h>
#include <itkImageFileWriter.h>

#include <iostream>
#include <sstream>
//#include <ofstream.h>
#include <sys/stat.h>

HaralickFeatures::HaralickFeatures(string prefix, ImageType::Pointer img, UCharImageType::Pointer mask, std::vector<int> size){
    this->prefix = prefix;
    this->img = img;
    this->mask = mask;
    this->size = size;
}

HaralickFeatures::~HaralickFeatures(){

}

void HaralickFeatures::Run(){
    //Defines types used
    typedef itk::RescaleIntensityImageFilter<ImageType>                              RescaleFilterType;
    typedef itk::Statistics::ScalarImageToCooccurrenceMatrixFilter<ImageType>        Image2CoOcurrenceType;
    typedef itk::Statistics::HistogramToTextureFeaturesFilter<Image2CoOcurrenceType::HistogramType>
                                                                                     Hist2FeaturesType;
    typedef itk::RegionOfInterestImageFilter<ImageType, ImageType>                   ROIType;
    typedef itk::ImageRegionIterator<ImageType>                                      IteratorType;
    typedef itk::ImageRegionIterator<UCharImageType>                                 UCharIteratorType;
    typedef itk::ImageFileWriter<ImageType>                                          WriterType;

    //typedef itk::ImageDuplicator<ImageType> DuplicatorType;

    //Rescales image for values between 0 and 255
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(this->img);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();

    //Creates output images
    ImageType::Pointer energyImage = ImageType::New();
    energyImage->CopyInformation(this->img);
    energyImage->SetRegions(this->img->GetRequestedRegion());
    energyImage->Allocate();
    energyImage->FillBuffer(0);

    ImageType::Pointer entropyImage = ImageType::New();
    entropyImage->CopyInformation(this->img);
    entropyImage->SetRegions(this->img->GetRequestedRegion());
    entropyImage->Allocate();
    entropyImage->FillBuffer(0);

    ImageType::Pointer correlationImage = ImageType::New();
    correlationImage->CopyInformation(this->img);
    correlationImage->SetRegions(this->img->GetRequestedRegion());
    correlationImage->Allocate();
    correlationImage->FillBuffer(0);

    ImageType::Pointer diffmomImage = ImageType::New();
    diffmomImage->CopyInformation(this->img);
    diffmomImage->SetRegions(this->img->GetRequestedRegion());
    diffmomImage->Allocate();
    diffmomImage->FillBuffer(0);

    ImageType::Pointer inertiaImage = ImageType::New();
    inertiaImage->CopyInformation(this->img);
    inertiaImage->SetRegions(this->img->GetRequestedRegion());
    inertiaImage->Allocate();
    inertiaImage->FillBuffer(0);

    ImageType::Pointer clustShadeImage = ImageType::New();
    clustShadeImage->CopyInformation(this->img);
    clustShadeImage->SetRegions(this->img->GetRequestedRegion());
    clustShadeImage->Allocate();
    clustShadeImage->FillBuffer(0);

    ImageType::Pointer clustPromImage = ImageType::New();
    clustPromImage->CopyInformation(this->img);
    clustPromImage->SetRegions(this->img->GetRequestedRegion());
    clustPromImage->Allocate();
    clustPromImage->FillBuffer(0);

    ImageType::Pointer harCorrImage = ImageType::New();
    harCorrImage->CopyInformation(this->img);
    harCorrImage->SetRegions(this->img->GetRequestedRegion());
    harCorrImage->Allocate();
    harCorrImage->FillBuffer(0);

    //Creates cooccurrence and texture filters
    typename Image2CoOcurrenceType::Pointer coocFilter = Image2CoOcurrenceType::New();
    typename Hist2FeaturesType::Pointer textFilter = Hist2FeaturesType::New();
    coocFilter->SetPixelValueMinMax(0, 255);
    coocFilter->SetNumberOfBinsPerAxis(16);
    //Defines offset for cooccurrence filter
    Image2CoOcurrenceType::OffsetVectorPointer offsetVec = Image2CoOcurrenceType::OffsetVector::New();
    bool breakLoops = false;
    cout<<"haralick offsets:"<<endl;
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            for(int k=-1; k<=1; k++){
                if(i==0 && j==0 && k==0){
                    breakLoops = true;
                }
                if(breakLoops)
                    break;
                ImageType::OffsetType offset = {{i,j,k}};
                offsetVec->push_back(offset);
                cout<<offset;
            }
            if(breakLoops)
                break;
        }
        if(breakLoops)
            break;
    }
    coocFilter->SetOffsets(offsetVec);

    //Creates roi to run through image
    ROIType::Pointer roiFilter = ROIType::New();
    roiFilter->SetInput(rescaleFilter->GetOutput());

    //Creates window to be used in roi
    ImageType::RegionType window;
    ImageType::RegionType::SizeType size;
    for(int i=0; i<this->size.size(); i++)
        size[i]=this->size[i];

    //size.Fill(this->size);

    //size[2]=1; //Specific use for 2D analysis *COMMENT THIS LINE IF YOU WANT ISOMETRIC OPERATION
    window.SetSize(size);

    //Create iterators
    IteratorType imgIt (this->img, this->img->GetRequestedRegion());
    UCharIteratorType mapIt (this->mask, this->mask->GetRequestedRegion());
    IteratorType engIt (energyImage, energyImage->GetRequestedRegion());
    IteratorType entIt (entropyImage, entropyImage->GetRequestedRegion());
    IteratorType corIt (correlationImage, correlationImage->GetRequestedRegion());
    IteratorType dfmIt (diffmomImage, diffmomImage->GetRequestedRegion());
    IteratorType ineIt (inertiaImage, inertiaImage->GetRequestedRegion());
    IteratorType clsIt (clustShadeImage, clustShadeImage->GetRequestedRegion());
    IteratorType clpIt (clustPromImage, clustPromImage->GetRequestedRegion());
    IteratorType hcorIt (harCorrImage, harCorrImage->GetRequestedRegion());

    //this->img->GetImageDimension()

    imgIt.GoToBegin();
    while(!imgIt.IsAtEnd()){
        mapIt.SetIndex(imgIt.GetIndex());
        if(mapIt.Get()!=0){
            window.SetIndex(imgIt.GetIndex());
            ImageType::RegionType region = this->img->GetRequestedRegion();

            if(region.IsInside(window)){
                roiFilter->SetRegionOfInterest(window);
                roiFilter->Update();

                coocFilter->SetInput(roiFilter->GetOutput());
                coocFilter->Update();
                textFilter->SetInput(coocFilter->GetOutput());
                textFilter->Update();

                engIt.SetIndex(imgIt.GetIndex());
                engIt.Set(textFilter->GetFeature(Hist2FeaturesType::Energy));
                entIt.SetIndex(imgIt.GetIndex());
                entIt.Set(textFilter->GetFeature(Hist2FeaturesType::Entropy));
                corIt.SetIndex(imgIt.GetIndex());
                corIt.Set(textFilter->GetFeature(Hist2FeaturesType::Correlation));
                dfmIt.SetIndex(imgIt.GetIndex());
                dfmIt.Set(textFilter->GetFeature(Hist2FeaturesType::InverseDifferenceMoment));
                ineIt.SetIndex(imgIt.GetIndex());
                ineIt.Set(textFilter->GetFeature(Hist2FeaturesType::Inertia));
                clsIt.SetIndex(imgIt.GetIndex());
                clsIt.Set(textFilter->GetFeature(Hist2FeaturesType::ClusterShade));
                clpIt.SetIndex(imgIt.GetIndex());
                clpIt.Set(textFilter->GetFeature(Hist2FeaturesType::ClusterProminence));
                hcorIt.SetIndex(imgIt.GetIndex());
                hcorIt.Set(textFilter->GetFeature(Hist2FeaturesType::HaralickCorrelation));
            }
        }
        ++imgIt;
    }

    typename WriterType::Pointer writer = WriterType::New();
    typename RescaleFilterType::Pointer rescale = RescaleFilterType::New();

    rescale->SetOutputMinimum(0);
    rescale->SetOutputMaximum(255);

    rescale->SetInput(energyImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_energy.nii.gz" );
    writer->Update();

    rescale->SetInput(entropyImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_entropy.nii.gz" );
    writer->Update();

    rescale->SetInput(correlationImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_correlation.nii.gz" );
    writer->Update();

    rescale->SetInput(diffmomImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_inv_diff_moment.nii.gz" );
    writer->Update();

    rescale->SetInput(inertiaImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_inertia.nii.gz" );
    writer->Update();

    rescale->SetInput(clustShadeImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_cluster_shade.nii.gz" );
    writer->Update();

    rescale->SetInput(clustPromImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_cluster_prom.nii.gz" );
    writer->Update();

    rescale->SetInput(harCorrImage);
    rescale->Update();
    writer->SetInput(rescale->GetOutput());
    writer->SetFileName( this->prefix+"_har_correlation.nii.gz" );
    writer->Update();
}
