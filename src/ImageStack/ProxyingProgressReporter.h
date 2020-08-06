#ifndef ProxyingProgressReporter_H
#define ProxyingProgressReporter_H

#include "IProgressReporter.h"

#include <functional>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    class ProxyingProgressReporter : public IProgressReporter
      {
      public:
        typedef std::function<void(unsigned, unsigned)> TOnProgressCallback;
        
      public:
        explicit ProxyingProgressReporter(TOnProgressCallback i_callback);
        virtual void OnProgress(unsigned i_processed_cnt, unsigned i_total_cnt) const;
        
      private:
          TOnProgressCallback m_callback;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif