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
//#define PROFILE

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern Thre_S * slow_sortByWeights          ( Thre_S * );
extern void     delete_sortedNode           ( Thre_S * );
extern int      Th_LocalMax                 ( Thre_S * , int , int );
extern int      Th_LocalMin                 ( Thre_S * , int , int );

// main function
Abc_Ntk_t*      Th_Ntk2Mux                  ( Vec_Ptr_t * , int , int );
// helper functions
static void         Th_Ntk2MuxCreatePio     ( Vec_Ptr_t * , Abc_Ntk_t * , Vec_Ptr_t * , Vec_Ptr_t * );
static void         Th_Ntk2MuxCreateMux     ( Vec_Ptr_t * , Abc_Ntk_t * , Vec_Ptr_t * , int , int );
static Abc_Obj_t*   Th_Node2Mux             ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int );
static Abc_Obj_t*   Th_Node2Mux_rec         ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int );
static Abc_Obj_t*   Th_Node2MuxDyn_rec      ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int , int );
static Abc_Obj_t*   Th_Node2MuxAhead_rec    ( Vec_Ptr_t * , Thre_S * , Abc_Ntk_t * , int , int , int );
static int          Th_SelectVar            ( Thre_S * , int , int );
static int          Th_SelectVar_Ahead      ( Thre_S * , int , int , int );
static void         Th_Ntk2MuxFinalize      ( Vec_Ptr_t * , Vec_Ptr_t * );

/**Function*************************************************************

  Synopsis    [Main function to convert threshold ntk to mux trees.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
Th_Ntk2Mux( Vec_Ptr_t * thre_list , int fDynamic , int fAhead )
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
   Th_Ntk2MuxCreateMux  ( thre_list , pNtkMux , vTh , fDynamic , fAhead );
   Th_Ntk2MuxFinalize   ( thre_list , vPo );
  
#ifdef PROFILE
   if ( fDynamic ) printf( "  > Ntk2Mux : number of redundancy gates = %d (out of %d)\n" , thProfiler.numRedundancy , Vec_PtrSize( vTh ) );
#endif

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
                     Vec_Ptr_t * vTh       , int fDynamic , int fAhead )
{
   printf( "  > Ntk2Mux : create mux (dynamic = %s , ahead = %s) ...\n" , 
            ( fDynamic ) ? "yes" : "no" ,
            ( fAhead )   ? "yes" : "no" ) ;
   Thre_S * tObj;
   int i , j , sum;

#ifdef PROFILE
   if ( fDynamic ) thProfiler.numRedundancy = 0;
#endif
   Vec_PtrForEachEntry( Thre_S * , vTh , tObj , i ) 
   {
      tObj->pCopy = Th_Node2Mux( thre_list , tObj , pNtkMux , fDynamic , fAhead );
#ifdef PROFILE
      if ( fDynamic ) {
         sum = 0;
         for ( j = 0 ; j < Vec_IntSize( tObj->Fanins ) ; ++j ) 
            if ( thProfiler.redund[j] == 0 ) ++sum;
         if ( sum != 0 ) {
            ++thProfiler.numRedundancy;
            printf( "    >  Redundancy  (Id = %d)   fanins detected : %d out of %d\n" , tObj->Id , sum , Vec_IntSize( tObj->Fanins ) );
         }
         for ( j = 0 ; j < 50 ; ++j ) thProfiler.redund[j] = 0;
      }
#endif
   }
}

Abc_Obj_t*
Th_Node2Mux( Vec_Ptr_t * thre_list , Thre_S * tObj ,  Abc_Ntk_t * pNtkMux , int fDynamic , int fAhead )
{
   Thre_S    * tObjSort;
   Abc_Obj_t * pObjMux;

   tObjSort = slow_sortByWeights( tObj );

   if ( fAhead ) 
      pObjMux = Th_Node2MuxAhead_rec( thre_list , tObjSort , pNtkMux , tObjSort->thre , 0 , 
                                      Vec_IntSize( tObjSort->weights )-1 );
   else if ( fDynamic ) 
      pObjMux = Th_Node2MuxDyn_rec( thre_list , tObjSort , pNtkMux , tObjSort->thre , 0 , 
                                    Vec_IntSize( tObjSort->weights )-1 );
   else            
      pObjMux = Th_Node2Mux_rec( thre_list , tObjSort , pNtkMux , tObjSort->thre , 0 );
   
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

   curMax = Th_LocalMax( tObjSort , lvl , Vec_IntSize( tObjSort->weights )-1 );
   curMin = Th_LocalMin( tObjSort , lvl , Vec_IntSize( tObjSort->weights )-1 );

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
Th_Node2MuxDyn_rec( Vec_Ptr_t * list , Thre_S * tObjSort , Abc_Ntk_t * pNtkMux ,
                    int thre , int head , int tail )
{
   assert( head <= tail + 1 );
   Abc_Obj_t * pObjMux; 
   Thre_S    * tObjC;
   int curMax , curMin , curW , splitVar;

   curMax = Th_LocalMax( tObjSort , head , tail );
   curMin = Th_LocalMin( tObjSort , head , tail );

   if ( curMax <  thre ) return Abc_ObjNot( Abc_AigConst1( pNtkMux ) );
   if ( curMin >= thre ) return Abc_AigConst1( pNtkMux );
   
   splitVar = Th_SelectVar( tObjSort , head , tail );
#ifdef PROFILE
   thProfiler.redund[splitVar] = 1;
#endif
   assert( splitVar == head || splitVar == tail );
   if ( splitVar == head ) ++head;
   else --tail;
   curW     = Vec_IntEntry( tObjSort->weights , splitVar );
   tObjC    = Th_GetObjById( list , Vec_IntEntry( tObjSort->Fanins , splitVar ) );
   pObjMux  = Abc_AigMux( pNtkMux->pManFunc , tObjC->pCopy , 
                          Th_Node2MuxDyn_rec( list , tObjSort , pNtkMux , thre-curW , head , tail ) ,
                          Th_Node2MuxDyn_rec( list , tObjSort , pNtkMux , thre      , head , tail ) );
   return pObjMux;
}

int
Th_SelectVar( Thre_S * t , int head , int tail )
{
   if ( head == tail ) return head;
   if ( abs( Vec_IntEntry( t->weights , head ) ) > abs( Vec_IntEntry( t->weights , tail ) ) )
      return head;
   else return tail;
}

Abc_Obj_t*
Th_Node2MuxAhead_rec( Vec_Ptr_t * list , Thre_S * tObjSort , Abc_Ntk_t * pNtkMux ,
                      int thre , int head , int tail )
{
   assert( head <= tail + 1 );
   Abc_Obj_t * pObjMux; 
   Thre_S    * tObjC;
   int curMax , curMin , curW , splitVar;

   curMax = Th_LocalMax( tObjSort , head , tail );
   curMin = Th_LocalMin( tObjSort , head , tail );

   if ( curMax <  thre ) return Abc_ObjNot( Abc_AigConst1( pNtkMux ) );
   if ( curMin >= thre ) return Abc_AigConst1( pNtkMux );
   
   splitVar = Th_SelectVar_Ahead( tObjSort , head , tail , thre );
   assert( splitVar == head || splitVar == tail );
   if ( splitVar == head ) ++head;
   else --tail;
   curW     = Vec_IntEntry( tObjSort->weights , splitVar );
   tObjC    = Th_GetObjById( list , Vec_IntEntry( tObjSort->Fanins , splitVar ) );
   pObjMux  = Abc_AigMux( pNtkMux->pManFunc , tObjC->pCopy , 
                          Th_Node2MuxDyn_rec( list , tObjSort , pNtkMux , thre-curW , head , tail ) ,
                          Th_Node2MuxDyn_rec( list , tObjSort , pNtkMux , thre      , head , tail ) );
   return pObjMux;
}

int
Th_SelectVar_Ahead( Thre_S * t , int head , int tail , int thre )
{
   if ( head == tail ) return head;
   int curMax , curMin , curW , curTh , selHead , selTail;
   selHead = selTail = 0;
   // select head
   curMax = Th_LocalMax   ( t , head+1 , tail );
   curMin = Th_LocalMin   ( t , head+1 , tail );
   curW   = Vec_IntEntry  ( t->weights , head );
   //curTh  = thre;
   //if ( curMax < curTh || curMin >= curTh ) ++selHead;
   curTh  = thre - curW;
   if ( curMax < curTh || curMin >= curTh ) ++selHead;
   // select tail
   curMax = Th_LocalMax   ( t , head , tail-1 );
   curMin = Th_LocalMin   ( t , head , tail-1 );
   curW   = Vec_IntEntry  ( t->weights , tail );
   //curTh  = thre;
   //if ( curMax < curTh || curMin >= curTh ) ++selTail;
   curTh  = thre - curW;
   if ( curMax < curTh || curMin >= curTh ) ++selTail;

   if ( selHead > selTail ) return head;
   else if ( selTail > selHead ) return tail;
   else {
      // break tie by absolute value
      if ( abs( Vec_IntEntry( t->weights , head ) ) > abs( Vec_IntEntry( t->weights , tail ) ) )
         return head;
      else return tail;
   }
}

/**Function*************************************************************

  Synopsis    [Finalize : connect Po.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_Ntk2MuxFinalize( Vec_Ptr_t * thre_list , Vec_Ptr_t * vPo )
{
   printf( "  > Ntk2Mux : connect Po ...\n" );
   Thre_S * tObj , * tObjFin;
   int i;

   Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
   {
      tObjFin = Th_GetObjById( thre_list , Vec_IntEntry( tObj->Fanins , 0 ) );
      if ( tObj->thre == 1 )
         Abc_ObjAddFanin( tObj->pCopy , tObjFin->pCopy );  
      else
         Abc_ObjAddFanin( tObj->pCopy , Abc_ObjNot( tObjFin->pCopy ) );  
   }
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
