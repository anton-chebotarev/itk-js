#include "HeaderParser.h"

#include "Header.h"

#include <string>
#include <sstream>
#include <vector>

namespace
  {
  unsigned _GetComponentSize(const std::string& i_component_type_str)
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
  }  

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    std::unique_ptr<Header> HeaderParser::ParseHeader(
      void* ip_data_buffer,
      unsigned i_size_in_bytes) const
      {
      std::unique_ptr<Header> p_header(new Header());
      
      std::string header_str(reinterpret_cast<char*>(ip_data_buffer), i_size_in_bytes);        
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
                  unsigned i = 0;
                  for (unsigned r = 0; r < 3; ++r)
                    for (unsigned c = 0; c < 3; ++c)
                      p_header->direction[r][c] = atof(tokens[i++].c_str());
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
      
      return p_header;
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }