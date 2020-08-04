#ifndef Header_H
#define Header_H

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    struct Header
      {
      unsigned component_size;
      unsigned dimensions[3];
      double spacing[3];
      double origin[3];
      double direction[3][3];
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif