#ifndef ImageView_H
#define ImageView_H

#include "IImage.h"

#include <itkImage.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageView : public IImage
      {        
      private:
        typedef itk::Image<unsigned char, 3> _TImageView;
        
      private:
        friend class ImageSlice;
        explicit ImageView(_TImageView::Pointer ip_image_view);
      
      public:
        ~ImageView() = default;
        
        // IImage
        virtual unsigned GetDimensions(unsigned i_index) const override;
        virtual unsigned GetComponentSize() const override;
        
        void FillRGBAPixelBuffer(unsigned char* ip_output_buf, unsigned i_output_buf_size) const;
        
      private:
        _TImageView::Pointer mp_image_view;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif