#include "..\defs.h"
#include "..\midgard\midgard.h"
#include "algorithm.h"

void KMP_preprocess(uchar *x, uchar m, int16 kmpNext[]) _REENTRANT_  {
   int16 i, j;

   i = 0;
   j = kmpNext[0] = -1;
   while (i < m) {
      while (j > -1 && x[i] != x[j])
         j = kmpNext[j];
      i++;
      j++;
      if (x[i] == x[j])
         kmpNext[i] = kmpNext[j];
      else
         kmpNext[i] = j;
   }
}


//KMP(pattern, kmpstates, pattern_length, text, text_length)
int16 KMP_search(uchar *x, int16 kmpNext[], uchar m, uchar *y, uchar n) _REENTRANT_  {
   int16 i, j; //kmpNext[XSIZE];		//XSIZE kemungkinan m+1
   //int8 * kmpNext;
   //kmpNext = (int8 *)m_alloc((m+1) * sizeof(int8));
   /* Preprocessing */
   //preKmp(x, m, kmpNext);

   /* Searching */
   i = j = 0;
   while (j < n) {
      while ((i != 0xffff) && (x[i] != y[j])) {
         i = kmpNext[i];
		}
      i++;
      j++;
      if (i >= m) {
         //OUTPUT(j - i);
		 //m_free(kmpNext);
		return j-i;
         i = kmpNext[i];
      }
   }
   //m_free(kmpNext);
	return -1;
}



