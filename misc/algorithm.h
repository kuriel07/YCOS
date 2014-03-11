#include "..\defs.h"
#ifndef ALGORITHM_H

//for use with KMP_search, KMP(pattern, pattern_length, kmp_states)
void KMP_preprocess(uchar *x, uchar m, int16 kmpNext[]) _REENTRANT_ ;
//KMP(pattern, kmp_states, pattern_length + 1, text, text_length)
int16 KMP_search(uchar *x, int16 kmpNext[], uchar m, uchar *y, uchar n) _REENTRANT_ ;

#define ALGORITHM_H
#endif