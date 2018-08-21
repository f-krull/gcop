# hapdose2bed

Converts Mach3 hapdose/info files to binary plink bed/fam files. 
Allows to set some QC parameters (see 'hapdose2bed -h'). 


Problem: The input matrix needs to be transposed.

Approach: The output bed file is created in-memory first, since it's 
much smaller than the input matrix. 
The in-memory output file allow random access writes during the conversion. 
Therefore the input matrix needs to be read once with only two lines stored 
in memory.
