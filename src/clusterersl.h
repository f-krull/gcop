#ifndef CLUSTERERSL_H_
#define CLUSTERERSL_H_

#include <stdint.h>
#include "dendrogram.h"
/*----------------------------------------------------------------------------*/

template<class T>
class ClustererSl {
public:
  ClustererSl(DistanceMatrix<T> *dm, T maxDist);
  bool linkNext(uint32_t *i, uint32_t *j, T *dist);
  static Dendrogram* clusterAll(DistanceMatrix<T> *dm);
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
    T dist;
    uint32_t node1;
    uint32_t node2;
    MstEdge(uint32_t node1, uint32_t node2, T dist);
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

/*----------------------------------------------------------------------------*/

#include <math.h>
#include <limits.h>
#include <float.h>


/*----------------------------------------------------------------------------*/

template<class T>
bool ClustererSl<T>::compareEdges(const MstEdge &a, const MstEdge &b) {
  return a.dist < b.dist;
}

/*----------------------------------------------------------------------------*/

template<class T>
ClustererSl<T>::MstEdge::MstEdge(uint32_t n1, uint32_t n2, T dist) {
  this->node1 = n1;
  this->node2 = n2;
  this->dist = dist;
}

/*----------------------------------------------------------------------------*/

template<class T>
ClustererSl<T>::ClustererSl(DistanceMatrix<T>* dm, T maxDist) {
  m_currentEdge = 0;
  /* create edge list */
  for (uint32_t i = 0; i < dm->getNumElements(); i++) {
    for (uint32_t j = i + 1; j < dm->getNumElements(); j++) {
      if ((maxDist != 0) && (dm->get(i, j) > maxDist)) {
        continue;
      }
      m_edgeList.push_back(MstEdge(i, j, dm->get(i, j)));
    }
  }
  /* sort edges by distance */
  sort(m_edgeList.begin(), m_edgeList.end(), compareEdges);
  /* create union-find sets */
  m_ufNodes.resize(dm->getNumElements());
  for (uint32_t i = 0; i < m_ufNodes.size(); i++) {
    m_ufNodes[i].head = &m_ufNodes[i];
    m_ufNodes[i].next = NULL;
    m_ufNodes[i].clusterId = i;
  }
  m_numClusters = dm->getNumElements();
}

/*----------------------------------------------------------------------------*/

template<class T>
void ClustererSl<T>::unionTrees(UfNode *node1, UfNode *node2, uint32_t id) {
  /* reach end of first set */
  while (node1->next != NULL) {
    node1 = node1->next;
  }
  /* connect sets */
  node2 = node2->head;
  node1->next = node2;
  node2->head = node1->head;
  /* heads of second set are head of first set */
  while (node2->next != NULL) {
    node2 = node2->next;
    node2->head = node1->head;
  }
  /* set id of head */
  node1->head->clusterId = id;
}

/*----------------------------------------------------------------------------*/

template<class T>
bool ClustererSl<T>::linkNext(uint32_t *i, uint32_t *j, T *dist) {
  uint32_t node1, node2;
  /* find smallest edge */
  while (m_currentEdge < m_edgeList.size()) {
    node1 = m_edgeList[m_currentEdge].node1;
    node2 = m_edgeList[m_currentEdge].node2;
    /* node has to connect two unconnected trees */
    if (m_ufNodes[node1].head != m_ufNodes[node2].head) {
      /*edge found; store result */
      (*dist) = m_edgeList[m_currentEdge].dist;
      /* get cluster Ids */
      (*i) = m_ufNodes[node1].head->clusterId;
      (*j) = m_ufNodes[node2].head->clusterId;
      /* connect trees and set cluster IDs  */
      unionTrees(&m_ufNodes[node1], &m_ufNodes[node2], m_numClusters);
      /* edge was processed */
      m_currentEdge++;
      m_numClusters++;
      return true;
    }
    m_currentEdge++;
  }
  /* no edge found */
  (*i) = 0;
  (*j) = 0;
  (*dist) = (T) 0;
  return false;
}

/*----------------------------------------------------------------------------*/

template<class T>
Dendrogram* ClustererSl<T>::clusterAll(DistanceMatrix<T> *dm) {
  T dist;
  /* do clustering */
  DendrogramBuilder db(dm->getNumElements());
  {
    uint32_t i, j;
    ClustererSl<T> cl(dm, FLT_MAX);
    while ((cl.linkNext(&i, &j, &dist))) {
      db.pair(i, j, dist);
    }
  }
  return db.getDend();
}

#endif /* CLUSTERERSL_H_ */
