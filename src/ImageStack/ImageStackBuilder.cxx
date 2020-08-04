#include "ImageStackBuilder.h"

#include "WgetAsyncDataLoader.h"
#include "HEVCDataDecoder.h"

#include <vector>
#include <sstream>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    ImageStackBuilder::ImageStackBuilder()
      : mp_data_decoder(new HEVCDataDecoder())
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
      
    unsigned ImageStackBuilder::_GetComponentSize(const std::string& i_component_type_str) const
      {
      if (i_component_type_str == "unsigned_char")
        return sizeof(unsigned char);
      if (i_component_type_str == "char")
        return sizeof(char);
      if (i_component_type_str == "unsigned_short")
        return sizeof(unsigned short);
      if (i_component_type_str == "short")
        return sizeof(short);
      throw std::runtime_error("Unsupported component type");
      }
      
    void ImageStackBuilder::_ProcessLoadedHeader(void* ip_buffer, unsigned i_buffer_size)
      {
      try
        {
        m_status_callback("Parsing header ...");
        
        std::unique_ptr<_Header> p_header(new _Header());
        
        std::string header_str(reinterpret_cast<char*>(ip_buffer), i_buffer_size);        
        std::istringstream ss(header_str);

        std::string line;
        while (std::getline(ss, line))
          {
          if (line.length() != 0 && line[0] != '#')
            {
            size_t pos = line.find(':');
            if (pos != -1 && pos != line.length() - 1)
              {
              std::string key = line.substr(0, pos);
              std::string val = line.substr(pos + 1, line.length() - pos - 1);
              if (val[0] == '=')
                {
                continue; // metadata
                }
              else if (val[0] == ' ')
                {
                std::istringstream ss(val.substr(1, val.length() - 1));
                std::vector<std::string> tokens;
                std::string token;
                while (std::getline(ss, token, ' '))
                  {
                  tokens.push_back(token);
                  }
                if (key == "component type")
                  {
                  p_header->component_size = _GetComponentSize(tokens[0]);
                  }
                else if (key == "dimensions")
                  {
                  if (tokens.size() == 3)
                    {
                    p_header->dimensions[0] = atoi(tokens[0].c_str());
                    p_header->dimensions[1] = atoi(tokens[1].c_str());
                    p_header->dimensions[2] = atoi(tokens[2].c_str());
                    }
                  }
                else if (key == "spacing")
                  {
                  if (tokens.size() == 3)
                    {
                    p_header->spacing[0] = atof(tokens[0].c_str());
                    p_header->spacing[1] = atof(tokens[1].c_str());
                    p_header->spacing[2] = atof(tokens[2].c_str());
                    }
                  }
                else if (key == "origin")
                  {
                  if (tokens.size() == 3)
                    {
                    p_header->origin[0] = atof(tokens[0].c_str());
                    p_header->origin[1] = atof(tokens[1].c_str());
                    p_header->origin[2] = atof(tokens[2].c_str());
                    }
                  }
                else if (key == "directions")
                  {
                  if (tokens.size() == 9)
                    {
                    p_header->directions[0] = atof(tokens[0].c_str());
                    p_header->directions[1] = atof(tokens[1].c_str());
                    p_header->directions[2] = atof(tokens[2].c_str());
                    }
                  }
                else if (key == "data file")
                  {
                  continue;
                  }
                }
              }
            }
          }
        
        mp_header = std::move(p_header);
      
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
      
    void ImageStackBuilder::_ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size)
      {
      try
        {
        m_status_callback("Decoding data ...");
        
        if (mp_header.get() == nullptr)
          throw std::logic_error("Header expected to be parsed before data decoding");
        
        std::vector<uint8_t> frame_buffer(mp_header->dimensions[0] * mp_header->dimensions[1] * mp_header->dimensions[2] * mp_header->component_size);
          
        mp_data_decoder->DecodeData(
          reinterpret_cast<void*>(&frame_buffer[0]),
          mp_header->dimensions[0],
          mp_header->dimensions[1],
          mp_header->dimensions[2],
          mp_header->component_size,
          ip_buffer,
          i_buffer_size);
          
        m_on_ready_callback();
        }
      catch (const std::exception& i_e)
        {
        m_on_failed_callback(i_e.what());
        }
      catch (...)
        {
        m_on_failed_callback("Exception during data decoding.");
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