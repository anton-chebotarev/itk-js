#include "ImageView.h"

#include <itkImageRegionIterator.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    ImageView::ImageView(_TImageView::Pointer ip_image_view)
      : mp_image_view(ip_image_view)
      {
      if (mp_image_view->GetBufferedRegion().GetSize()[2] != 1)
        throw std::logic_error("2D image is expected");
      mp_image_view->DisconnectPipeline();
      }
      
    unsigned ImageView::GetDimensions(unsigned i_index) const
      {
      if (i_index >= 2)
        return 0;
      return mp_image_view->GetBufferedRegion().GetSize()[i_index];
      }
    
    unsigned ImageView::GetComponentSize() const
      {
      return sizeof(_TImageView::PixelType);
      }
      
    void ImageView::FillRGBAPixelBuffer(unsigned char* ip_output_buf, unsigned i_output_buf_size) const
      {      
      _TImageView::RegionType region = mp_image_view->GetBufferedRegion();
      _TImageView::SizeType size = region.GetSize();
      
      if (i_output_buf_size != (size[0] * size[1] * 4))
        throw std::runtime_error("Buffer too small");

      unsigned char* p_buf = ip_output_buf;
      itk::ImageRegionIterator<_TImageView> it(mp_image_view, region);
      for (; !it.IsAtEnd(); ++it)
        {
        *p_buf++ = it.Get();
        *p_buf++ = it.Get();
        *p_buf++ = it.Get();
        *p_buf++ = 255;
        }
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }