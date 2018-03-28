#ifndef HISTOGRAMFEATURES_H
#define HISTOGRAMFEATURES_H

#include <itkImage.h>

using namespace std;

class HistogramFeatures
{
    typedef itk::Image<double, 3> ImageType;
    typedef itk::Image<unsigned char, 3> UCharImageType;

public:
    HistogramFeatures(string prefix, ImageType::Pointer img, UCharImageType::Pointer mask, std::vector<int> size);
    ~HistogramFeatures();
    void Run();
private:
    string prefix;
    ImageType::Pointer img;
    UCharImageType::Pointer mask;
    std::vector<int> size;
};

#endif // HISTOGRAMFEATURES_H
