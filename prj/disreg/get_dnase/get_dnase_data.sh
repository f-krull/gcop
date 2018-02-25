#!/usr/bin/env bash

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r OUTDIR="$SCRIPTDIR"/../../../testdata/dnase/

mkdir -p "$OUTDIR"
mkdir -p "$OUTDIR"/tmp


if [ ! -e $"$OUTDIR"/tmp/dnase_meta.txt ]; then
  wget http://hgdownload.cse.ucsc.edu/goldenpath/hg19/encodeDCC/wgEncodeUwDnase/files.txt \
    -O $"$OUTDIR"/tmp/dnase_meta.txt
fi

cat $"$OUTDIR"/tmp/dnase_meta.txt \
  | IFS=";" awk '{ if ($7 == "view=Peaks;") printf "%s\t%s\n", $1, $8;}' \
  | sed "s/cell=//" \
  | sed "s/;$//" \
  > $"$OUTDIR"/tmp/filelist_dnase_peaks.txt  

cat $"$OUTDIR"/tmp/dnase_meta.txt \
  | IFS=";" awk '{ if ($7 == "view=Hotspots;") printf "%s\t%s\n", $1, $8;}' \
  | sed "s/cell=//" \
  | sed "s/;$//" \
  > $"$OUTDIR"/tmp/filelist_dnase_hotspots.txt  


get_data() {
  fnsrc=$1
  fndst=$2
  if [ -e "$fndst" ]; then
    return 0
  fi
  wget http://hgdownload.cse.ucsc.edu/goldenpath/hg19/encodeDCC/wgEncodeUwDnase/$fnsrc \
    -O $fndst
}


get_files() {
  local -r infile="$1"
  local -r outdir="$2"
  while read fn _; do
    dstfn=$fn
    dstfn=${dstfn%%.*} #| sed "s/wgEncodeUwDnase//"
    dstfn=${dstfn#wgEncodeUwDnase}.gz
    echo $fn $dstfn
    get_data $fn $outdir/$dstfn
  done < $infile
}  

mkdir -p $OUTDIR/pk
mkdir -p $OUTDIR/hs
get_files $OUTDIR/tmp/filelist_dnase_peaks.txt ${OUTDIR}/pk/
#get_files $OUTFOLDER/filelist_dnase_hotspots.txt $OUTDIR/hs/

# http://genome.ucsc.edu/ENCODE/cellTypes.html

get_tissue_table() {
  local -r file_filelist=$1
  local -r file_celltypes=$2
  local -r file_path=$3

  printf "%s\t%s\t%s\t%s\t%s\t%s\n" "path" "fncell" "cell" "tissue" "karyotype" "sex"
  while read fn cell; do 
    tissue=$(cat $file_celltypes | grep "^$cell"$'\t' | awk '{print $4}')
    karyotype=$(cat $file_celltypes | grep "^$cell"$'\t' | awk '{print $5}')
    sex=$(cat $file_celltypes | grep "^$cell"$'\t' | awk '{print $6}')
    fncell=$fn
    fncell=${fncell%%.*}
    fncell=${fncell#wgEncodeUwDnase}
    #fncell=$(echo ${fncell} | sed "s/HotspotsRep.*//")
    #fncell=$(echo ${fncell} | sed "s/PkRep.*//")
    # fix tissue
    case "$tissue" in
      "gingiva") tissue="gingival"
    esac
    tissue=$(echo $tissue)
    printf "%s\t%s\t%s\t%s\t%s\t%s\n" "$( readlink -f "$file_path/$fncell.gz")" "$fncell" "$cell" "$tissue" "$karyotype" "$sex"
  done < $file_filelist 
}
get_tissue_table $OUTDIR/tmp/filelist_dnase_hotspots.txt "$SCRIPTDIR"/encode_cell_types.csv $OUTDIR/hs > $OUTDIR/list_dnase_hs_encode.txt
get_tissue_table $OUTDIR/tmp/filelist_dnase_peaks.txt    "$SCRIPTDIR"/encode_cell_types.csv $OUTDIR/pk > $OUTDIR/list_dnase_pk_encode.txt
