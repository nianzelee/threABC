/**CFile****************************************************************
 
  FileName    [threThPG.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Threshold network PG encoding.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [June 14, 2019.]

  Revision    [$Id: abc.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include "base/abc/abc.h"
#include "map/if/if.h"
#include "map/if/ifCount.h"
#include "threshold.h"
#include "bdd/extrab/extraBdd.h"
#include "misc/extra/extra.h"

//#define DEBUG
//#define CHECK
//#define PROFILE

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// main function
void Th_PBPGEncoding(Vec_Ptr_t*, int);
// helper functions
void Th_PGEncoding_rec(FILE*, Vec_Ptr_t*, Thre_S*, int);
void Th_PGWriteNode(FILE*, Thre_S*, int);

/**Function*************************************************************

  Synopsis    [Main function to for PG encoding.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_PBPGEncoding(Vec_Ptr_t * current_TList, int fPG)
{
   char *fileName = fPG ? "pg.opb" : "no_pg.opb";
   FILE *out      = fopen(fileName, "w");
   Thre_S *tObj, *tObjPo = NULL;
   int i;

	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
      tObj->pos = tObj->neg = 0;
	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
   {
      if (tObj->Type == Th_Po) {
         tObjPo = tObj;
         fprintf(out, "min: -1*x_%d;\n", tObj->Id);
         break;
      }
   }
   if (!tObjPo) {
      Abc_Print(-1, "There is no PO!\n");
      assert(0);
   }
   Th_PGEncoding_rec(out, current_TList, tObjPo, 1);
   if (!fPG) Th_PGEncoding_rec(out, current_TList, tObjPo, 0);
   fclose(out);
}

void
Th_PGEncoding_rec(FILE *out, Vec_Ptr_t *current_TList, Thre_S *tObj, int phase)
{
   Thre_S *tObjFin;
   int Entry, i, w;

   if (tObj->Type == Th_Pi) return;
   if (tObj->Type == Th_CONST1) {
      Abc_Print(-2, "CONST1 appears in the network!\n");
      assert(0);
   }
   if (phase == 1 && tObj->pos == 1) return;
   if (phase == 0 && tObj->neg == 1) return;

   Th_PGWriteNode(out, tObj, phase);
	Vec_IntForEachEntry(tObj->Fanins, Entry, i )
	{
      w = Vec_IntEntry(tObj->weights, i);
      tObjFin = Th_GetObjById(current_TList, Entry);
      Th_PGEncoding_rec(out, current_TList, tObjFin, phase == (w>0));
	}
   if (phase == 1) tObj->pos = 1;
   else            tObj->neg = 1;
}

void
Th_PGWriteNode(FILE *out, Thre_S *tObj, int phase)
{
   int Entry, w, i, M, m;
   int T = tObj->thre;
   if (phase == 1) {
      m = 0;
      Vec_IntForEachEntry(tObj->weights, w, i) if (w < 0) m += w;
      Vec_IntForEachEntry(tObj->Fanins, Entry, i)
      {
         w = Vec_IntEntry(tObj->weights, i);
         fprintf(out, "%s%d*x_%d ", w>0 ? "+" : "", w, Entry);
      }
      fprintf(out, "%d*x_%d >= %d;\n", m-T, tObj->Id, m);
   }
   else {
      M = 0;
      Vec_IntForEachEntry(tObj->weights, w, i) if (w > 0) M += w;
      Vec_IntForEachEntry(tObj->Fanins, Entry, i)
      {
         w = Vec_IntEntry(tObj->weights, i);
         fprintf(out, "%s%d*x_%d ", -w>0 ? "+" : "", -w, Entry);
      }
      fprintf(out, "+%d*x_%d >= %d;\n", -T+1+M, tObj->Id, 1-T);
   }
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
