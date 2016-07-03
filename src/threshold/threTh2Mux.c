/**CFile****************************************************************
 
  FileName    [threTh2Mux.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Threshold network to mux tree conversion.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [July 2, 2016.]

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

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern Thre_S * slow_sortByWeights          ( Thre_S * );
extern void     delete_sortedNode           ( Thre_S * );
extern int      Th_LocalMax                 ( Thre_S * , int );
extern int      Th_LocalMin                 ( Thre_S * , int );

// main function
Abc_Ntk_t*      Th_Ntk2Mux                  ( Vec_Ptr_t * , int );
// helper functions
static void         Th_Ntk2MuxCreatePio     ( Vec_Ptr_t * , Abc_Ntk_t * , Vec_Ptr_t * , Vec_Ptr_t * );
static void         Th_Ntk2MuxCreateMux     ( Vec_Ptr_t * , Abc_Ntk_t * , Vec_Ptr_t * , int );
static Abc_Obj_t*   Th_Node2Mux             ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int );
static Abc_Obj_t*   Th_Node2Mux_rec         ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int );
static Abc_Obj_t*   Th_Node2MuxDyn_rec      ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int );
static void         Th_Ntk2MuxFinalize      ( Vec_Ptr_t * , Abc_Ntk_t * , Vec_Ptr_t * );

/**Function*************************************************************

  Synopsis    [Main function to convert threshold ntk to mux trees.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
Th_Ntk2Mux( Vec_Ptr_t * thre_list , int fDynamic )
{
   char Buffer[1000];
   Abc_Ntk_t * pNtkMux;
	Vec_Ptr_t * vPi , * vPo , * vTh;
   Thre_S * tObj;
   int i;

   pNtkMux = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , "th2mux" );
   pNtkMux->pName = Extra_UtilStrsav( Buffer );
  
   vPi = Vec_PtrAlloc( 1024 );
	vPo = Vec_PtrAlloc( 1024 );
	vTh = Vec_PtrAlloc( 1024 );
	
   // thre_list must be topologically sorted!
   Vec_PtrForEachEntry( Thre_S * , thre_list , tObj , i )
	{
      if ( !tObj ) {
         printf( "[Warning] NULL object in the list!\n" );
         continue;
      }
      if      ( tObj->Type == Th_Pi   )  Vec_PtrPush( vPi , tObj );
      else if ( tObj->Type == Th_Po   )  Vec_PtrPush( vPo , tObj );
      else if ( tObj->Type == Th_Node )  Vec_PtrPush( vTh , tObj );
	}

   Th_Ntk2MuxCreatePio  ( thre_list , pNtkMux , vPi , vPo );
   Th_Ntk2MuxCreateMux  ( thre_list , pNtkMux , vTh , fDynamic );
   Th_Ntk2MuxFinalize   ( thre_list , pNtkMux , vPo );
   
	Vec_PtrFree( vPi );
	Vec_PtrFree( vPo );
	Vec_PtrFree( vTh );
   
   if ( !Abc_NtkCheck( pNtkMux ) ) {
      printf( "Th_Ntk2Mux() : The network check has failed.\n" );
      Abc_NtkDelete( pNtkMux );
      return NULL;
   }
   return pNtkMux;
}

/**Function*************************************************************

  Synopsis    [Create Pi and Po for the mux network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_Ntk2MuxCreatePio( Vec_Ptr_t * thre_list , Abc_Ntk_t * pNtkMux ,
                     Vec_Ptr_t * vPi       , Vec_Ptr_t * vPo )
{
   printf( "  > Ntk2Mux : create pio ...\n" );
   Thre_S    * tObj;
   char Buffer[1000];
   int i;

   tObj        = (Thre_S*)Vec_PtrEntry( thre_list , 0 );
   assert( tObj->Type == Th_CONST1 );
   tObj->pCopy = Abc_AigConst1( pNtkMux );

   Vec_PtrForEachEntry( Thre_S * , vPi , tObj , i )
   {
      tObj->pCopy = Abc_NtkCreatePi( pNtkMux );
      sprintf( Buffer , "muxPi-%d" , i );
      Abc_ObjAssignName( tObj->pCopy , Buffer , NULL );
   }
   
   Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
   {
      tObj->pCopy = Abc_NtkCreatePo( pNtkMux );
      sprintf( Buffer , "muxPo-%d" , i );
      Abc_ObjAssignName( tObj->pCopy , Buffer , NULL );
   }
}

/**Function*************************************************************

  Synopsis    [Create mux.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_Ntk2MuxCreateMux( Vec_Ptr_t * thre_list , Abc_Ntk_t * pNtkMux ,
                     Vec_Ptr_t * vTh       , int fDynamic )
{
   printf( "  > Ntk2Mux : create mux (dynamic = %s) ...\n" , ( fDynamic ) ? "yes" : "no" );
   Thre_S * tObj;
   int i;

   Vec_PtrForEachEntry( Thre_S * , vTh , tObj , i )
      tObj->pCopy = Th_Node2Mux( thre_list , tObj , pNtkMux , fDynamic );
}

Abc_Obj_t*
Th_Node2Mux( Vec_Ptr_t * thre_list , Thre_S * tObj ,  Abc_Ntk_t * pNtkMux , int fDynamic )
{
   Thre_S    * tObjSort;
   Abc_Obj_t * pObjMux;

   tObjSort = slow_sortByWeights( tObj );
   if ( fDynamic ) pObjMux  = Th_Node2MuxDyn_rec  ( thre_list , tObjSort , pNtkMux , tObjSort->thre , 0 ); // TODO
   else            pObjMux  = Th_Node2Mux_rec     ( thre_list , tObjSort , pNtkMux , tObjSort->thre , 0 );
   delete_sortedNode( tObjSort );
   if ( !pObjMux ) {
      printf( "[Error] convert Th node to mux trees fail.\n" );
      return NULL;
   }
   return pObjMux;
}

Abc_Obj_t*
Th_Node2Mux_rec( Vec_Ptr_t * list , Thre_S * tObjSort , Abc_Ntk_t * pNtkMux , int thre , int lvl )
{
   assert( lvl <= Vec_IntSize( tObjSort->Fanins ) );
   Abc_Obj_t * pObjMux; 
   Thre_S    * tObjC;
   int curMax , curMin , curW;

   curMax = Th_LocalMax( tObjSort , lvl );
   curMin = Th_LocalMin( tObjSort , lvl );

   if ( curMax <  thre ) return Abc_ObjNot( Abc_AigConst1( pNtkMux ) );
   if ( curMin >= thre ) return Abc_AigConst1( pNtkMux );
   
   curW    = Vec_IntEntry( tObjSort->weights , lvl );
   tObjC   = Th_GetObjById( list , Vec_IntEntry( tObjSort->Fanins , lvl ) );
   pObjMux = Abc_AigMux( pNtkMux->pManFunc , tObjC->pCopy , 
                         Th_Node2Mux_rec( list , tObjSort , pNtkMux , thre-curW , lvl+1 ) ,
                         Th_Node2Mux_rec( list , tObjSort , pNtkMux , thre      , lvl+1 ) );
   return pObjMux;
}

Abc_Obj_t*
Th_Node2MuxDyn_rec( Vec_Ptr_t * list , Thre_S * tObjSort , Abc_Ntk_t * pNtkMux , int thre , int lvl )
{
   return NULL;
#if 0
   assert( lvl <= Vec_IntSize( tObjSort->Fanins ) );
   Abc_Obj_t * pObjMux; 
   Thre_S    * tObjC;
   int curMax , curMin , curW;

   curMax = Th_LocalMax( tObjSort , lvl );
   curMin = Th_LocalMin( tObjSort , lvl );

   if ( curMax <  thre ) return Abc_ObjNot( Abc_AigConst1( pNtkMux ) );
   if ( curMin >= thre ) return Abc_AigConst1( pNtkMux );
   
   curW    = Vec_IntEntry( tObjSort->weights , lvl );
   tObjC   = Th_GetObjById( list , Vec_IntEntry( tObjSort->Fanins , lvl ) );
   pObjMux = Abc_AigMux( pNtkMux->pManFunc , tObjC->pCopy , 
                         Th_Node2Mux_rec( list , tObjSort , pNtkMux , thre-curW , lvl+1 ) ,
                         Th_Node2Mux_rec( list , tObjSort , pNtkMux , thre      , lvl+1 ) );
   return pObjMux;
#endif
}

/**Function*************************************************************

  Synopsis    [Finalize : connect Po.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_Ntk2MuxFinalize( Vec_Ptr_t * thre_list , Abc_Ntk_t * pNtkMux ,
                    Vec_Ptr_t * vPo )
{
   printf( "  > Ntk2Mux : connect Po ...\n" );
   Thre_S * tObj , * tObjFin;
   int i;

   Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
   {
      tObjFin = Th_GetObjById( thre_list , Vec_IntEntry( tObj->Fanins , 0 ) );
      if ( Vec_IntEntry( tObj->weights , 0 ) == 1 )
         Abc_ObjAddFanin( tObj->pCopy , tObjFin->pCopy );  
      else
         Abc_ObjAddFanin( tObj->pCopy , Abc_ObjNot( tObjFin->pCopy ) );  
   }
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
