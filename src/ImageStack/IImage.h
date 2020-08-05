#ifndef IImage_H
#define IImage_H

#include <itkImage.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class IImage
      {             
      public:
        virtual ~IImage() = default;
        
        virtual unsigned GetDimensions(unsigned i_index) const = 0;
        virtual unsigned GetComponentSize() const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif