#include "ImageStack.h"

#include "ImageSlice.h"

#include <itkExtractImageFilter.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    ImageStack::ImageStack(_TImageStack::Pointer ip_image_stack)
      : mp_image_stack(ip_image_stack)
      {
      mp_image_stack->DisconnectPipeline();
      }
      
    unsigned ImageStack::GetDimensions(unsigned i_index) const
      {
      if (i_index >= 3)
        return 0;
      return mp_image_stack->GetBufferedRegion().GetSize()[i_index];
      }
    
    unsigned ImageStack::GetComponentSize() const
      {
      return sizeof(_TImageStack::PixelType);
      }
        
    std::unique_ptr<ImageSlice> ImageStack::GetSlice(unsigned i_index) const
      {      
      _TImageStack::RegionType input_region = mp_image_stack->GetBufferedRegion();
      _TImageStack::SizeType size = input_region.GetSize();

      if (i_index >= size[2])
        throw std::runtime_error("Slice index out of range");
      
      size[2] = 1; // we extract along z direction
      
      _TImageStack::IndexType start = input_region.GetIndex();
      start[2] = i_index;
      
      _TImageStack::RegionType desired_region;
      desired_region.SetSize(size);
      desired_region.SetIndex(start);
      
      typedef itk::ExtractImageFilter<_TImageStack, _TImageStack> TExtractImageFilter;
      
      TExtractImageFilter::Pointer p_extract_filter = TExtractImageFilter::New();
      p_extract_filter->SetDirectionCollapseToSubmatrix();
      p_extract_filter->SetExtractionRegion(desired_region);
      p_extract_filter->SetInput(mp_image_stack);      
      p_extract_filter->Update();
      
      return std::unique_ptr<ImageSlice>(new ImageSlice(p_extract_filter->GetOutput()));
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }