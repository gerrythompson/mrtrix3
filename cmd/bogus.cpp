#include "command.h"
#include "debug.h"
#include "image.h"

#include "adapter/vector.h"

namespace MR {
}


using namespace MR;
using namespace App;


void usage ()
{
  AUTHOR = "Joe Bloggs (joe.bloggs@acme.org)";

  DESCRIPTION
  + "raise each voxel intensity to the given power (default: 2)";

  ARGUMENTS
  + Argument ("in", "the input image.").type_image_in ()
  + Argument ("out", "the output image.").type_image_out ();
}


void run ()
{
//  Eigen::MatrixXf M = Eigen::MatrixXf::Random (4,3);
//  std::cerr.width(15);
//  VAR(M);

  auto input = Image<float>::open (argument[0]).with_direct_io (Stride::contiguous_along_axis((3)));

  Adapter::Vector<decltype(input)> vector_input (input);

  vector_input.index(0) = 30;
  vector_input.index(1) = 30;
  vector_input.index(2) = 30;

  Eigen::VectorXf vector = vector_input.value();
  std::cout << vector << std::endl;


  auto header = input.header();
  header.stride(0) = 1;
  header.stride(1) = 2;
  header.stride(2) = 3;
  header.stride(3) = 0;
  auto temp = Image<float>::scratch (header);

  threaded_copy (input, temp, 0, 3);

  temp.index(0) = 30;
  temp.index(1) = 30;
  temp.index(2) = 30;

  std::cout << temp.value() << std::endl;
}
