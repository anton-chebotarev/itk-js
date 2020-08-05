#include "ImageStackBuilder.h"

#include "WgetAsyncDataLoader.h"
#include "HEVCDataDecoder.h"
#include "HeaderParser.h"
#include "Header.h"
#include "ImageStack.h"

#include <itkImportImageFilter.h>

#include <sstream>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    ImageStackBuilder::ImageStackBuilder()
      : mp_data_decoder(new HEVCDataDecoder())
      , mp_header_parser(new HeaderParser())
      {
      mp_header_processor.reset(new _DataDestinationProxy(*this, &ImageStackBuilder::_ProcessLoadedHeader, &ImageStackBuilder::_OnHeaderLoadingFailed));
      mp_data_processor.reset(new _DataDestinationProxy(*this, &ImageStackBuilder::_ProcessLoadedData, &ImageStackBuilder::_OnDataLoadingFailed));
      mp_header_loader.reset(new WgetAsyncDataLoader(*mp_header_processor));
      mp_data_loader.reset(new WgetAsyncDataLoader(*mp_data_processor));
      }
          
    void ImageStackBuilder::OnLoadingProgress(TOnLoadingProgressCallback i_callback)
      {
      class _LoadingProgressReporterProxy : public IAsyncDataLoader::ILoadingProgressReporter
        {          
        public:
          explicit _LoadingProgressReporterProxy(TOnLoadingProgressCallback i_callback)
            : m_callback(i_callback)
            {
            }
          virtual void OnLoadingProgress(int i_id, unsigned i_bytes_loaded, unsigned i_bytes_total) const override
            {
            m_callback(i_id, i_bytes_loaded, i_bytes_total);
            }          
        private:
          TOnLoadingProgressCallback m_callback;
        };
      mp_header_loader->SetProgressReporter(std::unique_ptr<IAsyncDataLoader::ILoadingProgressReporter>(new _LoadingProgressReporterProxy(i_callback)));
      mp_data_loader->SetProgressReporter(std::unique_ptr<IAsyncDataLoader::ILoadingProgressReporter>(new _LoadingProgressReporterProxy(i_callback)));  
      }
    
    void ImageStackBuilder::OnDecodingProgress(TOnDecodingProgressCallback i_callback)
      {
      class _DecodingProgressReporterProxy : public IDataDecoder::IDecodingProgressReporter
        {          
        public:
          explicit _DecodingProgressReporterProxy(TOnDecodingProgressCallback i_callback)
            : m_callback(i_callback)
            {
            }
          virtual void OnDecodingProgress(unsigned i_bytes_loaded, unsigned i_bytes_total) const override
            {
            m_callback(i_bytes_loaded, i_bytes_total);
            }          
        private:
          TOnDecodingProgressCallback m_callback;
        };
      mp_data_decoder->SetProgressReporter(std::unique_ptr<IDataDecoder::IDecodingProgressReporter>(new _DecodingProgressReporterProxy(i_callback)));        
      }
        
    void ImageStackBuilder::LoadDataAsync(
          const std::string& i_header_url,
          const std::string& i_data_url,
          TOnReadyCallback i_on_ready_callback,
          TOnFailedCallback i_on_failed_callback,
          TStatusCallback i_status_callback)
      {
      m_on_ready_callback = i_on_ready_callback;
      m_on_failed_callback = i_on_failed_callback;
      m_status_callback = i_status_callback;
      m_data_url = i_data_url;
      
      m_status_callback("Loading header ...");
      mp_header_loader->LoadDataAsync(i_header_url);
      }
      
    void ImageStackBuilder::_ProcessLoadedHeader(void* ip_buffer, unsigned i_buffer_size)
      {
      try
        {
        m_status_callback("Parsing header ...");
        
        mp_header = mp_header_parser->ParseHeader(ip_buffer, i_buffer_size);

        m_status_callback((std::ostringstream() << "Expecting " << mp_header->dimensions[2] << " frames of " << mp_header->dimensions[0] << "x" << mp_header->dimensions[1] << " pixels size (" << mp_header->component_size*8 << " bpp)").str().c_str());
      
        m_status_callback("Loading data ...");
        mp_data_loader->LoadDataAsync(m_data_url);
        }
      catch (const std::exception& i_e)
        {
        m_on_failed_callback(i_e.what());
        }
      catch (...)
        {
        m_on_failed_callback("Exception during header parsing.");
        }
      }
      
    void ImageStackBuilder::_OnHeaderLoadingFailed(const char* ip_description)
      {
      m_on_failed_callback(ip_description);  
      }
      
    template<class TPixelType>
    typename itk::Image<TPixelType, 3>::Pointer ImageStackBuilder::_BuildImage(const Header& i_header, std::unique_ptr<uint8_t[]>&& ip_image_buffer) const
      {
      typedef itk::Image<TPixelType, 3> TImage;
      typedef itk::ImportImageFilter<TPixelType, 3> TImportImageFilter;
      
      std::unique_ptr<TPixelType[]> p_buffer(reinterpret_cast<TPixelType*>(ip_image_buffer.release()));
      
      typename TImportImageFilter::Pointer p_import_image_filter = TImportImageFilter::New();
      
      typename TImage::IndexType start;
      start.Fill(0);
      typename TImage::SizeType size;
      size[0] = i_header.dimensions[0];
      size[1] = i_header.dimensions[1];
      size[2] = i_header.dimensions[2];
      typename TImage::RegionType region;
      region.SetIndex(start);
      region.SetSize(size);      
      p_import_image_filter->SetRegion(region);
      
      itk::SpacePrecisionType spacing[3];
      spacing[0] = i_header.spacing[0];
      spacing[1] = i_header.spacing[1];
      spacing[2] = i_header.spacing[2];
      p_import_image_filter->SetSpacing(spacing);
      
      itk::SpacePrecisionType origin[3];
      origin[0] = i_header.origin[0];
      origin[1] = i_header.origin[1];
      origin[2] = i_header.origin[2];
      p_import_image_filter->SetOrigin(origin);
      
      typename TImportImageFilter::DirectionType direction;
      for ( unsigned int r = 0; r < 3; r++ )
        for ( unsigned int c = 0; c < 3; c++ )
          direction[r][c] = i_header.direction[r][c];
      p_import_image_filter->SetDirection(direction);
           
      p_import_image_filter->SetImportPointer(
        p_buffer.release(),
        size[0] * size[1] * size[2],
        true); // owns buffer
        
      p_import_image_filter->Update();
        
      return p_import_image_filter->GetOutput();
      }
      
    void ImageStackBuilder::_ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size)
      {
      try
        {
        m_status_callback("Decoding data ...");
        
        if (mp_header.get() == nullptr)
          throw std::logic_error("Header expected to be parsed before data decoding");
        
        std::unique_ptr<uint8_t[]> p_frame_buffer(new uint8_t[mp_header->dimensions[0] * mp_header->dimensions[1] * mp_header->dimensions[2] * mp_header->component_size]);
          
        mp_data_decoder->DecodeData(
          reinterpret_cast<void*>(p_frame_buffer.get()),
          mp_header->dimensions[0],
          mp_header->dimensions[1],
          mp_header->dimensions[2],
          mp_header->component_size,
          ip_buffer,
          i_buffer_size);
          
        m_status_callback("Building Image Stack ...");
        
        ImageStack::_TImageStack::Pointer p_image_stack = _BuildImage<ImageStack::_TImageStack::PixelType>(*mp_header, std::move(p_frame_buffer));
        
        ImageStack::_TImageStack::SizeType size = p_image_stack->GetBufferedRegion().GetSize();
        m_status_callback((std::ostringstream() << size[0] << "x" << size[1] << "x" << size[2] << " 16bit image stack is ready").str().c_str());
          
        m_on_ready_callback(std::unique_ptr<ImageStack>(new ImageStack(p_image_stack)));
        }
      catch (const std::exception& i_e)
        {
        m_on_failed_callback(i_e.what());
        }
      catch (...)
        {
        m_on_failed_callback("Exception during data decoding/image stack building.");
        }
      }
      
    void ImageStackBuilder::_OnDataLoadingFailed(const char* ip_description)
      {
      m_on_failed_callback(ip_description);  
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    ImageStackBuilder::_DataDestinationProxy::_DataDestinationProxy(ImageStackBuilder& ir_parent, TProcessor ip_processor, TOnFailed ip_on_failed)
      : mr_parent(ir_parent)
      , mp_processor(ip_processor)
      , mp_on_failed(ip_on_failed)
      {
      }
    
    void ImageStackBuilder::_DataDestinationProxy::ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size)
      {
      (mr_parent.*mp_processor)(ip_buffer, i_buffer_size);
      }
      
    void ImageStackBuilder::_DataDestinationProxy::OnDataLoadingFailed(const char* ip_description)
      {
      (mr_parent.*mp_on_failed)(ip_description);
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }