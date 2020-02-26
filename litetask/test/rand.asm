-e100
0BBB:0100  20.
 55   25.8B   30.EC   34.8B   58.46   00.04   04.A3   46.00   
0BBB:0108  75.00   6E.C7   43.06   3A.02   5C.00   54.00   45.00   4D.5D   
0BBB:0110  50.C3   5C.90   66.B8   65.FD   66.43   65.BA   66.03   65.00   
0BBB:0118  66.52   65.50   2E.FF   66.36   34.02   00.00   AA.FF   0B.36   
0BBB:0120  00.00   04.00   53.E8   63.00   6F.00   70.05   65.C3   20.9E   
0BBB:0128  4F.83   66.D2   66.26   73.A3   65.00   74.00   3A.89   20.16   
0BBB:0130  20.02   20.00   20.8B   20.C2   20.80   20.E4   25.7F   30.C3
-u100 l38
0BBB:0100 55            PUSH	BP                                 
0BBB:0101 8BEC          MOV	BP,SP                              
0BBB:0103 8B4604        MOV	AX,[BP+04]                         
0BBB:0106 A30000        MOV	[0000],AX                          
0BBB:0109 C70602000000  MOV	WORD PTR [0002],0000               
0BBB:010F 5D            POP	BP                                 
0BBB:0110 C3            RET	                                   
0BBB:0111 90            NOP	                                   
0BBB:0112 B8FD43        MOV	AX,43FD                            
0BBB:0115 BA0300        MOV	DX,0003                            
0BBB:0118 52            PUSH	DX                                 
0BBB:0119 50            PUSH	AX                                 
0BBB:011A FF360200      PUSH	[0002]                             
0BBB:011E FF360000      PUSH	[0000]                             
0BBB:0122 E80000        CALL	0125                               
0BBB:0125 05C39E        ADD	AX,9EC3                            
0BBB:0128 83D226        ADC	DX,+26                             
0BBB:012B A30000        MOV	[0000],AX                          
0BBB:012E 89160200      MOV	[0002],DX                          
0BBB:0132 8BC2          MOV	AX,DX                              
0BBB:0134 80E47F        AND	AH,7F                              
0BBB:0137 C3            RET	                                   
-q
