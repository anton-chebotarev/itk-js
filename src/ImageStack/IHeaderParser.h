#ifndef IHeaderParser_H
#define IHeaderParser_H

#include <memory>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class IDataDecoder
      {
      public:
        class IDecodingProgressReporter;
        
      public:
        virtual ~IDataDecoder() = default;      
        virtual void SetProgressReporter(std::unique_ptr<IDecodingProgressReporter>&& /*ip_reporter*/) = 0;        
        virtual void DecodeData(
          void* op_frame_buffer,
          unsigned i_width,
          unsigned i_height,
          unsigned i_count,
          unsigned i_component_size,
          void* ip_data_buffer,
          unsigned i_data_buffer_size) = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class IDataDecoder::IDecodingProgressReporter
      {
      public:
        virtual ~IDecodingProgressReporter() = default;        
        virtual void OnDecodingProgress(unsigned /*i_frame_number*/, unsigned /*i_frames_total*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class NullDecodingProgressReporter : public IDataDecoder::IDecodingProgressReporter
      {
      public:
        virtual void OnDecodingProgress(unsigned /*i_frame_number*/, unsigned /*i_frames_total*/) const override {}
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif