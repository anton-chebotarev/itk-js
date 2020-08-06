#ifndef IAsyncDataLoader_H
#define IAsyncDataLoader_H

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class IProgressReporter;
      
    ////////////////////////////////////////////////////////////////////////
          
    class IAsyncDataLoader
      {
      public:
        virtual ~IAsyncDataLoader() = default;
        
        virtual void SetProgressReporter(std::unique_ptr<IProgressReporter>&& /*ip_reporter*/) = 0;
        virtual void LoadDataAsync(const std::string& /*i_data_url*/) = 0;    
        virtual void TerminateDataLoading() = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif