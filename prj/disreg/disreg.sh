#!/usr/bin/env bash

#-------------------------------------------------------------------------------

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r GCOPDIR="$SCRIPTDIR/../../"

make -C "$GCOPDIR" disreg 

#-------------------------------------------------------------------------------

# generate disreg matrix

time echo "
  load_strlist \
     dst=list_dnase \
     file=${GCOPDIR}/testdata/dnasepk_list.txt

  load_strlist \
     dst=list_gwas \
     file=${GCOPDIR}/testdata/gwascat/list.txt

  disreg list1=list_dnase fmt1=cse \
         list2=list_gwas  fmt2=...........cs \
         expand2=25000 \
         output=/tmp/disreg_matrix.txt
" \
  | "$GCOPDIR"/bin/disreg > /tmp/log

#-------------------------------------------------------------------------------

# create heatmap

time Rscript <(echo '
  t <- read.table("/tmp/disreg_matrix.txt", header=T, row.names=1)
  t <- as.matrix(t)
  # cap at max_val
  max_val <- 8
  t[which(t>max_val)] <- max_val
  pdf("/tmp/disreg.pdf", width=200, height=60)
  gplots::heatmap.2(t, margins=c(32,12), trace="none", key=F)
  dev.off()
')
