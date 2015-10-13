/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include <cmProcess.h>
#include <cmSystemTools.h>

cmProcess::cmProcess()
{
  this->Process = 0;
  this->Timeout = 0;
  this->TotalTime = 0;
  this->ExitValue = 0;
  this->Id = 0;
  this->StartTime = 0;
}

cmProcess::~cmProcess()
{
  cmsysProcess_Delete(this->Process);
}
void cmProcess::SetCommand(const char* command)
{
  this->Command = command;
}

void cmProcess::SetCommandArguments(std::vector<std::string> const& args)
{
  this->Arguments = args;
}

bool cmProcess::StartProcess()
{
  if(this->Command.empty())
    {
    return false;
    }
  this->StartTime = cmSystemTools::GetTime();
  this->ProcessArgs.clear();
  // put the command as arg0
  this->ProcessArgs.push_back(this->Command.c_str());
  // now put the command arguments in
  for(std::vector<std::string>::iterator i = this->Arguments.begin();
      i != this->Arguments.end(); ++i)
    {
    this->ProcessArgs.push_back(i->c_str());
    }
  this->ProcessArgs.push_back(0); // null terminate the list
  this->Process = cmsysProcess_New();
  cmsysProcess_SetCommand(this->Process, &*this->ProcessArgs.begin());
  if(!this->WorkingDirectory.empty())
    {
    cmsysProcess_SetWorkingDirectory(this->Process,
                                     this->WorkingDirectory.c_str());
    }
  cmsysProcess_SetTimeout(this->Process, this->Timeout);
  cmsysProcess_Execute(this->Process);
  return (cmsysProcess_GetState(this->Process)
          == cmsysProcess_State_Executing);
}

//----------------------------------------------------------------------------
bool cmProcess::Buffer::GetLine(std::string& line)
{
  // Scan for the next newline.
  for(size_type sz = this->size(); this->Last != sz; ++this->Last)
    {
    if((*this)[this->Last] == '\n' || (*this)[this->Last] == '\0')
      {
      // Extract the range first..last as a line.
      const char* text = &*this->begin() + this->First;
      size_type length = this->Last - this->First;
      while(length && text[length-1] == '\r')
        {
        length --;
        }
      line.assign(text, length);

      // Start a new range for the next line.
      ++this->Last;
      this->First = Last;

      // Return the line extracted.
      return true;
      }
    }

  // Available data have been exhausted without a newline.
  if(this->First != 0)
    {
    // Move the partial line to the beginning of the buffer.
    this->erase(this->begin(), this->begin() + this->First);
    this->First = 0;
    this->Last = this->size();
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmProcess::Buffer::GetLast(std::string& line)
{
  // Return the partial last line, if any.
  if(!this->empty())
    {
    line.assign(&*this->begin(), this->size());
    this->First = this->Last = 0;
    this->clear();
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
int cmProcess::GetNextOutputLine(std::string& line, double timeout)
{
  for(;;)
    {
    // Look for lines already buffered.
    if(this->StdOut.GetLine(line))
      {
      return cmsysProcess_Pipe_STDOUT;
      }
    else if(this->StdErr.GetLine(line))
      {
      return cmsysProcess_Pipe_STDERR;
      }

    // Check for more data from the process.
    char* data;
    int length;
    int p = cmsysProcess_WaitForData(this->Process, &data, &length, &timeout);
    if(p == cmsysProcess_Pipe_Timeout)
      {
      return cmsysProcess_Pipe_Timeout;
      }
    else if(p == cmsysProcess_Pipe_STDOUT)
      {
      this->StdOut.insert(this->StdOut.end(), data, data+length);
      }
    else if(p == cmsysProcess_Pipe_STDERR)
      {
      this->StdErr.insert(this->StdErr.end(), data, data+length);
      }
    else // p == cmsysProcess_Pipe_None
      {
      // The process will provide no more data.
      break;
      }
    }

  // Look for partial last lines.
  if(this->StdOut.GetLast(line))
    {
    return cmsysProcess_Pipe_STDOUT;
    }
  else if(this->StdErr.GetLast(line))
    {
    return cmsysProcess_Pipe_STDERR;
    }

  // No more data.  Wait for process exit.
  if(!cmsysProcess_WaitForExit(this->Process, &timeout))
    {
    return cmsysProcess_Pipe_Timeout;
    }

  // Record exit information.
  this->ExitValue = cmsysProcess_GetExitValue(this->Process);
  this->TotalTime = cmSystemTools::GetTime() - this->StartTime;
  // Because of a processor clock scew the runtime may become slightly
  // negative. If someone changed the system clock while the process was
  // running this may be even more. Make sure not to report a negative
  // duration here.
  if (this->TotalTime <= 0.0)
    {
    this->TotalTime = 0.0;
    }
  //  std::cerr << "Time to run: " << this->TotalTime << "\n";
  return cmsysProcess_Pipe_None;
}

// return the process status
int cmProcess::GetProcessStatus()
{
  if(!this->Process)
    {
    return cmsysProcess_State_Exited;
    }
  return cmsysProcess_GetState(this->Process);
}

int cmProcess::ReportStatus()
{
  int result = 1;
  switch(cmsysProcess_GetState(this->Process))
    {
    case cmsysProcess_State_Starting:
      {
      std::cerr << "cmProcess: Never started "
           << this->Command << " process.\n";
      } break;
    case cmsysProcess_State_Error:
      {
      std::cerr << "cmProcess: Error executing " << this->Command
                << " process: "
                << cmsysProcess_GetErrorString(this->Process)
                << "\n";
      } break;
    case cmsysProcess_State_Exception:
      {
      std::cerr << "cmProcess: " << this->Command
                      << " process exited with an exception: ";
      switch(cmsysProcess_GetExitException(this->Process))
        {
        case cmsysProcess_Exception_None:
          {
          std::cerr << "None";
          } break;
        case cmsysProcess_Exception_Fault:
          {
          std::cerr << "Segmentation fault";
          } break;
        case cmsysProcess_Exception_Illegal:
          {
          std::cerr << "Illegal instruction";
          } break;
        case cmsysProcess_Exception_Interrupt:
          {
          std::cerr << "Interrupted by user";
          } break;
        case cmsysProcess_Exception_Numerical:
          {
          std::cerr << "Numerical exception";
          } break;
        case cmsysProcess_Exception_Other:
          {
          std::cerr << "Unknown";
          } break;
        }
      std::cerr << "\n";
      } break;
    case cmsysProcess_State_Executing:
      {
      std::cerr << "cmProcess: Never terminated " <<
        this->Command << " process.\n";
      } break;
    case cmsysProcess_State_Exited:
      {
      result = cmsysProcess_GetExitValue(this->Process);
      std::cerr << "cmProcess: " << this->Command
                << " process exited with code "
                << result << "\n";
      } break;
    case cmsysProcess_State_Expired:
      {
      std::cerr << "cmProcess: killed " << this->Command
                << " process due to timeout.\n";
      } break;
    case cmsysProcess_State_Killed:
      {
      std::cerr << "cmProcess: killed " << this->Command << " process.\n";
      } break;
    }
  return result;

}


int cmProcess::GetExitException()
{
  return cmsysProcess_GetExitException(this->Process);
}
