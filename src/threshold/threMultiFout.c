/**CFile****************************************************************
 
  FileName    [threMultiFout.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Deal with multi-fanout collapsing.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [Mar 11, 2016.]

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

//#define CHECK
//#define DEBUG

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern  Thre_S* Th_GetObjById       ( Vec_Ptr_t * , int );
// main functions
int     Th_Check2FoutCollapse       ( const Thre_S * , const Thre_S * , int );
int     Th_CheckMultiFoutCollapse   ( const Thre_S * , int );
// helper functions
Thre_S* Th_2FoutGetOther            ( const Thre_S * , const Thre_S * );
int     Th_ObjFanoutFaninNum        ( const Thre_S * , const Thre_S * );
int     Th_CheckPairCollapse        ( const Thre_S * , const Thre_S * , int );
int     Th_KLClpCheck               ( const Thre_S * , const Thre_S * , const Pair_S * , int , int );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Check if 2 fanout collapse is feasible.]

  Description [tObj1 has two fanouts tObj2 and tObj3.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Th_Check2FoutCollapse( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin21 )
{
	assert( Vec_IntSize(tObj1->Fanouts) == 2 );
	Thre_S * tObj3;
	int nFanin31;

   tObj3    = Th_2FoutGetOther( tObj1 , tObj2 );
	assert(tObj3);
	if ( tObj3->nId == globalRef || tObj3->Type != Th_Node ) return 0;
	nFanin31 = Th_ObjFanoutFaninNum( tObj1 , tObj3 );
	assert( nFanin31 >=0 && nFanin31 < Vec_IntSize(tObj3->Fanins) );
   return ( Th_CheckPairCollapse( tObj1 , tObj2 , nFanin21 ) &&
		      Th_CheckPairCollapse( tObj1 , tObj3 , nFanin31 ) );

}

Thre_S*
Th_2FoutGetOther( const Thre_S * tObj1 , const Thre_S * tObj2 )
{
	int tObj3Id;

	tObj3Id = ( Vec_IntEntry(tObj1->Fanouts , 0) == tObj2->Id ) ? 
		         Vec_IntEntry(tObj1->Fanouts , 1) : Vec_IntEntry(tObj1->Fanouts , 0);
	//printf( "tObj1 Id = %d , tObj2 Id = %d , tObj3 Id = %d\n" , tObj1->Id , tObj2->Id , tObj3Id );
	return Th_GetObjById( current_TList , tObj3Id );
}

int
Th_ObjFanoutFaninNum( const Thre_S * tObj1 , const Thre_S * tObj2 )
{
	int Entry , i;
	Vec_IntForEachEntry( tObj2->Fanins , Entry , i )
		if ( Entry == tObj1->Id ) return i;
	printf( "Th_ObjFanoutFaninNum() : tObj1(Id=%d) is not a fanin of tObj2(Id=%d)\n" , tObj1->Id , tObj2->Id );
	assert(0);
	return Vec_IntSize( tObj2->Fanins );
}

/**Function*************************************************************

  Synopsis    [Check if a pair of nodes can be collapsed.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Th_CheckPairCollapse( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin )
{
	Thre_S * tObjInv;
	Pair_S * pair;
	int w , fInvert , RetValue;
	
	tObjInv       = tObj1;
	w             = Vec_IntEntry( tObj2->weights , nFanin );
	fInvert       = 0;
	RetValue      = 0;
	
	if ( w < 0 ) {
	   tObjInv      = Th_InvertObj(tObj1);
		w           *= -1;
      fInvert      = 1;
	}

	//pair = Th_CalKLDP( tObjInv , tObj2 , nFanin , w , fInvert );
	pair = Th_CalKLIf( tObjInv , tObj2 , nFanin , w , fInvert );
   
	if ( pair && pair->IntK > 0 && pair->IntL > 0 && 
	     Th_KLClpCheck( tObjInv , tObj2 , pair , w , fInvert ) ) {
		RetValue = 1; 
	}
   if ( fInvert ) Th_DeleteObjNoInsert( tObjInv ); // delete the inverted object created in this function
	ABC_FREE( pair );

	return RetValue;
}

int 
Th_KLClpCheck( const Thre_S * tObj1 , const Thre_S * tObj2 , const Pair_S * pair , int w , int fInvert )
{	
	Thre_S * tObjMerge;
	int T1 , T2 , fMark , Entry , i , limit;
   
	limit = 255;
	T1    = tObj1->thre;
	T2    = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);

	tObjMerge = Th_CreateObjNoInsert( Th_Node );
	tObjMerge->thre = pair->IntK * T1 + pair->IntL * (T2 - w);
	if ( tObjMerge->thre > limit || tObjMerge->thre < -limit ) {
	   Th_DeleteObjNoInsert( tObjMerge );
		return 0;
	}
	// set merged weights and fanins , tObj1 part
	Vec_IntForEachEntry( tObj1->weights , Entry , i )
		Vec_IntPush( tObjMerge->weights , pair->IntK * Entry );
	Vec_IntForEachEntry( tObj1->Fanins , Entry , i )
		Vec_IntPush( tObjMerge->Fanins , Entry );

	// set merged weights and fanins , tObj2 part , take care of joint fanins
	Vec_IntForEachEntry( tObj2->Fanins , Entry , i )
	{
		if ( Entry == tObj1->Id ) continue;
      fMark = Th_ObjIsFanin( tObj1 , Entry );
		if ( fMark > -1 ) { // joint fanin , sum weights up
		   Vec_IntWriteEntry( tObjMerge->weights , fMark , Vec_IntEntry( tObjMerge->weights , fMark ) + 
					                                          pair->IntL * Vec_IntEntry( tObj2->weights , i ));
		}
		else {
			Vec_IntPush( tObjMerge->weights , pair->IntL * Vec_IntEntry( tObj2->weights , i ) );
         Vec_IntPush( tObjMerge->Fanins  , Entry );
		}
   }
	Vec_IntForEachEntry( tObjMerge->weights , Entry , i )
	{
	   if ( Entry > limit || Entry < -limit ) {
	      Th_DeleteObjNoInsert( tObjMerge );
		   return 0;
		}
	}
	Th_DeleteObjNoInsert( tObjMerge );
	return 1;
}

/**Function*************************************************************

  Synopsis    [Check if multi fanout collapse is feasible.]

  Description [tObj1 has multi fanouts; return true if all can be collapsed.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Th_CheckMultiFoutCollapse( const Thre_S * tObj1 , int fOutBound )
{
	// controlling multi-fanout number
	//int foutBound = 30;
	if ( fOutBound == -1 ); // -1 --> no limit
	else if ( Vec_IntSize( tObj1->Fanouts ) > fOutBound ) return 0;

	Thre_S * tObj2;
	int RetValue , nFanin , Entry , i;
	RetValue = 1;

	Vec_IntForEachEntry( tObj1->Fanouts , Entry , i )
	{
		tObj2 = Th_GetObjById( current_TList , Entry );
		assert(tObj2);
	   nFanin = Th_ObjFanoutFaninNum( tObj1 , tObj2 );
	   assert( nFanin >=0 && nFanin < Vec_IntSize(tObj2->Fanins) );
		if ( tObj2->nId == globalRef || tObj2->Type != Th_Node || !Th_CheckPairCollapse(tObj1 , tObj2 , nFanin) ) {
		   RetValue = 0;
			break;
		}
	}
	return RetValue;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
