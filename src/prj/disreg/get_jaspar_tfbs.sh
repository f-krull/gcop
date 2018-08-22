#!/usr/bin/env bash

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r OUTDIR="$SCRIPTDIR"/../../testdata/tfbs_jaspar/

mkdir -p "$OUTDIR"
mkdir -p "$OUTDIR"/tmp


declare -r JASPARBED="$OUTDIR"/tmp/jaspar_bed.tar.gz

if [ ! -e $JASPARBED ]; then
  wget http://jaspar.genereg.net/download/bed_files.tar.gz \
    -O "$JASPARBED"
fi

# top dir is called "bed_files"...
tar xf $JASPARBED -C $OUTDIR --strip-components 1

# the files seem to cover other otganisms and builds - filter hg19
(
  printf "path\tID\tnum.BS\n"
  cd $OUTDIR
  for i in *bed; do
    if $(grep -q "hg" $i ); then
      printf "%s" $(readlink -f $i)
      printf "\t%s" $(basename $i .bed)
      printf "\t%s" $(cat $i | wc -l)
      printf "\n"
    fi
  done
) > $OUTDIR/tab_hg19.txt
