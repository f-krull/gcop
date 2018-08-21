#ifndef CLUSTERERSL_H_
#define CLUSTERERSL_H_

#include "dendrogram.h"
#include "distmatrix.h"
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class ClustererSl {
public:
  ClustererSl(DistanceMatrix *dm, double maxDist);
  bool linkNext(uint32_t *i, uint32_t *j, double *dist);
  static Dendrogram* clusterAll(DistanceMatrix *dm);
private:
  /* union-find data struct */
  class UfNode {
  public:
    UfNode *head;
    UfNode *next;
    uint32_t clusterId;
  };
  /* edge list entry of MST */
  class MstEdge {
  public:
    double dist;
    uint32_t node1;
    uint32_t node2;
    MstEdge(uint32_t node1, uint32_t node2, double dist);
  };
  /* union-find nodes */
  std::vector<UfNode> m_ufNodes;
  /* MST edges */
  std::vector<MstEdge> m_edgeList;
  uint32_t m_currentEdge;
  uint32_t m_numClusters;

  /* used for sort(); smaller distances first */
  static bool compareEdges(const MstEdge &a, const MstEdge &b);
  /* union two UF sets */
  void unionTrees(UfNode *node1, UfNode *node2, uint32_t newId);

};

#endif /* CLUSTERERSL_H_ */
