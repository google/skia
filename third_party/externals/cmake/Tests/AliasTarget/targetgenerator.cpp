
#include <fstream>

int main(int argc, char **argv)
{
  std::fstream fout;
  fout.open("targetoutput.h", std::ios::out);
  if (!fout)
    return 1;
  fout << "#define TARGETOUTPUT_DEFINE\n";
  fout.close();
  return 0;
}
