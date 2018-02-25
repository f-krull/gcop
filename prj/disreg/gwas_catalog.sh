#!/usr/bin/env bash

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r OUTDIR="$SCRIPTDIR"/../../../testdata/gwascat/

mkdir -p "$OUTDIR"
mkdir -p "$OUTDIR"/tmp


declare -r GWASCAT="$OUTDIR"/tmp/gwas_catalog_v1.0.1-associations_e91_r2018-02-06.tsv.gz

if [ ! -e $GWASCAT ]; then
  wget https://www.ebi.ac.uk/gwas/api/search/downloads/alternative \
    -O - | gzip > "$GWASCAT"
fi


declare -ar TRAITS=( $( zcat $GWASCAT | tail -n +2 | cut -f 8 | sort | uniq | tr " " "_"  ) )
rm -f $OUTDIR/list.txt
for i in ${!TRAITS[@]}; do
  trait="$(echo ${TRAITS[i]} | tr "_" " ")"
  outfile=$OUTDIR/${TRAITS[i]}.txt.gz 
  zcat $GWASCAT \
    | tail -n +2 \
    | awk -F $'\t' -v trait="$trait" '$8==trait && $12~/^[0-9]+$/ && $13~/^[0-9]+$/ { print $0 }' \
    | gzip \
    > $outfile
  if [ -s $outfile -a $(zcat $outfile | wc -l) != '0' ]; then
    echo $( readlink -f $outfile )  | tee -a $OUTDIR/list.txt
  fi
done

# make table
{ 
  printf  "%s\t%s\t%s\n" path name num.SNPs 
  cat $OUTDIR/list.txt \
      | while read line; do 
      printf "%s\t%s\t%s\n" $line \
                          $(basename $line ".txt.gz" | sed "s/['(),:]//g") \
                          $( zcat $line | wc -l )
    done | sort -r -n -k 3,3
} > $OUTDIR/tab.txt
