#ifndef HEVCDataDecoder_H
#define HEVCDataDecoder_H

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class HEVCDataDecoder
      {
      public:
        class IDecodingProgressReporter;
        
      public:
        HEVCDataDecoder();
      
        void SetProgressReporter(std::unique_ptr<IDecodingProgressReporter>&& ip_reporter);
        
        void DecodeData(
          void* op_frame_buffer,
          unsigned i_width,
          unsigned i_height,
          unsigned i_count,
          unsigned i_component_size,
          void* ip_data_buffer,
          unsigned i_data_buffer_size);
        
      private:
        std::unique_ptr<IDecodingProgressReporter> mp_progress_reporter;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class HEVCDataDecoder::IDecodingProgressReporter
      {
      public:
        virtual ~IDecodingProgressReporter() = default;
        
        virtual void OnProgress(unsigned /*i_frame_number*/, unsigned /*i_frames_total*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class NullDecodingProgressReporter : public HEVCDataDecoder::IDecodingProgressReporter
      {
      public:
        virtual void OnProgress(unsigned /*i_frame_number*/, unsigned /*i_frames_total*/) const override;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class StdoutDecodingProgressReporter : public HEVCDataDecoder::IDecodingProgressReporter
      {
      public:
        explicit StdoutDecodingProgressReporter(const std::string& i_stdout_prefix = "Data Decoder: ");
        
        virtual void OnProgress(unsigned i_frame_number, unsigned i_frames_total) const override;
        
      private:
        std::string m_stdout_prefix;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif