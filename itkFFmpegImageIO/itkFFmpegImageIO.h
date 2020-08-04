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
#ifndef itkFFmpegImageIO_h
#define itkFFmpegImageIO_h
 //#include "ITKIOJP3DExport.h"


#include <itk/itkImageIOBase.h>
#include <fstream>

namespace itk
  {
  /** \class FFmpegImageIO
   *
   * \brief Read and write the images as video stream. Metadata is serialized separately.
   *
   * \ingroup IOFilters
   * \ingroup ITKIOFFmpeg
   */
  class /*ITKIOJP3D_EXPORT*/ FFmpegImageIO :public ImageIOBase
    {
    public:
      /** Standard class typedefs. */
      typedef FFmpegImageIO          Self;
      typedef ImageIOBase          Superclass;
      typedef SmartPointer< Self > Pointer;

      /** Method for creation through the object factory. */
      itkNewMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(FFmpegImageIO, ImageIOBase);

      /** The different types of ImageIO's can support data of varying
       * dimensionality. For example, some file formats are strictly 2D
       * while others can support 2D, 3D, or even n-D. This method returns
       * true/false as to whether the ImageIO can support the dimension
       * indicated. */
      virtual bool SupportsDimension(unsigned long) ITK_OVERRIDE;

      /** Determine the file type. Returns true if this ImageIO can read the
       * file specified. */
      virtual bool CanReadFile(const char *) ITK_OVERRIDE;

      /** Set the spacing and dimension information for the set filename. */
      virtual void ReadImageInformation() ITK_OVERRIDE;

      /** Reads the data from disk into the memory buffer provided. */
      virtual void Read(void *buffer) ITK_OVERRIDE;

      /** Determine the file type. Returns true if this ImageIO can write the
       * file specified. */
      virtual bool CanWriteFile(const char *) ITK_OVERRIDE;

      /** Set the spacing and dimension information for the set filename. */
      virtual void WriteImageInformation() ITK_OVERRIDE;

      /** Writes the data to disk from the memory buffer provided. Make sure
       * that the IORegions has been set properly. */
      virtual void Write(const void *buffer) ITK_OVERRIDE;

    public:
      enum class Encoding : unsigned char
        {
        custom_encoding_command_line,
        ffv1,
        h264,
        h265
        };

      void SetFFmpegPath(const std::string& i_path);
      void SetEncoding(Encoding i_enc);
      void SetCustomEncodingCommandLine(const std::string& i_cmd, const std::string& i_datafile_extension = ".mp4");

    protected:
      FFmpegImageIO();
      ~FFmpegImageIO() ITK_OVERRIDE;
      virtual void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

    private:
      ITK_DISALLOW_COPY_AND_ASSIGN(FFmpegImageIO);

      std::string _GetFFmpegPath() const;

      void _SerializeMeta(std::ostream& o_stream) const;
      void _ParseMeta(std::istream& i_stream);

      bool _HasSupportedReadExtension(const char * fileName, bool ignoreCase) const;
      bool _HasSupportedWriteExtension(const char * fileName, bool ignoreCase) const;
      bool _HasSupportedExtension(
        const char *                               filename,
        const ImageIOBase::ArrayOfExtensionsType & supportedExtensions,
        bool                                       ignoreCase) const;

      std::string _GetFFmpegDecodingPexelFormatStr() const;
      std::string _GetFFmpegEncodingParametersStr() const;

      std::string _GetDatafileExtension() const;
      void _SetDatafileName(std::string filename);
      std::string _GetDatafileName() const;
      std::string _GetDatafilePath() const;

    private:
      mutable std::string m_datafile_name;
      std::string m_ffmpegPath;
      Encoding m_encoding;
      std::string m_custom_datafile_extension;
      std::string m_custom_encoding_command_line;
    };
  } // end namespace itk

#endif // itkFFmpegImageIO_h
