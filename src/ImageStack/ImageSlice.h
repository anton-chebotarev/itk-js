#ifndef ImageSlice_H
#define ImageSlice_H

#include "IImage.h"

#include <itkImage.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class ImageView;
    
    struct ContrastFunction;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageSlice : public IImage
      {        
      private:
        typedef itk::Image<unsigned short, 2> _TImageSlice;
        
      private:
        friend class ImageStack;
        explicit ImageSlice(_TImageSlice::Pointer ip_image_stack);
      
      public:
        ~ImageSlice() = default;
        
        // IImage
        virtual unsigned GetDimensions(unsigned i_index) const override;
        virtual unsigned GetComponentSize() const override;
        
        std::unique_ptr<ImageView> CalculateView(const ContrastFunction& i_contrast_function) const;
        
      private:
        _TImageSlice::Pointer mp_image_slice;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif