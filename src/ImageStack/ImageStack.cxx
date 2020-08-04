#include "ImageStack.h"

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    ImageStack::ImageStack(_TImageStack::Pointer ip_image_stack)
      : mp_image_stack(ip_image_stack)
      {
      }
      
    unsigned ImageStack::GetDimensions(unsigned i_index) const
      {
      return mp_image_stack->GetLargestPossibleRegion().GetSize()[i_index];
      }
    
    unsigned ImageStack::GetComponentSize() const
      {
      return 2;
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }