#ifndef ImageStack_H
#define ImageStack_H

#include "IImage.h"

#include <itkImage.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class ImageSlice;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStack : public IImage
      {        
      private:
        typedef itk::Image<unsigned short, 3> _TImageStack;
        
      private:
        friend class ImageStackBuilder;
        explicit ImageStack(_TImageStack::Pointer ip_image_stack);
      
      public:
        ~ImageStack() = default;
        
        // IImage
        virtual unsigned GetDimensions(unsigned i_index) const override;
        virtual unsigned GetComponentSize() const override;
        
        std::unique_ptr<ImageSlice> GetSlice(unsigned i_index) const;
        
      private:
        _TImageStack::Pointer mp_image_stack;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif