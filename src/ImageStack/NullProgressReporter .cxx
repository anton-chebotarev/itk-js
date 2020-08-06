#include "NullProgressReporter.h"

void itkjs::ImageStack::NullProgressReporter::OnProgress(unsigned /*i_processed_cnt*/, unsigned /*i_total_cnt*/) const
  {
  }