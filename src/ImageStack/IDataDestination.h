#ifndef IDataDestination_H
#define IDataDestination_H

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    class IDataDestination
      {
      public:
        virtual ~IDataDestination() = default;
        
        virtual void ProcessLoadedData(void* /*ip_buffer*/, unsigned /*i_buffer_size*/) = 0;
        virtual void OnDataLoadingFailed(const char* /*ip_description*/) = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif