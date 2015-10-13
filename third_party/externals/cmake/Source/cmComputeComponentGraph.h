/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmComputeComponentGraph_h
#define cmComputeComponentGraph_h

#include "cmStandardIncludes.h"

#include "cmGraphAdjacencyList.h"

#include <stack>

/** \class cmComputeComponentGraph
 * \brief Analyze a graph to determine strongly connected components.
 *
 * Convert a directed graph into a directed acyclic graph whose nodes
 * correspond to strongly connected components of the original graph.
 *
 * We use Tarjan's algorithm to enumerate the components efficiently.
 * An advantage of this approach is that the components are identified
 * in a topologically sorted order.
 */
class cmComputeComponentGraph
{
public:
  // Represent the graph with an adjacency list.
  typedef cmGraphNodeList NodeList;
  typedef cmGraphEdgeList EdgeList;
  typedef cmGraphAdjacencyList Graph;

  cmComputeComponentGraph(Graph const& input);
  ~cmComputeComponentGraph();

  /** Get the adjacency list of the component graph.  */
  Graph const& GetComponentGraph() const
    { return this->ComponentGraph; }
  EdgeList const& GetComponentGraphEdges(int c) const
    { return this->ComponentGraph[c]; }

  /** Get map from component index to original node indices.  */
  std::vector<NodeList> const& GetComponents() const
    { return this->Components; }
  NodeList const& GetComponent(int c) const
    { return this->Components[c]; }

  /** Get map from original node index to component index.  */
  std::vector<int> const& GetComponentMap() const
    { return this->TarjanComponents; }

private:
  void TransferEdges();

  Graph const& InputGraph;
  Graph ComponentGraph;

  // Tarjan's algorithm.
  struct TarjanEntry
  {
    int Root;
    int VisitIndex;
  };
  int TarjanWalkId;
  std::vector<int> TarjanVisited;
  std::vector<int> TarjanComponents;
  std::vector<TarjanEntry> TarjanEntries;
  std::stack<int> TarjanStack;
  int TarjanIndex;
  void Tarjan();
  void TarjanVisit(int i);

  // Connected components.
  std::vector<NodeList> Components;
};

#endif
