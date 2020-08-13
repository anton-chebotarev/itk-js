#include "ImageStack.h"

#include "ImageSlice.h"
#include "Vector.h"

#include "../MatSDK/ikObliqueSliceImageFilter.h"

#include <itkExtractImageFilter.h>

#include <math.h>

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

    Vector ImageStack::GetSpacings() const
      {
      auto v = mp_image_stack->GetSpacing();
      return Vector{v[0], v[1], v[2]};
      }
    
    Vector ImageStack::GetOrigin() const
      {
      auto v = mp_image_stack->GetOrigin();
      return Vector{v[0], v[1], v[2]};
      }
      
    Vector ImageStack::GetVecX() const
      {
      auto v = mp_image_stack->GetDirection()[0];
      return Vector{v[0], v[1], v[2]};
      }

    Vector ImageStack::GetVecY() const
      {
      auto v = mp_image_stack->GetDirection()[1];
      return Vector{v[0], v[1], v[2]};
      }

    Vector ImageStack::GetVecZ() const
      {
      auto v = mp_image_stack->GetDirection()[2];
      return Vector{v[0], v[1], v[2]};
      }
        
    std::unique_ptr<ImageSlice> ImageStack::GetSlice(unsigned i_index) const
      {      
      _TImageStack::RegionType input_region = mp_image_stack->GetBufferedRegion();
      _TImageStack::SizeType size = input_region.GetSize();

      if (i_index >= size[2])
        throw std::runtime_error("Slice index out of range");
      
      size[2] = 0;
      
      _TImageStack::IndexType start = input_region.GetIndex();
      start[2] = i_index;
      
      _TImageStack::RegionType desired_region;
      desired_region.SetSize(size);
      desired_region.SetIndex(start);
      
      typedef itk::ExtractImageFilter<_TImageStack, _TImageSlice> TExtractImageFilter;
      
      TExtractImageFilter::Pointer p_extract_filter = TExtractImageFilter::New();
      p_extract_filter->SetDirectionCollapseToSubmatrix();
      p_extract_filter->SetExtractionRegion(desired_region);
      p_extract_filter->SetInput(mp_image_stack);      
      p_extract_filter->Update();
      
      return std::unique_ptr<ImageSlice>(new ImageSlice(p_extract_filter->GetOutput()));
      }
      
    std::unique_ptr<ImageSlice> ImageStack::GetArbitrarySlice(Vector i_pt_corner, Vector i_vec_x, Vector i_vec_y) const
      {
      _TImageStack::RegionType input_region = mp_image_stack->GetBufferedRegion();
      _TImageStack::SizeType input_size = input_region.GetSize();
      _TImageStack::SpacingType input_spacing = mp_image_stack->GetSpacing();
      
      unsigned sz = static_cast<unsigned>(ceil(sqrt(input_size[0] * input_size[0] + input_size[1] * input_size[1])));
        
      _TImageSlice::SizeType output_size;
      output_size[0] = input_size[0];
      output_size[1] = input_size[1];
      
      typedef itk::ObliqueSliceImageFilter<_TImageStack, _TImageSlice> TObliqueSliceImageFilter;
      
      TObliqueSliceImageFilter::InputVectorType pt_corner;
      pt_corner[0] = i_pt_corner[0];
      pt_corner[1] = i_pt_corner[1];
      pt_corner[2] = i_pt_corner[2];
      TObliqueSliceImageFilter::InputVectorType vec_x;
      vec_x[0] = i_vec_x[0];
      vec_x[1] = i_vec_x[1];
      vec_x[2] = i_vec_x[2];
      TObliqueSliceImageFilter::InputVectorType vec_y;
      vec_y[0] = i_vec_y[0];
      vec_y[1] = i_vec_y[1];
      vec_y[2] = i_vec_y[2];
      
      TObliqueSliceImageFilter::Pointer p_slice_filter = TObliqueSliceImageFilter::New();
      //p_slice_filter->SetNumberOfThreads(1);
      p_slice_filter->SetFrame(pt_corner, vec_x, vec_y);
      p_slice_filter->SetOutputOrigin(pt_corner.GetDataPointer());
      p_slice_filter->SetOutputSpacing(input_spacing.GetDataPointer());
      p_slice_filter->SetOutputSize(output_size);
      p_slice_filter->SetInterpolator(itk::LinearInterpolateImageFunction<_TImageStack>::New());
      p_slice_filter->SetInput(mp_image_stack);
      p_slice_filter->Update();
      
      return std::unique_ptr<ImageSlice>(new ImageSlice(p_slice_filter->GetOutput()));
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }