#ifndef NullProgressReporter_H
#define NullProgressReporter_H

#include "IProgressReporter.h"

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    class NullProgressReporter : public IProgressReporter
      {
      public:        
        virtual void OnProgress(unsigned /*i_processed_cnt*/, unsigned /*i_total_cnt*/) const;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif