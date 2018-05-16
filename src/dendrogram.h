#ifndef DENDROGRAM_H_
#define DENDROGRAM_H_

#include <string>
#include <vector>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

class DeNode;

/*----------------------------------------------------------------------------*/

class Dendrogram {
public:
  Dendrogram(uint32_t n, const std::vector<std::string> *nodeLables = NULL);
  ~Dendrogram();
  bool pair(uint32_t i, uint32_t j, float dist);
  std::vector<std::vector<uint32_t> > getClusters() const;
  std::vector<uint32_t> getOrder() const;

private:
  std::vector<std::string> m_nodeLables;
  std::vector<uint32_t> m_dict;
  std::vector<DeNode*> m_nodes;
  uint32_t m_n;
  uint32_t m_numPairs;
};

#endif /* DENDROGRAM_H_ */
