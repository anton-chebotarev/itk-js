#include "ImageSlice.h"

#include "ImageView.h"

#include "ContrastFunction.h"

#include <itkIntensityWindowingImageFilter.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    ImageSlice::ImageSlice(_TImageSlice::Pointer ip_image_slice)
      : mp_image_slice(ip_image_slice)
      {
      mp_image_slice->DisconnectPipeline();
      }
      
    unsigned ImageSlice::GetDimensions(unsigned i_index) const
      {
      if (i_index >= 1)
        return 0;
      return mp_image_slice->GetBufferedRegion().GetSize()[i_index];
      }
    
    unsigned ImageSlice::GetComponentSize() const
      {
      return sizeof(_TImageSlice::PixelType);
      }
        
    std::unique_ptr<ImageView> ImageSlice::CalculateView(const ContrastFunction& i_contrast_function) const
      {
      typedef itk::IntensityWindowingImageFilter<_TImageSlice, ImageView::_TImageView> TIntensityWindowingImageFilter;
      
      auto p_intensity_windowing_filter = TIntensityWindowingImageFilter::New();
      p_intensity_windowing_filter->SetInput(mp_image_slice);
      p_intensity_windowing_filter->SetWindowMinimum(i_contrast_function.from[0]);
      p_intensity_windowing_filter->SetWindowMaximum(i_contrast_function.from[1]);
      p_intensity_windowing_filter->SetOutputMinimum(i_contrast_function.to[0]);
      p_intensity_windowing_filter->SetOutputMaximum(i_contrast_function.to[1]);
      p_intensity_windowing_filter->Update();
      
      return std::unique_ptr<ImageView>(new ImageView(p_intensity_windowing_filter->GetOutput()));
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }