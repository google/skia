
#include <fstream>

#include "object.h"

int main(int argc, char **argv)
{
  std::fstream fout;
  fout.open("commandoutput.h", std::ios::out);
  if (!fout)
    return 1;
  fout << "#define COMMANDOUTPUT_DEFINE\n";
  fout.close();
  return object();
}
