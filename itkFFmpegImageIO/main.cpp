#include <iostream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNrrdImageIO.h>
#include <itkImageFileWriter.h>
#include "itk/itkFFmpegImageIO.h"

//-----------------------------------------------------------------------------------
int wmain(int argc, wchar_t* argv[])
  {
  namespace po = boost::program_options;

  po::options_description desc("Allowed parameters");
  desc.add_options()
    ("ffmpeg-path,f", po::value<std::string>()->required(), "[Optional] Path to ffmpeg.exe.")
    ("nrrd-file,n"  , po::value<std::string>()->required(), "NRRD file.")
    ("ihdr-file,i", po::value<std::string>()->required(), "IHDR file.")
    ("working-mode,m", po::value<std::string>()->required(), "Working mode [encode|decode]. Encode means NRRD->(Video+IHDR). Decode means (Video+IHDR)->NRRD.")
    ("use-compression,c", "[Optional] Indicates that compression should be used during encoding if applicable.")
    ("ffmpeg-encoder-parameters,p", po::value<std::string>()->default_value(""), "[Optional] Custom command line options for FFmpeg encoder. Empty by default.")
    ("ffmpeg-encoder,e", po::value<std::string>()->default_value("h265"), "[Optional] FFmpeg encoder. Default is [ffv1]. Possible options are [ffv1|h264|h265]. ffv1 has 16bit support. h264 has 10bit support. h265 has 12bit support.")
    ;

  try
    {
    po::variables_map vm;
    auto opts = po::parse_command_line(argc, argv, desc);
    po::store(opts, vm);
    po::notify(vm);

    auto nrrd_file = vm["nrrd-file"].as<std::string>();

    auto ihdr_file = vm["ihdr-file"].as<std::string>();

    bool encode = true;
    if (true)
      {
      auto working_mode = vm["working-mode"].as<std::string>();
      if (working_mode == "encode")
        encode = true;
      else if (working_mode == "decode")
        encode = false;
      else
        throw std::logic_error("Working mode allowed values are 'encode' or 'decode' only.");
      }

    bool use_compression = (vm.count("use-compression") == 1u);

    auto ffmpeg_encoder_parameters = vm["ffmpeg-encoder-parameters"].as<std::string>();
    auto ffmpeg_encoder = itk::FFmpegImageIO::Encoding::ffv1;
    if (true)
      {
      auto encoder_str = vm["ffmpeg-encoder"].as<std::string>();
      if (encoder_str == "ffv1")
        ffmpeg_encoder = itk::FFmpegImageIO::Encoding::ffv1;
      else if (encoder_str == "h264")
        ffmpeg_encoder = itk::FFmpegImageIO::Encoding::h264;
      else if (encoder_str == "h265")
        ffmpeg_encoder = itk::FFmpegImageIO::Encoding::h265;
      else
        throw std::logic_error("Unknown encoder.");
      }

    auto ffmpeg_path = vm["ffmpeg-path"].as<std::string>();

    typedef itk::Image<uint16_t, 3> _Image;
    typedef itk::ImageFileReader<_Image> _Reader;
    typedef itk::ImageFileWriter<_Image> _Writer;

    auto reader = _Reader::New();
    auto writer = _Writer::New();

    if (encode)
      {
      reader->SetFileName(nrrd_file);
      reader->SetImageIO(itk::NrrdImageIO::New());
      
      auto io = itk::FFmpegImageIO::New();
      io->SetFFmpegPath(ffmpeg_path);
      if (!ffmpeg_encoder_parameters.empty())
        io->SetCustomEncodingCommandLine(ffmpeg_encoder_parameters);
      else
        io->SetEncoding(ffmpeg_encoder);

      writer->SetFileName(ihdr_file);
      writer->SetImageIO(io);
      }
    else
      {
      auto io = itk::FFmpegImageIO::New();
      io->SetFFmpegPath(ffmpeg_path);

      reader->SetFileName(ihdr_file);
      reader->SetImageIO(io);

      writer->SetFileName(nrrd_file);
      writer->SetImageIO(itk::NrrdImageIO::New());
      }

    if (use_compression)
      writer->UseCompressionOn();
    else
      writer->UseCompressionOff();

    std::cout << ">>>>>>> Decoding input file" << std::endl << "..." << std::endl;
    reader->Update();
    std::cout << "DONE." << std::endl;

    writer->SetInput(reader->GetOutput());
    writer->UseInputMetaDataDictionaryOn();

    std::cout << std::endl << "<<<<<<< Encoding output file" << std::endl << "..." << std::endl;
    writer->Update();
    std::cout << "DONE." << std::endl;

    return 0;
    }
  catch (const po::error& e)
    {
    std::cerr << "EXCEPTION:" << std::endl << "    " << e.what() << std::endl << std::endl;
    desc.print(std::cerr);
    return 1;
    }
  }
