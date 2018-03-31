#!/usr/bin/env bash

#-------------------------------------------------------------------------------

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r GCOPDIR="$SCRIPTDIR/../../"

make -C "$GCOPDIR" disreg 

#-------------------------------------------------------------------------------

# generate disreg matrix

time echo "
  load_strtable \
     dst=tab_tfbs \
     file=${GCOPDIR}/testdata/tfbs_gtrd/tab_gtrdtf.txt \
     maxlines=200000 \
     header=1
     

  load_strtable \
     dst=tab_gwas \
     file=${GCOPDIR}/testdata/gwascat/tab_genes.txt \
     maxlines=681 \
     header=1


  disreg tab1=tab_tfbs fmt1=cse \
         tab2=tab_gwas  fmt2=..c.se \
         expand2=25000 \
         output=/tmp/disreg_matrix.txt
" \
  | tee /tmp/script | "$GCOPDIR"/bin/disreg > /tmp/log

#-------------------------------------------------------------------------------

exit 0

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
