#!/usr/bin/env bash

#-------------------------------------------------------------------------------

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r GCOPDIR="$SCRIPTDIR/../../../"

make

#-------------------------------------------------------------------------------

# generate disreg matrix

time echo "
  load_strtable \
     dst=tab_tfbs \
     file=${GCOPDIR}/testdata/tfbs_gtrd/tab_gtrdtf.txt \
     maxlines=20000 \
     header=1


  load_strtable \
     dst=tab_gwas \
     file=${GCOPDIR}/testdata/gwascat/tab_genes.txt \
     maxlines=20000 \
     header=1


  disreg tabs=tab_gwas fmts=..c.se \
         tabp=tab_tfbs fmtp=cse \
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


#t <- read.table("/tmp/bla", skip=1, sep="\t")
#x <- seq(-4,4, length=100)
#plot(x,dnorm(x), type="l", col="red", ylim=c(0,0.6))
#points(density(unlist(t)), type="l")
#legend("topleft", c("normal dist","z-scores"), fill=c("red","black"))
