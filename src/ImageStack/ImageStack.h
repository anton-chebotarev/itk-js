#ifndef ImageStack_H
#define ImageStack_H

#include <itkImage.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStack
      {        
      private:
        typedef itk::Image<unsigned short, 3> _TImageStack;
        
      private:
        friend class ImageStackBuilder;
        explicit ImageStack(_TImageStack::Pointer ip_image_stack);
      
      public:
        ~ImageStack() = default;
        
        unsigned GetDimensions(unsigned i_index) const;
        unsigned GetComponentSize() const;
        
      private:
        _TImageStack::Pointer mp_image_stack;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif