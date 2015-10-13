/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(SystemInformation.hxx)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemInformation.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

#if defined(KWSYS_USE_LONG_LONG)
# if defined(KWSYS_IOS_HAS_OSTREAM_LONG_LONG)
#  define iostreamLongLong(x) (x)
# else
#  define iostreamLongLong(x) ((long)x)
# endif
#elif defined(KWSYS_USE___INT64)
# if defined(KWSYS_IOS_HAS_OSTREAM___INT64)
#  define iostreamLongLong(x) (x)
# else
#  define iostreamLongLong(x) ((long)x)
# endif
#else
# error "No Long Long"
#endif

#define printMethod(info, m) kwsys_ios::cout << #m << ": " \
<< info.m() << "\n"

#define printMethod2(info, m, unit) kwsys_ios::cout << #m << ": " \
<< info.m() << " " << unit << "\n"

#define printMethod3(info, m, unit) kwsys_ios::cout << #m << ": " \
<< iostreamLongLong(info.m) << " " << unit << "\n"

int testSystemInformation(int, char*[])
{
  kwsys_ios::cout << "CTEST_FULL_OUTPUT\n"; // avoid truncation

  kwsys::SystemInformation info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();
  printMethod(info, GetOSName);
  printMethod(info, GetOSIsLinux);
  printMethod(info, GetOSIsApple);
  printMethod(info, GetOSIsWindows);
  printMethod(info, GetHostname);
  printMethod(info, GetFullyQualifiedDomainName);
  printMethod(info, GetOSRelease);
  printMethod(info, GetOSVersion);
  printMethod(info, GetOSPlatform);
  printMethod(info, GetVendorString);
  printMethod(info, GetVendorID);
  printMethod(info, GetTypeID);
  printMethod(info, GetFamilyID);
  printMethod(info, GetModelID);
  printMethod(info, GetExtendedProcessorName);
  printMethod(info, GetSteppingCode);
  printMethod(info, GetProcessorSerialNumber);
  printMethod2(info, GetProcessorCacheSize, "KB");
  printMethod(info, GetLogicalProcessorsPerPhysical);
  printMethod2(info, GetProcessorClockFrequency, "MHz");
  printMethod(info, Is64Bits);
  printMethod(info, GetNumberOfLogicalCPU);
  printMethod(info, GetNumberOfPhysicalCPU);
  printMethod(info, DoesCPUSupportCPUID);
  printMethod(info, GetProcessorAPICID);
  printMethod2(info, GetTotalVirtualMemory, "MB");
  printMethod2(info, GetAvailableVirtualMemory, "MB");
  printMethod2(info, GetTotalPhysicalMemory, "MB");
  printMethod2(info, GetAvailablePhysicalMemory, "MB");
  printMethod3(info, GetHostMemoryTotal(), "KiB");
  printMethod3(info, GetHostMemoryAvailable("KWSHL"), "KiB");
  printMethod3(info, GetProcMemoryAvailable("KWSHL","KWSPL"), "KiB");
  printMethod3(info, GetHostMemoryUsed(), "KiB");
  printMethod3(info, GetProcMemoryUsed(), "KiB");

  for (long int i = 0; i <= 31; i++)
    {
    if (info.DoesCPUSupportFeature(static_cast<long int>(1) << i))
      {
      kwsys_ios::cout << "CPU feature " << i << "\n";
      }
    }

  /* test stack trace
  */
  kwsys_ios::cout
    << "Program Stack:" << kwsys_ios::endl
    << kwsys::SystemInformation::GetProgramStack(0,0) << kwsys_ios::endl
    << kwsys_ios::endl;

  /* test segv handler
  info.SetStackTraceOnError(1);
  double *d = (double*)100;
  *d=0;
  */

  /* test abort handler
  info.SetStackTraceOnError(1);
  abort();
  */

  return 0;
}
