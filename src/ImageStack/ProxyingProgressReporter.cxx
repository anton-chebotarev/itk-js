#include "ProxyingProgressReporter.h"

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    ProxyingProgressReporter::ProxyingProgressReporter(TOnProgressCallback i_callback)
      : m_callback(i_callback)
      {
      }
    
    void ProxyingProgressReporter::OnProgress(unsigned i_processed_cnt, unsigned i_total_cnt) const
      {
      m_callback(i_processed_cnt, i_total_cnt);
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }