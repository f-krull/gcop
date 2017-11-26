#include "melist.h"
#include "smat.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Notes:
 * For all entries (e_ij) stored in the sparse matrix: i < j
 * (The value of elements e_ij with i > j will be "0")
 *
 * TODO:
 * reduce: write out_snp
 *
 */


/*----------------------------------------------------------------------------*/

void testPlinkTriGz(const char* fn_in, const char* fn_out) {
  MatEntryList mel(0);
  Timer tmel;
  MatEntryList::readMatPlinkTri(fn_in, &mel);
  tmel.stop();
  mel.printLen();

  Timer tsm;
  SparseMat sm(mel);
  tsm.stop();
  printf("time mel: %.2f  sm: %.2f\n", tmel.getSec(), tsm.getSec());
  sm.printSize();
  //sm.print();
  //sm.printMat();
  sm.write(fn_out);
  //sm.read(fn_out);
  //sm.printMat();
}

/* input snps, bed, ld
 *  snps -> bed_id
 */
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

typedef std::map<std::string, uint32_t> Snp2Idx;

Snp2Idx readIdxFromBed(const char *fn) {
  std::ifstream infile;
  std::string line;

  Snp2Idx snp2bed;
  infile.open(fn, std::ios::in);
  if (infile == NULL) {
    fprintf(stderr, "error: cannot open file %s", fn);
    exit(1);
  }
  uint32_t n = 0;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    std::string data;
    getline(iss, data, '\t');
    getline(iss, data, '\t');
    snp2bed[data] = n;
    n++;
  }
  infile.close();
  return snp2bed;
}

/*----------------------------------------------------------------------------*/

std::vector<std::string> readSnpIds(const char *fn) {
  std::ifstream infile;
  std::string line;
  std::vector<std::string> snps;

  infile.open(fn, std::ios::in);
  if (infile == NULL) {
    fprintf(stderr, "error: cannot open file %s", fn);
    exit(1);
  }
  while (std::getline(infile, line)) {
    snps.push_back(line);
  }
  infile.close();
  return snps;
}

/*----------------------------------------------------------------------------*/

bool getIdx(const Snp2Idx &s2i, const std::string &name, uint32_t *idx) {
  Snp2Idx::const_iterator it = s2i.find(name);
  if (it == s2i.end()) {
    return false;
  }
  (*idx) = (*it).second;
  return true;
}

/*----------------------------------------------------------------------------*/

void createSubMat(const char* fnMysnps, const char* fnBed,
    const char* fnSmat_in, const char* fnSmat_out) {
  /* read my SNP ids */
  fprintf(stderr, "reading custom SNP IDs (%s)\n", fnMysnps);
  std::vector<std::string> mySnps = readSnpIds(fnMysnps);
  if (mySnps.size() == 0) {
    fprintf(stderr, "error: read empty SNP id file\n");
  }
  fprintf(stderr, " ... found %lu SNP IDs\n", mySnps.size());

  /* read big SNP ids from .bed */
  fprintf(stderr, "reading SNP IDs from superset (%s)\n", fnBed);
  Snp2Idx snp2bed = readIdxFromBed(fnBed);
  fprintf(stderr, " ... found %lu SNP IDs\n", snp2bed.size());
  
  /* read big smat */
  fprintf(stderr, "reading superset LD matrix (%s)\n", fnSmat_in);
  SparseMat mBig(fnSmat_in);
  mBig.printSize();
  fprintf(stderr, "building indices\n");
  /* translate my-idx to bim-idx: */
  std::vector<uint8_t> hasIdx(mySnps.size(), 0);
  std::vector<uint32_t> bIdx(mySnps.size(), 0);
  uint32_t skip = 0; 
  for (uint32_t i = 0; i < mySnps.size(); i++) {
    hasIdx[i] = getIdx(snp2bed, mySnps[i], &bIdx[i]);
    skip += hasIdx[i] ? 0 : 1;
  }
  fprintf(stderr, "skip: %u\n", skip);
  fprintf(stderr, "building custom LD matrix (%s)\n", fnSmat_in);
  /* skip missing with zeros */
  {
    MatEntryList mel(mySnps.size());
    for (uint32_t i = 0; i < mySnps.size(); i++) {
      if (!hasIdx[i]) {
        continue;
      }
      for (uint32_t j = i+1; j < mySnps.size(); j++) {
        if (!hasIdx[j]) {
          continue;
        }
        mel.addValue(i, j, mBig.get(bIdx[i],bIdx[j]));
      }
      if (i % 100 == 0) {
        printf("progress: %f\n", float(i+1)/mySnps.size());
      }
    }
    fprintf(stderr, "mel:  %lu\n", mel.l().size());
    /* only a sorted list can be used to construct SparseMat */
    mel.sort();
    /* build sparse matrix */
    SparseMat mSmall(mel);
    mSmall.printSize();
    /* write to file */
    mSmall.write(fnSmat_out);
  }
}

/*----------------------------------------------------------------------------*/

uint32_t processPlinkLdTriTxt(const char* fn_in, MatrixEntrySink *sink) {
  FILE *f = fopen(fn_in, "r");
  if (!f) {
    fprintf(stderr, "error: opening file \"%s\"\n", fn_in);
    exit(1);
  }
  size_t  linesiz = 0;
  char*   linebuf = 0;
  ssize_t linelen = 0;
  uint32_t j = 0;
  const uint32_t report_intvl = 1000;
  rewind(f);
  while ((linelen=getline(&linebuf, &linesiz, f)) > 0) {
    char* pos = linebuf;
    char* end = NULL;
    uint32_t i = 0;

    /* break if newline or null */
    while (pos[0] != '\n' && pos[0] != '\0') {
      float value = strtof(pos, &end);
      if (end == pos) {
        break;
      }
      pos=end;
      sink->addEntryF(i, j, value);
      i++;
    }
    if (i != j+1) {
      fprintf(stderr, "error: wrong file format - expected triangular matrix (lower half)\n");
      exit(1);
    }
    free(linebuf);
    linebuf=NULL;
    j++;
    if ((j % report_intvl) == 0) {
      const float coverage = float(sink->numEntries())/pow(j ,2.);
      fprintf(stderr, "line: %u - entries: %lu (%f)\n", j, sink->numEntries(), coverage);
    }
  }
  free(linebuf);
  const float coverage = float(sink->numEntries())/pow(j ,2.);
  printf("read matrix rows: %u entries: %lu (%f)\n", j, sink->numEntries(), coverage);
  fclose(f);
  return j;
}

/*----------------------------------------------------------------------------*/

void cmd_list(const char* fn_in, const char* fn_out) {
  /* open ld_file */
  MatEntryWriter mew;
  if (!mew.open(fn_out)) {
    fprintf(stderr, "error: opening file \"%s\"\n", fn_in);
    exit(1);
  }
  const uint32_t numRows = processPlinkLdTriTxt(fn_in, &mew);
  if (!mew.close(numRows)) {
    fprintf(stderr, "error: writing to file \"%s\"\n", fn_in);
    exit(1);
  }
}

/*----------------------------------------------------------------------------*/

void cmd_smat(const char* fn_in, const char* fn_out) {
  /* open ld-list file */
  MatEntryReader mer;
  if (!mer.open(fn_in)) {
    fprintf(stderr, "error: opening file \"%s\"\n", fn_in);
    exit(1);
  }
  printf("read list-matrix - rows: %u entries: %lu\n", mer.n(), mer.numEntries());

  Timer tsm;
  SparseMat sm(mer);
  tsm.stop();
  printf("time: %.2f\n", tsm.getSec());
  sm.printSize();
  sm.write(fn_out);
}

/*----------------------------------------------------------------------------*/

struct SelInfo {
  std::vector<uint32_t> orgSelectedIdx;
  std::vector<uint32_t> org2SubIdx;
};

/*----------------------------------------------------------------------------*/
#include <algorithm>
SelInfo getBimIds(const char *fnBim, const char *fnSel) {
  SelInfo si;
  /* read custom SNP list */
  std::vector<std::string> selectionNames;
  {
    std::ifstream infile;
    std::string line;

    infile.open(fnSel, std::ios::in);
    if (infile == NULL) {
      fprintf(stderr, "error: cannot open file %s", fnSel);
      exit(1);
    }
    while (std::getline(infile, line)) {
      selectionNames.push_back(line);
    }
    infile.close();
  }
  /* read big SNP list (from .bim) */
  std::sort(selectionNames.begin(), selectionNames.end());
  {
     std::ifstream infile;
     std::string line;

     infile.open(fnBim, std::ios::in);
     if (infile == NULL) {
       fprintf(stderr, "error: cannot open file %s", fnBim);
       exit(1);
     }
     uint32_t i = 0;
     uint32_t numSel = 0;
     while (std::getline(infile, line)) {
       si.org2SubIdx.push_back(0);
       if (std::binary_search(selectionNames.begin(), selectionNames.end(), line)) {
         si.orgSelectedIdx.push_back(i);
         si.org2SubIdx.back() = numSel;
         numSel++;
       }
       i++;
     }
     infile.close();
  }
  return si;
}

void cmd_reduce(const char* fnMysnps, const char* fnBed,
    const char* fnMel, const char* fnMelSel) {

  SelInfo si = getBimIds(fnBed, fnMysnps);
  /* get list if  */
  MatEntryReader mer;
  if (!mer.open(fnMel)) {
     fprintf(stderr, "error: opening file \"%s\"\n", fnMel);
     exit(1);
   }
   printf("read list-matrix - rows: %u entries: %lu\n", mer.n(), mer.numEntries());
   MatEntryList::MatEntry e;
   std::vector<uint32_t> eSel;
   uint32_t n = 0;

   MatEntryWriter mew;
   if (!mew.open(fnMelSel)) {
     fprintf(stderr, "error: opening file \"%s\"\n", fnMelSel);
     exit(1);
   }

   while (mer.read(&e)) {
     bool keep = true;
     keep = keep && std::binary_search(si.orgSelectedIdx.begin(), si.orgSelectedIdx.end(), e.i);
     keep = keep && std::binary_search(si.orgSelectedIdx.begin(), si.orgSelectedIdx.end(), e.j);
     if (keep) {
       eSel.push_back(n);
       /* org-idx -> sel-idx */
       mew.addEntryB(si.org2SubIdx[e.i], si.org2SubIdx[e.j], e.v);
     }
     n++;
   }
   if (n != mer.numEntries() || n == 0) {
     fprintf(stderr, "error: reading file \"%s\"\n", fnMel);
     exit(1);
   }
   if (!mew.close(si.orgSelectedIdx.size())) {
     fprintf(stderr, "error: writing to file \"%s\"\n", fnMelSel);
     exit(1);
   }
   printf("selected %lu of %u entries\n", eSel.size(), n);
}

/*----------------------------------------------------------------------------*/

void usage(const char* prog) {
  fprintf(stderr, "Usage: %s convert IN_LD OUT_SMAT\n", prog);
  fprintf(stderr, "       %s list    IN_LD OUT_LDLIST\n", prog);
  fprintf(stderr, "       %s smat    IN_LDLIST OUT_MAT\n", prog);
  fprintf(stderr, "       %s derive  [args]\n", prog);
  fprintf(stderr, "       %s reduce  IN_MY_SNP IN_MYSNP IN_LDLIST OUT_LDLIST\n", prog);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char ** argv) {
  if (argc == 1) {
    usage(argv[0]);
    exit(1);
  }
  if (argv[1][0] == 'c' && argc == 4) {
    const char* fn_in  = argc < 3 ? NULL : argv[2];
    const char* fn_out = argc < 4 ? NULL : argv[3];
    testPlinkTriGz(fn_in, fn_out);
    return 0;
  }
  if (argv[1][0] == 'l' && argc == 4) {
    const char* fn_in  = argc < 3 ? NULL : argv[2];
    const char* fn_out = argc < 4 ? NULL : argv[3];
    cmd_list(fn_in, fn_out);
    return 0;
  }
  if (argv[1][0] == 's' && argc == 4) {
    const char* fn_in  = argc < 3 ? NULL : argv[2];
    const char* fn_out = argc < 4 ? NULL : argv[3];
    cmd_smat(fn_in, fn_out);
    return 0;
  }
  
  if (argv[1][0] == 'r' && argc == 6) {
    cmd_reduce(argv[2], argv[3], argv[4], argv[5]);
    return 0;
  }

  if (argv[1][0] == 'd' && argc == 6) {
    createSubMat(argv[2], argv[3], argv[4], argv[5]);
    return 0;
  }

  usage(argv[0]);
  return 0;
}
