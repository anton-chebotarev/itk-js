#ifndef IAsyncDataLoader_H
#define IAsyncDataLoader_H

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class IAsyncDataLoader
      {
      public:
        class ILoadingProgressReporter;
              
      public:
        virtual ~IAsyncDataLoader() = default;
        
        virtual void SetProgressReporter(std::unique_ptr<ILoadingProgressReporter>&& /*ip_reporter*/) = 0;
        virtual void LoadDataAsync(const std::string& /*i_data_url*/) = 0;    
        virtual void TerminateDataLoading() = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class IAsyncDataLoader::ILoadingProgressReporter
      {
      public:
        virtual ~ILoadingProgressReporter() = default;        
        virtual void OnLoadingProgress(unsigned /*i_bytes_loaded*/, unsigned /*i_bytes_total*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class NullLoadingProgressReporter : public IAsyncDataLoader::ILoadingProgressReporter
      {
      public:
        virtual void OnLoadingProgress(unsigned /*i_bytes_loaded*/, unsigned /*i_bytes_total*/) const override {}
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif