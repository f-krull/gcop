#include "clusterercl.h"
#include "dendrogram.h"
#include <limits.h>
#include <float.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <assert.h>

/*----------------------------------------------------------------------------*/

ClustererCl::Candidate::Candidate(double dist) {
  this->dist = dist;
  last = NULL;
  next = NULL;
  clus1 = NULL;
  clus2 = NULL;
}

/*----------------------------------------------------------------------------*/

ClustererCl::Adjacent::Adjacent(Candidate *cand, uint32_t id) {
  this->cand = cand;
  this->id = id;
  next = NULL;
  last = NULL;
}

/*----------------------------------------------------------------------------*/

ClustererCl::CandList::CandList() {
  head = NULL;
  tail = NULL;
  size = 0;
}

/*----------------------------------------------------------------------------*/

ClustererCl::CandList::~CandList() {
  Candidate *tmp;

  while (head != NULL) {
    tmp = head;
    head = head->next;
    delete tmp;
  }
}

/*----------------------------------------------------------------------------*/

void ClustererCl::CandList::append(Candidate *cand) {
  if (tail == NULL) {
    /* list is empty */
    head = cand;
    tail = cand;
    cand->next = NULL;
    cand->last = NULL;
  } else {
    /* list contains at least one element */
    tail->next = cand;
    cand->last = tail;
    tail = cand;
    tail->next = NULL;
  }
  size++;
}

/*----------------------------------------------------------------------------*/

void ClustererCl::CandList::insertAfter(Candidate *cand, Candidate *cand_new) {
  if (cand != NULL) {
    if (cand->next != NULL) {
      cand->next->last = cand_new;
    } else { /* next is tail */
      tail = cand_new;
    }
    cand_new->next = cand->next;
    cand->next = cand_new;
    cand_new->last = cand;
  } else {
    append(cand_new);
  }
}

/*----------------------------------------------------------------------------*/

void ClustererCl::CandList::print() {
  Candidate *cd;

  cd = head;
  std::cout << "candlist" << std::endl;
  while (cd != NULL) {
    std::cout << std::setw(3) << cd->clus1->id << std::setw(5) << cd->dist;
    std::cout << std::setw(3) << cd->clus2->id << std::endl;
    cd = cd->next;
  }
}

/*----------------------------------------------------------------------------*/

bool ClustererCl::CandList::compareCandidates(const Candidate *a,
    const Candidate *b) {
  return a->dist < b->dist;
}

/*----------------------------------------------------------------------------*/

void ClustererCl::CandList::sort() {
  Candidate *cnode;
  std::vector<Candidate *> cv;

  /* copy list to array */
  cv.resize(size);
  /* NOTE: time efficient but needs unfragmented memory */
  cnode = head;
  for (uint32_t i = 0; i < cv.size(); i++) {
    cv[i] = cnode;
    cnode = cnode->next;
  }
  /* sort array */
  std::sort(cv.begin(), cv.end(), compareCandidates);
  /* clear list */
  head = NULL;
  tail = NULL;
  size = 0;
  /* rebuild list */
  for (uint32_t i = 0; i < cv.size(); i++) {
    append(cv[i]);
  }
}

/*----------------------------------------------------------------------------*/

bool ClustererCl::CandList::isEmpty() {
  return (head == NULL);
}

/*----------------------------------------------------------------------------*/

void ClustererCl::CandList::remove(Candidate *cnode) {
  if (cnode == head) {
    head = cnode->next;
  } else {
    cnode->last->next = cnode->next;
  }
  if (cnode == tail) {
    tail = cnode->last;
  } else {
    cnode->next->last = cnode->last;
  }
  delete cnode;
  size--;
}

/*----------------------------------------------------------------------------*/

ClustererCl::AdjList::AdjList() {
  head = NULL;
  tail = NULL;
  size = 0;
}

/*----------------------------------------------------------------------------*/

ClustererCl::AdjList::~AdjList() {
  Adjacent *tmp;

  /* TODO: still a bug here */
  return;
  while (head != NULL) {
    tmp = head;
    head = head->next;
    delete tmp;
  }
}

/*----------------------------------------------------------------------------*/

void ClustererCl::AdjList::append(Adjacent *adj) {
  if (tail == NULL) {
    /* list is empty */
    head = adj;
    tail = adj;
    adj->next = NULL;
    adj->last = NULL;
  } else {
    /* list contains at least one element */
    tail->next = adj;
    adj->last = tail;
    tail = adj;
    tail->next = NULL;
  }
  size++;
}

/*----------------------------------------------------------------------------*/

void ClustererCl::AdjList::print() {
  Adjacent *ad;

  ad = head;
  std::cout << "adlist" << std::endl;
  while (ad != NULL) {
    std::cout << std::setw(3) << ad->id << std::endl;
    ad = ad->next;
  }
}

/*----------------------------------------------------------------------------*/

bool ClustererCl::AdjList::compareAdjacents(const Adjacent *a,
    const Adjacent *b) {
  return a->id < b->id;
}

/*----------------------------------------------------------------------------*/

void ClustererCl::AdjList::sort() {
  Adjacent *anode;
  std::vector<Adjacent *> av;

  /* copy list to array */
  av.resize(size);
  anode = head;
  for (uint32_t i = 0; i < av.size(); i++) {
    av[i] = anode;
    anode = anode->next;
  }
  /* sort array */
  std::sort(av.begin(), av.end(), compareAdjacents);
  /* clear list */
  head = NULL;
  tail = NULL;
  size = 0;
  /* rebuild list */
  for (uint32_t i = 0; i < av.size(); i++) {
    append(av[i]);
  }
}

/*----------------------------------------------------------------------------*/

bool ClustererCl::AdjList::isEmpty() {
  return (head == NULL);
}

/*----------------------------------------------------------------------------*/

void ClustererCl::AdjList::remove(Adjacent *anode) {
  if (anode == head) {
    head = anode->next;
  } else {
    anode->last->next = anode->next;
  }
  if (anode == tail) {
    tail = anode->last;
  } else {
    anode->next->last = anode->last;
  }
  delete anode;
  size--;
}

/*----------------------------------------------------------------------------*/

void ClustererCl::clearAlist(AdjList *alist) {
  Adjacent *a1, *a2;
  uint32_t i1, i2;

  while (!alist->isEmpty()) {
    a1 = alist->head->cand->clus1;
    a2 = alist->head->cand->clus2;
    i1 = a1->id;
    i2 = a2->id;
    m_candList.remove(alist->head->cand);
    m_adjLists[i1].remove(a2);
    m_adjLists[i2].remove(a1);
  }
}

/*----------------------------------------------------------------------------*/

ClustererCl::ClustererCl(DistanceMatrix *dm, double maxDist) :
    m_candList(), m_adjLists(), m_clusterIds() {
  std::vector<uint32_t> nodes1;
  std::vector<uint32_t> nodes2;

  m_numClusters = dm->getNumElements();
  /* create candidate list */
  for (uint32_t i = 0; i < dm->getNumElements(); i++) {
    for (uint32_t j = i + 1; j < dm->getNumElements(); j++) {
      if ((maxDist != 0) && (dm->get(i, j) > maxDist)) {
        continue;
      }
      m_candList.append(new Candidate(dm->get(i, j)));
      nodes1.push_back(i);
      nodes2.push_back(j);
    }
  }
  m_adjLists.resize(dm->getNumElements());
  {
    Candidate *cand;
    uint32_t k;
    cand = m_candList.head;
    Adjacent *anode1, *anode2;

    k = 0;
    while (cand != NULL) {
      anode1 = new Adjacent(cand, nodes1[k]);
      anode2 = new Adjacent(cand, nodes2[k]);
      m_adjLists[nodes1[k]].append(anode2);
      m_adjLists[nodes2[k]].append(anode1);
      cand->clus1 = anode1;
      cand->clus2 = anode2;
      cand = cand->next;
      k++;
    }
  }
  m_candList.sort();
  for (uint32_t i = 0; i < m_adjLists.size(); i++) {
    m_adjLists[i].sort();
  }
  m_clusterIds.resize(dm->getNumElements());
  for (uint32_t i = 0; i < m_clusterIds.size(); i++) {
    m_clusterIds[i] = i;
  }
}

/*----------------------------------------------------------------------------*/

ClustererCl::~ClustererCl() {
  for (uint32_t i = 0; i < m_adjLists.size(); i++) {
    clearAlist(&m_adjLists[i]);
  }
}

/*----------------------------------------------------------------------------*/

bool ClustererCl::linkNext(uint32_t *i, uint32_t *j, double *dist) {
  Candidate *cand, *cand_new;
  Candidate *cand1_r, *cand2_r;
  Adjacent *cia_r, *cja_r;
  Adjacent *c1a_new, *c2a_new;
  AdjList *lc1, *lc2;
  AdjList lc_new;
  uint32_t c1, c2;
  uint32_t ci, cj, c_new;

  /* any candidates left? */
  if (m_candList.isEmpty()) {
    (*i) = 0;
    (*j) = 0;
    (*dist) = (double) 0;
    return false;
  }
  cand = m_candList.head;
  /* convert indices to cluster IDs */
  (*i) = m_clusterIds[cand->clus1->id];
  (*j) = m_clusterIds[cand->clus2->id];
  (*dist) = cand->dist;
  c1 = cand->clus1->id;
  c2 = cand->clus2->id;
  c_new = m_numClusters;
  /* process adjacency lists of c1 and c2 */
  while ((!(lc1 = &m_adjLists[c1])->isEmpty())
      && (!(lc2 = &m_adjLists[c2])->isEmpty())) {
    cia_r = (
        lc1->head->id != lc1->head->cand->clus1->id ?
            lc1->head->cand->clus1 : lc1->head->cand->clus2);
    cja_r = (
        lc2->head->id != lc2->head->cand->clus1->id ?
            lc2->head->cand->clus1 : lc2->head->cand->clus2);
    ci = lc1->head->id;
    cj = lc2->head->id;
    if (m_clusterIds[ci] < m_clusterIds[cj]) {
      /* delete entry in lc1 and its references */
      cand1_r = lc1->head->cand;
      m_adjLists[c1].remove(lc1->head);
      m_adjLists[ci].remove(cia_r);
      m_candList.remove(cand1_r);
    } else if (m_clusterIds[ci] > m_clusterIds[cj]) {
      /* delete entry in lc2 and its references */
      cand2_r = lc2->head->cand;
      m_adjLists[c2].remove(lc2->head);
      m_adjLists[cj].remove(cja_r);
      m_candList.remove(cand2_r);
    } else { /* clusterIds[ci] == clusterIds[cj] */
      cand1_r = lc1->head->cand;
      cand2_r = lc2->head->cand;
      /* lists have same adjacent -> link to new cluster */
      if (cand1_r->dist <= cand2_r->dist) {
        cand_new = new Candidate(cand1_r->dist);
        m_candList.insertAfter(cand1_r, cand_new);
      } else {
        cand_new = new Candidate(cand2_r->dist);
        m_candList.insertAfter(cand2_r, cand_new);
      }
      c1a_new = new Adjacent(cand_new, c1);
      c2a_new = new Adjacent(cand_new, ci);
      cand_new->clus1 = c1a_new;
      cand_new->clus2 = c2a_new;
      m_adjLists[ci].append(c1a_new);
      lc_new.append(c2a_new);
      /* clear old references to c1 and c2 */
      m_adjLists[c1].remove(lc1->head);
      m_adjLists[c2].remove(lc2->head);
      m_adjLists[ci].remove(cia_r);
      m_adjLists[ci].remove(cja_r);
      m_candList.remove(cand1_r);
      m_candList.remove(cand2_r);
    }
  }
  /* clean up lc1 and lc2 */
  clearAlist(&m_adjLists[c1]);
  clearAlist(&m_adjLists[c2]);
  /* replace list of c1 with list of new cluster */
  m_adjLists[c1] = lc_new;
  /* incerement cluster ID for new cluster */
  m_clusterIds[c1] = c_new;
  m_numClusters++;
  return true;
}

/*----------------------------------------------------------------------------*/

Dendrogram* ClustererCl::clusterAll(DistanceMatrix *dm) {
  double dist;
  /* do clustering */
  DendrogramBuilder db(dm->getNumElements());
  {
    uint32_t i, j;
    ClustererCl cl(dm, FLT_MAX);
    while ((cl.linkNext(&i, &j, &dist))) {
      db.pair(i, j, dist);
    }
  }
  return db.getDend();
}
