/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGraphAdjacencyList_h
#define cmGraphAdjacencyList_h

#include "cmStandardIncludes.h"

/**
 * Graph edge representation.  Most use cases just need the
 * destination vertex, so we support conversion to/from an int.  We
 * also store boolean to indicate whether an edge is "strong".
 */
class cmGraphEdge
{
public:
  cmGraphEdge(): Dest(0), Strong(true) {}
  cmGraphEdge(int n): Dest(n), Strong(true) {}
  cmGraphEdge(int n, bool s): Dest(n), Strong(s) {}
  cmGraphEdge(cmGraphEdge const& r): Dest(r.Dest), Strong(r.Strong) {}
  operator int() const { return this->Dest; }

  bool IsStrong() const { return this->Strong; }
private:
  int Dest;
  bool Strong;
};
struct cmGraphEdgeList: public std::vector<cmGraphEdge> {};
struct cmGraphNodeList: public std::vector<int> {};
struct cmGraphAdjacencyList: public std::vector<cmGraphEdgeList> {};

#endif
