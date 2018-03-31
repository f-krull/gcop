#!/usr/bin/env bash

trap 'exit' ERR

declare -r SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r OUTDIR="$SCRIPTDIR"/../../testdata/tfbs_gtrd/

mkdir -p "$OUTDIR"
mkdir -p "$OUTDIR"/tmp


declare -r GTRDFILE="$OUTDIR"/tmp/human_meta_clusters.interval.gz

if [ ! -e $GTRDFILE ]; then
  wget http://gtrd.biouml.org/downloads/18.01/human_meta_clusters.interval.gz \
    -O "$GTRDFILE"
fi

if false; then
printf "path\ttitle\tID\tcellset\t\n" > $OUTDIR/tab_gtrdtf.txt
declare last_tfid=""
while IFS=$'\t' read chr start end su tfid tftitle cellset _; do
  fnid="$(echo "$tfid" | tr . "_")"
  fn=$OUTDIR/tf_${fnid}.txt
  if [ "$last_tfid" != "$tfid" ]; then
    rm -f $OUTDIR/$fnid
    last_tfid="$tfid"
    printf "%s\t%s\t%s\t%s\n" "$fn" "$tftitle" "$tfid" "$cellset" \
      >> $OUTDIR/tab_gtrdtf.txt
  fi
  printf "%s\t%s\t%s\n" "$chr" "$start" "$end" >> ${fn}

done < <(zcat $GTRDFILE | tail -n +2)
fi

zcat $GTRDFILE | tail -n +2 \
 | awk -F$'\t' -v dir=$OUTDIR '
    BEGIN{print "path\tname\tid\tcellset" > dir"/tab_gtrdtf.txt"}
    {printf "%s\t%s\t%s\n", $1, $2, $3 > dir"/tf_"$5".txt"}
    !seen[$5]++ {printf "%s\t%s\t%s\t%s\n", dir"/tf_"$5".txt.gz", $6, $5, $7 > dir"/tab_gtrdtf.txt"}
  '

for f in $OUTDIR/tf_*.txt; do
  gzip -f $f
done

#IDX_CHR=1
#IDX_S=2
#IDX_E=3
#IDX_TFID=5
#IDX_TFTITLE=6
#IDX_CELLSET=7
