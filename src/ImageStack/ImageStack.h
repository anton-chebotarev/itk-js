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
    
    struct Vector;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStack : public IImage
      {        
      private:
        typedef itk::Image<unsigned short, 3> _TImageStack;
        typedef itk::Image<unsigned short, 2> _TImageSlice;
        
      private:
        friend class ImageStackBuilder;
        explicit ImageStack(_TImageStack::Pointer ip_image_stack);
      
      public:
        ~ImageStack() = default;
        
        // IImage
        virtual unsigned GetDimensions(unsigned i_index) const override;
        virtual unsigned GetComponentSize() const override;
        
        Vector GetSpacings() const;
        Vector GetOrigin() const;
        Vector GetVecX() const;
        Vector GetVecY() const;
        Vector GetVecZ() const;
        
        std::unique_ptr<ImageSlice> GetSlice(unsigned i_index) const;
        std::unique_ptr<ImageSlice> GetArbitrarySlice(Vector i_pt_corner, Vector i_vec_x, Vector i_vec_y) const;
        
      private:
        _TImageStack::Pointer mp_image_stack;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif