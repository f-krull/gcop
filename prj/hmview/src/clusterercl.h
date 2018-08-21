
#ifndef SRC_CLUSTERERCL_H_
#define SRC_CLUSTERERCL_H_

#include "distmatrix.h"
#include <vector>
#include <stdlib.h>

class Dendrogram;

/*----------------------------------------------------------------------------*/

class ClustererCl {
public:
  ClustererCl(DistanceMatrix *dm, double maxDist);
  ~ClustererCl();
  bool linkNext(uint32_t *i, uint32_t *j, double *dist);
  static Dendrogram* clusterAll(DistanceMatrix *dm);

private:
  /* prototype */
  class Adjacent;
  /* candidate consisting of two clusters which can be combined */
  class Candidate {
  public:
    Adjacent *clus1;
    Adjacent *clus2;
    double dist;
    Candidate *next;
    Candidate *last;
    Candidate(double dist);
  };
  /* links a cluster to a candidate */
  class Adjacent {
  public:
    uint32_t id;
    Candidate *cand;
    Adjacent *next;
    Adjacent *last;
    Adjacent(Candidate *cand, uint32_t id);
  };
  /* list containing all candidates */
  class CandList {
  public:
    Candidate *head;
    Candidate *tail;
    uint32_t size;
    CandList();
    ~CandList();
    void append(Candidate *cand);
    void insertAfter(Candidate *cand, Candidate *cand_new);
    void print();
    /* smaller distances first */
    static bool compareCandidates(const Candidate *a, const Candidate *b);
    void sort();
    bool isEmpty();
    void remove(Candidate *cand);
  };
  /* list containing all candidates including a specific cluster */
  class AdjList {
  public:
    Adjacent *head;
    Adjacent *tail;
    uint32_t size;
    AdjList();
    ~AdjList();
    void append(Adjacent *adj);
    void print();
    /* smaller IDs first */
    static bool compareAdjacents(const Adjacent *a, const Adjacent *b);
    void sort();
    bool isEmpty();
    void remove(Adjacent *anode);
  };
  /* list of candidates */
  CandList m_candList;
  /* adjacency lists for all clusters */
  std::vector<AdjList> m_adjLists;
  /* converts indices to increasing cluster IDs */
  std::vector<uint32_t> m_clusterIds;
  uint32_t m_numClusters;
  /* clears an adjacency list and its occurrences in other lists */
  void clearAlist(AdjList *alist);
};



#endif /* SRC_CLUSTERERCL_H_ */
