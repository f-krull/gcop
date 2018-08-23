# hapdose2bed

Converts Mach3 hapdose/info files to binary plink bed/fam files. 
Allows to set some QC parameters (see `hapdose2bed -h`). 


Problem: The input matrix needs to be transposed.

Approach: The output bed file is created in-memory first, since it's 
much smaller than the input matrix. 
The in-memory output file allow random access writes during the conversion. 
Therefore the input matrix needs to be read once with only two lines stored 
in memory.

## md5sums

### input

f90db07add171ffecc79ee7773555608  test.hapDose.gz
b032c5007be42b863ef1d5ed2de2ec2a  test.info.gz

### output

hapdose2bed testdata/test out

5ae0cd65745020f333cb45f8d8ac0986  out.bed
1dc6fc121d9df3c1ed964a4a46cac9f7  out.bim
d2361e51421bf057baf214dedb16b39c  out.fam
