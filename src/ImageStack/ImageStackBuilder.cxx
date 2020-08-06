#include "ImageStackBuilder.h"

#include "WgetAsyncDataLoader.h"
#include "HEVCAsyncDataDecoder.h"
#include "HeaderParser.h"
#include "Header.h"
#include "ImageStack.h"
#include "ProxyingProgressReporter.h"

#include <itkImportImageFilter.h>

#include <sstream>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    ImageStackBuilder::ImageStackBuilder()
      : mp_header_parser(new HeaderParser())
      {
      mp_header_loading_processor.reset(new _DataDestinationProxy(*this, &ImageStackBuilder::_ProcessLoadedHeader, nullptr, &ImageStackBuilder::_OnHeaderLoadingFailed));
      mp_header_loader.reset(new WgetAsyncDataLoader(*mp_header_loading_processor));
      
      mp_data_loading_processor.reset(new _DataDestinationProxy(*this, &ImageStackBuilder::_ProcessLoadedData, nullptr, &ImageStackBuilder::_OnDataLoadingFailed));
      mp_data_loader.reset(new WgetAsyncDataLoader(*mp_data_loading_processor));
      
      mp_data_decoding_processor.reset(new _DataDestinationProxy(*this, nullptr, &ImageStackBuilder::_ProcessDecodedData, &ImageStackBuilder::_OnDataDecodingFailed));  
      mp_data_decoder.reset(new HEVCAsyncDataDecoder(*mp_data_decoding_processor));
      }
      
      
    void ImageStackBuilder::OnHeaderLoadingProgress(TOnProgressCallback i_callback)
      {
      mp_header_loader->SetProgressReporter(std::unique_ptr<IProgressReporter>(new ProxyingProgressReporter(i_callback)));       
      }
      
    void ImageStackBuilder::OnDataLoadingProgress(TOnProgressCallback i_callback)
      {
      mp_data_loader->SetProgressReporter(std::unique_ptr<IProgressReporter>(new ProxyingProgressReporter(i_callback)));
      }
      
    void ImageStackBuilder::OnDataDecodingProgress(TOnProgressCallback i_callback)
      {
      mp_data_decoder->SetProgressReporter(std::unique_ptr<IProgressReporter>(new ProxyingProgressReporter(i_callback)));        
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
      
    template<class TPixelType>
    typename itk::Image<TPixelType, 3>::Pointer ImageStackBuilder::_BuildImage(const Header& i_header, TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes) const
      {
      if (i_size_in_bytes != (i_header.dimensions[0] * i_header.dimensions[1] * i_header.dimensions[2] * i_header.component_size))
        throw std::logic_error("Invalid decoded data buffer size");
        
      typedef itk::Image<TPixelType, 3> TImage;
      typedef itk::ImportImageFilter<TPixelType, 3> TImportImageFilter;
      
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
        reinterpret_cast<TPixelType*>(ip_buffer.release()),
        size[0] * size[1] * size[2],
        true); // owns buffer
        
      p_import_image_filter->Update();
        
      return p_import_image_filter->GetOutput();
      }
      
    void ImageStackBuilder::_ProcessLoadedHeader(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      try
        {
        m_status_callback("Parsing header ...");
        
        mp_header = mp_header_parser->ParseHeader(ip_buffer.get(), i_size_in_bytes);

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
      
    void ImageStackBuilder::_OnHeaderLoadingFailed(const std::string& i_description)
      {
      m_on_failed_callback(i_description);  
      }
      
    void ImageStackBuilder::_ProcessLoadedData(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      try
        {
        m_status_callback("Decoding data ...");
        
        if (mp_header.get() == nullptr)
          throw std::logic_error("Header expected to be parsed before data decoding");

        mp_data_decoder->DecodeData(*mp_header, ip_buffer.get(), i_size_in_bytes);
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
      
    void ImageStackBuilder::_OnDataLoadingFailed(const std::string& i_description)
      {
      m_on_failed_callback(i_description);  
      }
    
    void ImageStackBuilder::_ProcessDecodedData(TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      try
        {
        m_status_callback("Building Image Stack ...");
        
        ImageStack::_TImageStack::Pointer p_image_stack = _BuildImage<ImageStack::_TImageStack::PixelType>(*mp_header, std::move(ip_buffer), i_size_in_bytes);
        
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
        m_on_failed_callback("Exception during header parsing.");
        }
      }
    
    void ImageStackBuilder::_OnDataDecodingFailed(const std::string& i_description)
      {
      m_on_failed_callback(i_description);
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    ImageStackBuilder::_DataDestinationProxy::_DataDestinationProxy(
          ImageStackBuilder& ir_parent,
          TMallocPtrProcessor ip_malloc_ptr_processor,
          TBufferPtrProcessor ip_buffer_ptr_processor,
          TOnFailed ip_on_failed)
      : mr_parent(ir_parent)
      , mp_malloc_ptr_processor(ip_malloc_ptr_processor)
      , mp_buffer_ptr_processor(ip_buffer_ptr_processor)
      , mp_on_failed(ip_on_failed)
      {
      }
    
    void ImageStackBuilder::_DataDestinationProxy::OnSuccess(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      if (!mp_malloc_ptr_processor || mp_buffer_ptr_processor)
        throw std::logic_error("Buffer memory allocator mismatch");
      (mr_parent.*mp_malloc_ptr_processor)(std::move(ip_buffer), i_size_in_bytes);
      }
    
    void ImageStackBuilder::_DataDestinationProxy::OnSuccess(TUniqueBufferPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      if (!mp_buffer_ptr_processor || mp_malloc_ptr_processor)
        throw std::logic_error("Buffer memory allocator mismatch");
      (mr_parent.*mp_buffer_ptr_processor)(std::move(ip_buffer), i_size_in_bytes);
      }
      
    void ImageStackBuilder::_DataDestinationProxy::OnFailure(const std::string& i_description)
      {
      (mr_parent.*mp_on_failed)(i_description);
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }