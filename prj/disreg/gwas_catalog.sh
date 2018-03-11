#!/usr/bin/env bash

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r OUTDIR="$SCRIPTDIR"/../../testdata/gwascat/

mkdir -p "$OUTDIR"
mkdir -p "$OUTDIR"/tmp


declare -r GWASCAT="$OUTDIR"/tmp/gwas_catalog_v1.0.1-associations_e91_r2018-02-06.tsv.gz

if [ ! -e $GWASCAT ]; then
  wget https://www.ebi.ac.uk/gwas/api/search/downloads/alternative \
    -O - | gzip > "$GWASCAT"
fi


# separate SNPs by trait
declare -ar TRAITS=( $( zcat $GWASCAT | tail -n +2 | cut -f 8 | sort | uniq | tr " " "_"  ) )
rm -f $OUTDIR/list.txt
for i in ${!TRAITS[@]}; do
  trait="$(echo ${TRAITS[i]} | tr "_" " ")"
  outfile="$( printf "$OUTDIR/%s.txt.gz" $(echo ${TRAITS[i]} | sed "s/[/-]//g") )"
  if [ ! -e $outfile ]; then
    zcat $GWASCAT \
      | tail -n +2 \
      | awk -F $'\t' -v trait="$trait" '$8==trait && $12~/^[0-9]+$/ && $13~/^[0-9]+$/ { print $0 }' \
      | gzip \
      > $outfile
  fi
  if [ -s $outfile -a $(zcat $outfile | wc -l) != '0' ]; then
    echo $( readlink -f $outfile )  | tee -a $OUTDIR/list.txt
  fi
done


# make table of traits
{ 
  printf  "%s\t%s\t%s\n" path name num.SNPs 
  cat $OUTDIR/list.txt \
      | while read line; do 
      printf "%s\t%s\t%s\n" $line \
                          $(basename $line ".txt.gz" | sed "s/['(),:]//g") \
                          $( zcat $line | wc -l )
    done | sort -r -n -k 3,3
} > $OUTDIR/tab.txt


# get genes + gene postitions per trait

declare -r REFGENE="$OUTDIR"/tmp/refGene.txt
if [ ! -e $REFGENE ]; then
  wget http://hgdownload.soe.ucsc.edu/goldenPath/hg19/database/refGene.txt.gz \
    -O - | gunzip > "$REFGENE"
fi

{
  printf  "%s\t%s\t%s\n" path name num.genes
  while read file trait numsnps; do
    outfile="$(readlink -f $OUTDIR/${trait}_genes.txt.gz)"
    # get all gene symbols
    genes="$(zcat $file | cut -f 14 | tr -d " " | tr "," "\n" | sort | uniq)"
    if [ -z "$genes" ]; then
      continue
    fi
    # grep for all gene symbols at col 13 (exact match in set a)
    # filter chr1-22,X,Y
    genepos="$(
      awk 'FNR==NR{a[$1];next} ($13 in a)' <(echo "$genes") $REFGENE
    )"

    if [ -z "$genepos" ]; then
      continue
    fi

    echo "$genepos" | gzip > "$outfile"

    printf "%s\t%s\t%s\n" "$outfile" \
                          "$trait" \
                          "$(echo "$genepos" | wc -l)"
  done < <(tail -n +2 $OUTDIR/tab.txt)
} > $OUTDIR/tab_genes.txt
