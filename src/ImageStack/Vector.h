#ifndef Vector_H
#define Vector_H

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    struct Vector
      {
      double x;
      double y;
      double z;
      
      double operator[](unsigned i_index) const
        {
        switch (i_index)
          {
          case 0: return x;
          case 1: return y;
          case 2: return z;
          default: throw std::out_of_range("Vector index out of range");
          }
        }
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif