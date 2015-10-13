/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmComputeLinkDepends_h
#define cmComputeLinkDepends_h

#include "cmStandardIncludes.h"
#include "cmTarget.h"

#include "cmGraphAdjacencyList.h"

#include <queue>

class cmComputeComponentGraph;
class cmGlobalGenerator;
class cmMakefile;
class cmTarget;
class cmake;

/** \class cmComputeLinkDepends
 * \brief Compute link dependencies for targets.
 */
class cmComputeLinkDepends
{
public:
  cmComputeLinkDepends(cmTarget const* target, const std::string& config);
  ~cmComputeLinkDepends();

  // Basic information about each link item.
  struct LinkEntry
  {
    std::string Item;
    cmTarget const* Target;
    bool IsSharedDep;
    bool IsFlag;
    LinkEntry(): Item(), Target(0), IsSharedDep(false), IsFlag(false) {}
    LinkEntry(LinkEntry const& r):
      Item(r.Item), Target(r.Target), IsSharedDep(r.IsSharedDep),
      IsFlag(r.IsFlag) {}
  };

  typedef std::vector<LinkEntry> EntryVector;
  EntryVector const& Compute();

  void SetOldLinkDirMode(bool b);
  std::set<cmTarget const*> const& GetOldWrongConfigItems() const
    { return this->OldWrongConfigItems; }

private:

  // Context information.
  cmTarget const* Target;
  cmMakefile* Makefile;
  cmGlobalGenerator const* GlobalGenerator;
  cmake* CMakeInstance;
  bool DebugMode;

  // Configuration information.
  bool HasConfig;
  std::string Config;
  cmTarget::LinkLibraryType LinkType;

  // Output information.
  EntryVector FinalLinkEntries;

  typedef cmTarget::LinkLibraryVectorType LinkLibraryVectorType;

  std::map<std::string, int>::iterator
  AllocateLinkEntry(std::string const& item);
  int AddLinkEntry(cmLinkItem const& item);
  void AddVarLinkEntries(int depender_index, const char* value);
  void AddDirectLinkEntries();
  template <typename T>
    void AddLinkEntries(int depender_index, std::vector<T> const& libs);
  cmTarget const* FindTargetToLink(int depender_index,
                                   const std::string& name);

  // One entry for each unique item.
  std::vector<LinkEntry> EntryList;
  std::map<std::string, int> LinkEntryIndex;

  // BFS of initial dependencies.
  struct BFSEntry
  {
    int Index;
    const char* LibDepends;
  };
  std::queue<BFSEntry> BFSQueue;
  void FollowLinkEntry(BFSEntry const&);

  // Shared libraries that are included only because they are
  // dependencies of other shared libraries, not because they are part
  // of the interface.
  struct SharedDepEntry
  {
    cmLinkItem Item;
    int DependerIndex;
  };
  std::queue<SharedDepEntry> SharedDepQueue;
  std::set<int> SharedDepFollowed;
  void FollowSharedDeps(int depender_index,
                        cmTarget::LinkInterface const* iface,
                        bool follow_interface = false);
  void QueueSharedDependencies(int depender_index,
                               std::vector<cmLinkItem> const& deps);
  void HandleSharedDependency(SharedDepEntry const& dep);

  // Dependency inferral for each link item.
  struct DependSet: public std::set<int> {};
  struct DependSetList: public std::vector<DependSet> {};
  std::vector<DependSetList*> InferredDependSets;
  void InferDependencies();

  // Ordering constraint graph adjacency list.
  typedef cmGraphNodeList NodeList;
  typedef cmGraphEdgeList EdgeList;
  typedef cmGraphAdjacencyList Graph;
  Graph EntryConstraintGraph;
  void CleanConstraintGraph();
  void DisplayConstraintGraph();

  // Ordering algorithm.
  void OrderLinkEntires();
  std::vector<char> ComponentVisited;
  std::vector<int> ComponentOrder;
  int ComponentOrderId;
  struct PendingComponent
  {
    // The real component id.  Needed because the map is indexed by
    // component topological index.
    int Id;

    // The number of times the component needs to be seen.  This is
    // always 1 for trivial components and is initially 2 for
    // non-trivial components.
    int Count;

    // The entries yet to be seen to complete the component.
    std::set<int> Entries;
  };
  std::map<int, PendingComponent> PendingComponents;
  cmComputeComponentGraph* CCG;
  std::vector<int> FinalLinkOrder;
  void DisplayComponents();
  void VisitComponent(unsigned int c);
  void VisitEntry(int index);
  PendingComponent& MakePendingComponent(unsigned int component);
  int ComputeComponentCount(NodeList const& nl);
  void DisplayFinalEntries();

  // Record of the original link line.
  std::vector<int> OriginalEntries;

  // Compatibility help.
  bool OldLinkDirMode;
  void CheckWrongConfigItem(cmLinkItem const& item);
  std::set<cmTarget const*> OldWrongConfigItems;
};

#endif
