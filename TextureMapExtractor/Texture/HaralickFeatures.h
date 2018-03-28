#ifndef HARALICKFEATURES_H
#define HARALICKFEATURES_H

#include <itkImage.h>

using namespace std;

class HaralickFeatures
{
    typedef itk::Image<double, 3> ImageType;
    typedef itk::Image<unsigned char, 2> SliceUCharImageType;
    typedef itk::Image<unsigned char, 3> UCharImageType;

public:
    HaralickFeatures(string prefix, ImageType::Pointer img, UCharImageType::Pointer mask, std::vector<int> size);
    ~HaralickFeatures();
    void Run();
private:
    string prefix;
    ImageType::Pointer img;
    UCharImageType::Pointer mask;
    std::vector<int> size;
};

#endif // HARALICKFEATURES_H
