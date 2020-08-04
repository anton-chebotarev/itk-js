/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkFFmpegImageIO.h"

#include <itk/itkMetaDataObject.h>
#include <itk/itkIOCommon.h>
#include <itk/itkFloatingPointExceptions.h>
#include <itk/itksys/SystemTools.hxx>

#include <iomanip>

#include <wtypes.h>

namespace itk
  {

  FFmpegImageIO::FFmpegImageIO()
    : m_encoding(Encoding::h265)
    {
    this->SetNumberOfDimensions(3);

    const char * extensions[] = { ".ihdr" };

    for (auto ext : extensions)
      {
      this->AddSupportedWriteExtension(ext);
      }
    }

  FFmpegImageIO::~FFmpegImageIO() = default;

  void FFmpegImageIO::SetFFmpegPath(const std::string& i_path)
    {
    m_ffmpegPath = i_path;
    }

  void FFmpegImageIO::SetEncoding(Encoding i_enc)
    {
    if (i_enc == Encoding::custom_encoding_command_line)
      throw std::logic_error("Custom command line must be set in dedicated method");
    m_encoding = i_enc;
    }

  void FFmpegImageIO::SetCustomEncodingCommandLine(const std::string& i_cmd, const std::string& i_datafile_extension/* = ".mp4"*/)
    {
    m_encoding = Encoding::custom_encoding_command_line;
    m_custom_encoding_command_line = i_cmd;
    m_custom_datafile_extension = i_datafile_extension;
    }

  bool FFmpegImageIO::SupportsDimension(unsigned long dim)
    {
    return dim == 3;
    }

  void FFmpegImageIO::PrintSelf(std::ostream & os, Indent indent) const
    {
    Superclass::PrintSelf(os, indent);
    }

  bool FFmpegImageIO::_HasSupportedReadExtension(const char * fileName, bool ignoreCase) const
    {
    return this->_HasSupportedExtension(fileName, this->GetSupportedReadExtensions(), ignoreCase);
    }

  bool FFmpegImageIO::_HasSupportedWriteExtension(const char * fileName, bool ignoreCase) const
    {
    return this->_HasSupportedExtension(fileName, this->GetSupportedWriteExtensions(), ignoreCase);
    }

  bool FFmpegImageIO::_HasSupportedExtension(
    const char *                               filename,
    const ImageIOBase::ArrayOfExtensionsType & supportedExtensions,
    bool                                       ignoreCase) const
    {
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    if (ignoreCase)
      {
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      }

    for (auto && candidate : supportedExtensions)
      {
      if (ignoreCase)
        {
        size_t n = candidate.size();
        if (n == ext.size() && n > 0)
          {

          while (true)
            {
            --n;
            if (ext[n] != ::tolower(candidate[n]))
              {
              break;
              }
            if (n == 0)
              {
              return true;
              }
            }
          }
        }
      else
        {
        if (candidate == ext)
          {
          return true;
          }
        }
      }
    return false;
    }

  void FFmpegImageIO::_SetDatafileName(std::string filename)
    {
    m_datafile_name = filename;
    }

  std::string FFmpegImageIO::_GetDatafileName() const
    {
    if (m_datafile_name.empty())
      {
      std::string filename = itksys::SystemTools::GetFilenameWithoutLastExtension(this->GetFileName());

      m_datafile_name = (filename + _GetDatafileExtension());
      }
    return m_datafile_name;
    }

  std::string FFmpegImageIO::_GetFFmpegPath() const
    {
    return m_ffmpegPath;
    }

  std::string FFmpegImageIO::_GetDatafilePath() const
    {
    return itksys::SystemTools::GetParentDirectory(this->GetFileName()) + _GetDatafileName();
    }

  std::string FFmpegImageIO::_GetDatafileExtension() const
    {
    switch (m_encoding)
      {
      case Encoding::custom_encoding_command_line:
        return m_custom_datafile_extension;
      case Encoding::ffv1:
        return ".nut";
      case Encoding::h264:
        return ".mp4";
      case Encoding::h265:
        return ".hevc";
      default:
        return ".mp4";
      }
    }

  std::string FFmpegImageIO::_GetFFmpegDecodingPexelFormatStr() const
    {
    std::stringstream ss;

    ss << "gray";

    if (this->GetComponentSize() == 2)
      {
      switch (this->GetByteOrder())
        {
        case BigEndian:
          ss << "16be";
          break;
        default:
          ss << "16le";
          break;
        }
      }

    return ss.str();
    }


  std::string FFmpegImageIO::_GetFFmpegEncodingParametersStr() const
    {
    std::stringstream ss;
    switch (m_encoding)
      {
      case Encoding::custom_encoding_command_line:
        return m_custom_encoding_command_line;
      case Encoding::ffv1:
        ss << "-c:v ffv1";
        if (this->GetPixelSize() == 2)
          ss << " -pix_fmt gray16le";
        return ss.str();
      case Encoding::h264:
        ss << "-c:v libx264 -crf 0";
        if (this->GetPixelSize() == 2)
          throw std::runtime_error("Unsupported Pixel Size"); //ss << " -pix_fmt gray10le";
        return ss.str();
      case Encoding::h265:
        ss << "-c:v libx265 -x265-params lossless=1";
        if (this->GetPixelSize() == 2)
          ss << " -pix_fmt gray12le";
        return ss.str();
      }
    throw std::logic_error("Unknown encoding");
    }

  void FFmpegImageIO::_SerializeMeta(std::ostream& o_stream) const
    {
    o_stream << std::setprecision(20);
    o_stream << "# Created by itkFFmpegImageIO using FFmpeg video converter";
    o_stream << std::endl << "component type: " << this->GetComponentTypeAsString(this->GetComponentType());
    o_stream << std::endl << "dimensions: " << this->GetDimensions(0) << " " << this->GetDimensions(1) << " " << this->GetDimensions(2);
    o_stream << std::endl << "spacing: " << this->GetSpacing(0) << " " << this->GetSpacing(1) << " " << this->GetSpacing(2);
    o_stream << std::endl << "origin: " << this->GetOrigin(0) << " " << this->GetOrigin(1) << " " << this->GetOrigin(2);
    std::vector<double> dir0 = this->GetDirection(0);
    std::vector<double> dir1 = this->GetDirection(1);
    std::vector<double> dir2 = this->GetDirection(2);
    o_stream << std::endl << "directions: "
      << dir0[0] << " " << dir0[1] << " " << dir0[2] << " "
      << dir1[0] << " " << dir1[1] << " " << dir1[2] << " "
      << dir2[0] << " " << dir2[1] << " " << dir2[2];

    o_stream << std::endl << "data file: " << _GetDatafileName();
    o_stream << std::endl << "encoding parameters: " << _GetFFmpegEncodingParametersStr();

    // Go through MetaDataDictionary and get key/value pairs
    const MetaDataDictionary&                thisDic = this->GetMetaDataDictionary();
    std::vector<std::string>                 keys = thisDic.GetKeys();
    std::vector<std::string>::const_iterator keyIt;
    for (keyIt = keys.begin(); keyIt != keys.end(); ++keyIt)
      {
      const std::string& key = *keyIt;
      if (key.find("ITK_") == 0)
        continue;
      if (key.find("NRRD_") == 0)
        continue;
      std::string value;
      ExposeMetaData<std::string>(thisDic, key, value);
      o_stream << std::endl << key << ":=" << value;
      }
    }

  void FFmpegImageIO::_ParseMeta(std::istream& i_stream)
    {
    this->SetByteOrder(ImageIOBase::LittleEndian);
    this->SetFileTypeToBinary();
    this->SetNumberOfDimensions(3);
    this->SetPixelType(ImageIOBase::SCALAR);
    this->SetNumberOfComponents(1);

    MetaDataDictionary & thisDic = this->GetMetaDataDictionary();
    // Necessary to clear dict if ImageIO object is re-used
    thisDic.Clear();

    std::string line;
    while (std::getline(i_stream, line))
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
            EncapsulateMetaData<std::string>(thisDic, key, val.substr(1, val.length() - 1));
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
              this->SetComponentType(this->GetComponentTypeFromString(tokens[0]));
              }
            else if (key == "dimensions")
              {
              if (tokens.size() == 3)
                {
                this->SetDimensions(0, atoi(tokens[0].c_str()));
                this->SetDimensions(1, atoi(tokens[1].c_str()));
                this->SetDimensions(2, atoi(tokens[2].c_str()));
                }
              }
            else if (key == "spacing")
              {
              if (tokens.size() == 3)
                {
                this->SetSpacing(0, atof(tokens[0].c_str()));
                this->SetSpacing(1, atof(tokens[1].c_str()));
                this->SetSpacing(2, atof(tokens[2].c_str()));
                }
              }
            else if (key == "origin")
              {
              if (tokens.size() == 3)
                {
                this->SetOrigin(0, atof(tokens[0].c_str()));
                this->SetOrigin(1, atof(tokens[1].c_str()));
                this->SetOrigin(2, atof(tokens[2].c_str()));
                }
              }
            else if (key == "directions")
              {
              if (tokens.size() == 9)
                {
                this->SetDirection(0, std::vector<double>{ atof(tokens[0].c_str()), atof(tokens[1].c_str()), atof(tokens[2].c_str()) });
                this->SetDirection(1, std::vector<double>{ atof(tokens[3].c_str()), atof(tokens[4].c_str()), atof(tokens[5].c_str()) });
                this->SetDirection(2, std::vector<double>{ atof(tokens[6].c_str()), atof(tokens[7].c_str()), atof(tokens[8].c_str()) });
                }
              }
            else if (key == "data file")
              {
              _SetDatafileName(tokens[0]);
              }
            }
          }
        }
      }
    }

  bool FFmpegImageIO::CanReadFile(const char * filename)
    {
    // Check the extension first to avoid opening of wrong files.
    // The file must have an appropriate extension to be recognized.
    std::string fname = filename;

    bool extensionFound = this->_HasSupportedReadExtension(filename, true);

    if (!extensionFound)
      {
      itkDebugMacro(<< "The filename extension is not recognized");
      return false;
      }

    // We have the correct extension, so now check for the JP3D magic bytes
    std::ifstream inputStream;
    try
      {
      this->OpenFileForReading(inputStream, fname);
      }
    catch (ExceptionObject &)
      {
      return false;
      }

    std::string line;
    if (!std::getline(inputStream, line))
      {
      return false;
      }

    return (line[0] == '#');
    }

  bool FFmpegImageIO::CanWriteFile(const char * name)
    {
    std::string filename = name;
    if (filename.empty())
      {
      return false;
      }
    return this->_HasSupportedWriteExtension(name, true);
    }

  void FFmpegImageIO::ReadImageInformation()
    {
    std::ifstream inputStream;
    this->OpenFileForReading(inputStream, this->GetFileName());
    _ParseMeta(inputStream);
    }

  void FFmpegImageIO::WriteImageInformation()
    {
    std::ofstream outputStream;
    this->OpenFileForWriting(outputStream, this->GetFileName());
    _SerializeMeta(outputStream);
    }

  void FFmpegImageIO::Read(void * buffer)
    {
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    HANDLE readPipe, writePipe;
    CreatePipe(&readPipe, &writePipe, &sa, 0);

    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = writePipe;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {};

    std::wostringstream stringBuidler;
    stringBuidler
      << _GetFFmpegPath().c_str()
      << " -y"
      << " -i " << _GetDatafilePath().c_str()
      << " -pix_fmt " << _GetFFmpegDecodingPexelFormatStr().c_str()
      << " -f rawvideo pipe:";
    auto cmd = stringBuidler.str();

    CreateProcess(nullptr, const_cast<wchar_t*>(cmd.c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

    for (void* p_buffer = buffer; ;)
      {
      DWORD bytes_read = 0;
      if (!ReadFile(readPipe, p_buffer, 4096, &bytes_read, nullptr))
        break;
      p_buffer = reinterpret_cast<void*>(reinterpret_cast<char*>(p_buffer) + bytes_read);      
      }

    CloseHandle(readPipe);
    CloseHandle(writePipe);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    }

  void FFmpegImageIO::Write(const void * buffer)
    {
    if (this->GetComponentSize() > 2)
      throw std::runtime_error("Unsupported Component Size");

    if (this->GetPixelType() != SCALAR)
      throw std::runtime_error("Unsupported Pixel Type");

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    HANDLE readPipe, writePipe;
    CreatePipe(&readPipe, &writePipe, &sa, 0);

    SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = readPipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {};

    std::wostringstream stringBuidler;
    stringBuidler
      << _GetFFmpegPath().c_str()
      << " -y"
      << " -f rawvideo"
      << " -s " << this->GetDimensions(0) << "x" << this->GetDimensions(1)
      << " -r 1"
      << " -pix_fmt " << _GetFFmpegDecodingPexelFormatStr().c_str()
      << " -i pipe:"
      << " " << _GetFFmpegEncodingParametersStr().c_str()
      << " " << _GetDatafilePath().c_str();
    auto cmd = stringBuidler.str();

    CreateProcess(nullptr, const_cast<wchar_t*>(cmd.c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

    size_t sz = this->GetDimensions(0) * this->GetDimensions(1) * this->GetDimensions(2) * this->GetComponentSize();
    WriteFile(writePipe, buffer, sz, nullptr, nullptr);

    CloseHandle(readPipe);
    CloseHandle(writePipe);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    WriteImageInformation();
    }

  }