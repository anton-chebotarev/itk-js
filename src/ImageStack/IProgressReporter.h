#ifndef IProgressReporter_H
#define IProgressReporter_H

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    class IProgressReporter
      {
      public:
        virtual ~IProgressReporter() = default;
        
        virtual void OnProgress(unsigned /*i_processed_cnt*/, unsigned /*i_total_cnt*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif