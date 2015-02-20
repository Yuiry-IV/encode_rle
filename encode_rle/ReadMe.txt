========================================================================
    CONSOLE APPLICATION : encode_rle Project Overview
========================================================================
This project has been implemented modified RLE encodgin. 
/////////////////////////////////////////////////////////////////////////
Inpute sequence should be transformed to:
 control     data[0] data[1]    data[n]
 1st byte  2nd byte             n <= 127
			or bytes
[76543210][76543210][76543210][76543210]
 |+--+--+  +--+---+
 |   |        |
 |   |        + data[0] value 
 |   |        
 |   + data repeat count or
 |     data sequece length  
 |     max 127 bytes 
 | 
 +- control bit +- it should be 0 if repeat count or 1 if seqence length 
 
Example:
"aaaabcebcebceffff" transformed to binary representation {'\x4','a','\x89','b','c','e','b','c','e','b','c','e','\x4','f'};
								   text represetation( for human use only): "[r4]a[s9]bcebcebce[r4]f";

