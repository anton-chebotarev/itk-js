#ifndef ImageStackBuilder_H
#define ImageStackBuilder_H

#include "IDataDestination.h"

#include <itkImage.h>

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class IAsyncDataLoader;
    class IDataDecoder;
    class IHeaderParser;
    
    struct Header;
    
    class ImageStack;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStackBuilder
      {
      public:
        typedef std::function<void(std::unique_ptr<ImageStack>&&)> TOnReadyCallback;
        typedef std::function<void(const std::string&)> TOnFailedCallback;
        typedef std::function<void(const std::string&)> TStatusCallback;
        typedef std::function<void(unsigned, unsigned)> TOnProgressCallback;
        
      public:
        ImageStackBuilder();
        ~ImageStackBuilder() = default;
        
        void OnHeaderLoadingProgress(TOnProgressCallback i_callback);
        void OnDataLoadingProgress(TOnProgressCallback i_callback);
        void OnDataDecodingProgress(TOnProgressCallback i_callback);
        
        void LoadDataAsync(
          const std::string& i_header_url,
          const std::string& i_data_url,
          TOnReadyCallback i_on_ready_callback,
          TOnFailedCallback i_on_failed_callback,
          TStatusCallback i_status_callback);
        
      private:
        class _DataDestinationProxy;
        
      private:        
        friend class _DataDestinationProxy;
        
        void _ProcessLoadedHeader(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes);
        void _OnHeaderLoadingFailed(const std::string& i_description);
        
        void _ProcessLoadedData(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes);
        void _OnDataLoadingFailed(const std::string& i_description);
        
        void _ProcessDecodedData(TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes);
        void _OnDataDecodingFailed(const std::string& i_description);
        
        template<class TPixelType>
        typename itk::Image<TPixelType, 3>::Pointer _BuildImage(const Header& i_header, TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes) const;
        
      private:
        std::unique_ptr<Header> mp_header;
        
        TOnReadyCallback m_on_ready_callback;
        TOnFailedCallback m_on_failed_callback;
        TStatusCallback m_status_callback;
        std::string m_data_url;
        
        std::unique_ptr<IDataDestination> mp_header_loading_processor;
        std::unique_ptr<IAsyncDataLoader> mp_header_loader;
        
        std::unique_ptr<IDataDestination> mp_data_loading_processor;
        std::unique_ptr<IAsyncDataLoader> mp_data_loader;
        
        std::unique_ptr<IDataDestination> mp_data_decoding_processor;
        std::unique_ptr<IDataDecoder> mp_data_decoder;
        
        std::unique_ptr<IHeaderParser> mp_header_parser;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class ImageStackBuilder::_DataDestinationProxy : public IDataDestination
      {
      public:
        typedef void(ImageStackBuilder::*TMallocPtrProcessor)(TUniqueMallocPtr&&, unsigned);
        typedef void(ImageStackBuilder::*TBufferPtrProcessor)(TUniqueBufferPtr&&, unsigned);
        typedef void(ImageStackBuilder::*TOnFailed)(const std::string&);
        
      public:
        _DataDestinationProxy(
          ImageStackBuilder& ir_parent,
          TMallocPtrProcessor ip_malloc_ptr_processor,
          TBufferPtrProcessor ip_buffer_ptr_processor,
          TOnFailed ip_on_failed);
        
        // IDataDestination
        virtual void OnSuccess(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes) override;
        virtual void OnSuccess(TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes) override;
        virtual void OnFailure(const std::string& i_description) override;
        
      private:
        ImageStackBuilder& mr_parent;
        TMallocPtrProcessor mp_malloc_ptr_processor;
        TBufferPtrProcessor mp_buffer_ptr_processor;
        TOnFailed mp_on_failed;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif