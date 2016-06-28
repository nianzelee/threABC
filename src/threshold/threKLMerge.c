/**CFile****************************************************************
 
  FileName    [threKLMerge.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Collapse threshold network.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [Mar 9, 2016.]

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

#define CHECK
#define PROFILE

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern Pair_S*    Th_CalKLDP                  ( const Thre_S * , const Thre_S * , int , int , int );
extern int        Th_ObjIsConst               ( const Thre_S * );
extern Thre_S*    Th_GetObjById               ( Vec_Ptr_t * , int );
extern int        Th_Check2FoutCollapse       ( const Thre_S * , const Thre_S * , int );
extern int        Th_CheckMultiFoutCollapse   ( const Thre_S * );
// main functions
void       mergeThreNtk_Iter      ( Vec_Ptr_t * , int );
// main helper functions for merge
Thre_S*    Th_MergeNodes          ( Thre_S *    , int );
Thre_S*    Th_CalKLMerge          ( const Thre_S * , const Thre_S * , int );
Pair_S*    Th_CalKL               ( const Thre_S * , const Thre_S * , int , int , int );
Thre_S*    Th_KLMerge             ( const Thre_S * , const Thre_S * , const Pair_S * , int , int );
Thre_S*    Th_KLCreateClpObj      ( const Thre_S * , const Thre_S * , const Pair_S * , int , int );
int        Th_ObjIsFanin          ( const Thre_S * , int );
void       Th_KLPatchFanio        ( const Thre_S * , const Thre_S * , const Thre_S * );
// dumper functions
void       Th_DumpObj             ( const Thre_S * );
void       Th_DumpMergeObj        ( const Thre_S * , const Thre_S * , const Thre_S *);
// constructor/destructor
Thre_S*    Th_CreateObjNoInsert   ( Th_Gate_Type );
void       Th_DeleteObjNoInsert   ( Thre_S * );
void       deleteNode             ( Vec_Ptr_t * , Thre_S * );
// other helper functions
void       Th_UnmarkAllNode       ();
Vec_Ptr_t* Th_CopyList            ( Vec_Ptr_t * );
Thre_S*    Th_InvertObj           ( const Thre_S* );
Thre_S*    Th_CopyObj             ( const Thre_S* );
int        MaxF                   ( Vec_Int_t * , int );
int        MinF                   ( Vec_Int_t * , int );
void       Th_ObjNormalCheck      ( const Thre_S * );

/**Function*************************************************************

  Synopsis    [Dumping functions for debugging.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void 
Th_DumpMergeObj( const Thre_S * tObj1 , const Thre_S * tObj2 , const Thre_S * tObjMerge )
{
   printf("tObj1\n");
	Th_DumpObj(tObj1);

   printf("tObj2\n");
	Th_DumpObj(tObj2);

   printf("tObjMerge\n");
	Th_DumpObj(tObjMerge);
}

void 
Th_DumpObj( const Thre_S * tObj )
{
	int Entry , i;

	printf( "\tDump object %d... \n" , tObj->Id );
	
	printf( "\tDump fanouts...\n" );

	Vec_IntForEachEntry( tObj->Fanouts , Entry , i )
	{
		printf( "%d " , Entry );
	}
	
	printf( "\n\tDump fanins...\n" );

	Vec_IntForEachEntry( tObj->Fanins , Entry , i )
	{
		printf( "%d " , Entry );
	}
	
	printf( "\n\tDump weights and threshold...\n" );
	
	Vec_IntForEachEntry( tObj->weights , Entry , i )
	{
		printf( "%d " , Entry );
	}
	printf( "%d\n" , tObj->thre );
	
}

/**Function*************************************************************

  Synopsis    [Constructor and destructor for collapsing.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Thre_S* 
Th_CreateObjNoInsert( Th_Gate_Type Type )
{
   Thre_S * tObj    = ABC_ALLOC( Thre_S , 1 );
	tObj->thre       = 0;
	tObj->Type       = Type;
	tObj->Ischoose   = 0;
   tObj->Id         = -1; // no insert into TList
	tObj->oId        = 0;
	tObj->nId        = 0;
	tObj->cost       = 0;
	tObj->level      = 0;
	tObj->pName      = NULL;
	tObj->weights    = Vec_IntAlloc(16);
	tObj->Fanins     = Vec_IntAlloc(16);
	tObj->Fanouts    = Vec_IntAlloc(16);

	return tObj;
}

void 
Th_DeleteObjNoInsert( Thre_S * tObj )
{
	if ( tObj->pName ) ABC_FREE( tObj->pName );
   Vec_IntFree( tObj->weights );
   Vec_IntFree( tObj->Fanins );
   Vec_IntFree( tObj->Fanouts );
   ABC_FREE( tObj );
}

void 
deleteNode(Vec_Ptr_t* list, Thre_S * tObj)
{  
   if (tObj->pName) ABC_FREE(tObj->pName);
   Vec_IntFree(tObj->weights);
   Vec_IntFree(tObj->Fanins);
   Vec_IntFree(tObj->Fanouts);
   Vec_PtrWriteEntry( list, tObj->Id, NULL );
   ABC_FREE( tObj );
}

/**Function*************************************************************

  Synopsis    [Starting point to collapse two nodes (interface).]

  Description [Consider single fanout only.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Thre_S* 
Th_MergeNodes( Thre_S * tObj2 , int nFanin )
{
   assert( tObj2 );
	assert( nFanin >= 0 && nFanin < Vec_IntSize(tObj2->Fanins) );

	Thre_S * tObj1;

	tObj1 = Th_GetObjById( current_TList , Vec_IntEntry( tObj2->Fanins, nFanin ) );
	assert( tObj1 );
#ifdef PROFILE
	++thProfiler.numTotal;
	if ( tObj1->Type != Th_Node ) {
	   ++thProfiler.numNotThNode;
		return NULL;
	}
	if ( Vec_IntSize(tObj1->Fanouts) > 1 ) {
	   ++thProfiler.numMultiFout;
      //if ( Vec_IntSize(tObj1->Fanouts) > 2 && Th_CheckMultiFoutCollapse(tObj1) ) ++thProfiler.numMultiFoutOk;
      if ( Th_CheckMultiFoutCollapse(tObj1) ) ++thProfiler.numMultiFoutOk;
		/*if ( Vec_IntSize(tObj1->Fanouts) == 2 ) {
			++thProfiler.numTwoFout;
			if ( Th_Check2FoutCollapse( tObj1 , tObj2 , nFanin ) ) ++thProfiler.numTwoFoutOk;
		}*/
		return NULL;
	}
	else return Th_CalKLMerge( tObj1 , tObj2 , nFanin );
#else
	if ( tObj1->Type != Th_Node || Vec_IntSize(tObj1->Fanouts) > 1 )
		return NULL;
	else return Th_CalKLMerge( tObj1 , tObj2 , nFanin );
#endif
}

/**Function*************************************************************

  Synopsis    [Other utilities for collapsing.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Vec_Ptr_t* 
Th_CopyList( Vec_Ptr_t * tList )
{
    Vec_Ptr_t * copyList;
    Thre_S    * tObj , * tObjCopy;
    int i;

    copyList = Vec_PtrAlloc(16);

    Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
    {
        assert( tObj );
        tObjCopy = Th_CopyObj( tObj );
        Vec_PtrPush( copyList , tObjCopy );
    }
    
    return copyList;
}

Thre_S* 
Th_CopyObj( const Thre_S * tObj )
{
   int Entry , i;
	Thre_S * tObjCopy;
   
	tObjCopy             = ABC_ALLOC( Thre_S , 1 );
	tObjCopy->thre       = tObj->thre;
	tObjCopy->Type       = tObj->Type;
	tObjCopy->Ischoose   = tObj->Ischoose;
   tObjCopy->Id         = tObj->Id;
	tObjCopy->oId        = tObj->oId;
	tObjCopy->nId        = tObj->nId;
	tObjCopy->cost       = tObj->cost;
	tObjCopy->level      = tObj->level;
	tObjCopy->pName      = tObj->pName;
	tObjCopy->weights    = Vec_IntAlloc(16);
	tObjCopy->Fanins     = Vec_IntAlloc(16);
	tObjCopy->Fanouts    = Vec_IntAlloc(16);
	
	Vec_IntForEachEntry( tObj->weights , Entry , i )
	{
		Vec_IntPush( tObjCopy->weights , Entry );
	}
	Vec_IntForEachEntry( tObj->Fanins , Entry , i )
	{
		Vec_IntPush( tObjCopy->Fanins , Entry );
	}
	Vec_IntForEachEntry( tObj->Fanouts , Entry , i )
	{
		Vec_IntPush( tObjCopy->Fanouts , Entry );
	}
	return tObjCopy;
}

Thre_S* 
Th_InvertObj( const Thre_S * tObj )
{
	int Entry , i;
	Thre_S * tObjInvert = Th_CopyObj( tObj );
   
	// invert weights
	Vec_IntForEachEntry( tObjInvert->weights , Entry , i )
	{
		Vec_IntWriteEntry( tObjInvert->weights , i , -Entry );
	}
	// invert threshold
	tObjInvert->thre = 1 - tObjInvert->thre;
   return tObjInvert;
}

int 
MaxF(Vec_Int_t * weights , int nFanin )
{
	int Entry , i , sum;
	sum = 0;

   Vec_IntForEachEntry( weights , Entry , i )
	{
      if ( i != nFanin && Entry > 0 ) sum += Entry;
	}
	return sum;
}

int 
MinF(Vec_Int_t * weights , int nFanin )
{
	int Entry , i , sum;
	sum = 0;

   Vec_IntForEachEntry( weights , Entry , i )
	{
      if ( i != nFanin && Entry < 0 ) sum += Entry;
	}
	return sum;
}

void 
Th_UnmarkAllNode()
{
   ++globalRef;
}

void
Th_ObjNormalCheck( const Thre_S * tObj )
{
	int Entry , i;
	if ( Th_ObjIsConst(tObj) ) printf( "tObj id=%d is const\n" , tObj->Id );
   Vec_IntForEachEntry( tObj->weights , Entry , i )
		if ( Entry == 0 ) printf( "tObj %d-th fanin has 0 weight\n" , i );
}

/**Function*************************************************************

  Synopsis    [Compute KL pair by if conditions.]

  Description [Refactor version : in threCalKL.c]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Pair_S* 
Th_CalKL( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin , int w , int fInvert )
{
#if 1
	printf( "Please use the refactored version Th_CalKLIf()\n" );
	return NULL;
#else
	Pair_S * pair;
	int T1 , T2 , condition , maxf2 , minf2 , n1 , n2 , n3 , n4 , cond1 , cond2;

	pair          = ABC_ALLOC( Pair_S , 1 );
	pair->IntK    = 0;
	pair->IntL    = 0;
	T1            = tObj1->thre;
	T2            = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);
	maxf2         = MaxF( tObj2->weights , nFanin );
	minf2         = MinF( tObj2->weights , nFanin );
	n1            = MaxF( tObj1->weights , Vec_IntSize( tObj1->weights ) ) - T1; 
	n2            = w - 1;
	n3            = T1 - MinF( tObj1->weights , Vec_IntSize( tObj1->weights ) );
	n4            = w;
	cond1         = 0;
	cond2         = 0;
	
	// 03/05 remark : 
	// assume cond3 must be satisfied, OK?
	// if cond3 = 0 --> no connection between obj1 and obj2
   condition     = T2 - w - 1;
	if ( minf2 <= condition ) cond1 = 1;

	condition     = T2 ;
	if ( maxf2 >= condition ) cond2 = 1;
	
	// K , L computation
	if( cond1 == 1 && cond2 == 1 ) { //1&2&3
		int N = 1 - n1 * n2;  // indicating 1 and 2 intersect
		int M = n4 - n2 * n3; // indicating 3 and (1,2) intersect
		if ( N > 0 && M > 0) {
			double intersectL = (n1 + 1) / N;
			double intersectK = n2 * intersectL + 1;
			double slope = intersectK / intersectL;
         if ( (double)n4 >= ((double)n3) * slope ) {
			   pair->IntL = (n1 + 1) / N + 1;
				pair->IntK = n2 * pair->IntL + 1;
			}
			else {
			   pair->IntL = n3 / M + 1;
				pair->IntK = n2 * pair->IntL + 1;
			}
#ifdef CHECK
		assert( pair->IntK > 0  && pair->IntL > 0 );
		assert( pair->IntL      >= pair->IntK * n1 + 1 ); // 1
		assert( pair->IntK      >= pair->IntL * n2 + 1 ); // 2
		assert( pair->IntL * n4 >= pair->IntK * n3    );  // 3
#endif
		}
	}
	else if( cond1 == 1 && cond2 == 0 ) {     //1&2
		int N = 1 - n1*n2;
		if( N > 0 ) {
			pair->IntL = ( (n1 + 1) / N ) + 1;
			pair->IntK = pair->IntL * n2  + 1;
#ifdef CHECK
		assert( pair->IntK > 0  && pair->IntL > 0 );
		assert( pair->IntL      >= pair->IntK * n1 + 1 ); // 1
		assert( pair->IntK      >= pair->IntL * n2 + 1 ); // 2
#endif
		}	
	}
	else if( cond1 == 0 && cond2 == 1 ) {	//2&3
		int N = n4 - n2*n3;
		if( N > 0 ) {
			pair->IntL = n3/N + 1;
			pair->IntK = pair->IntL * n2 + 1;
#ifdef CHECK
		assert( pair->IntK > 0  && pair->IntL > 0 );
		assert( pair->IntK      >= pair->IntL * n2 + 1);  // 2
		assert( pair->IntL * n4 >= pair->IntK * n3    );  // 3
#endif
		}
	}
	else {				//2
		pair->IntL = 1;
		pair->IntK = n2 + 1;
#ifdef CHECK
		assert( pair->IntK > 0  && pair->IntL > 0 );
		assert( pair->IntK      >= pair->IntL * n2 + 1);  // 2
#endif
	}

	return pair;
#endif
}

/**Function*************************************************************

  Synopsis    [Collapse two nodes based on the KL pair.]

  Description [Assign threshold and weights;modify fanin, fanouts.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Thre_S* 
Th_KLMerge( const Thre_S * tObj1 , const Thre_S * tObj2 , const Pair_S * pair , int w , int fInvert )
{	
	Thre_S * tObjMerge;

	tObjMerge = Th_KLCreateClpObj( tObj1 , tObj2 , pair , w , fInvert );
   Th_KLPatchFanio( tObj1 , tObj2 , tObjMerge );
	return tObjMerge;
}

Thre_S*
Th_KLCreateClpObj( const Thre_S * tObj1 , const Thre_S * tObj2 , 
		             const Pair_S * pair , int w , int fInvert )
{
	Thre_S * tObjMerge;
	int T1 , T2 , fMark , Entry , i;
   
	T1 = tObj1->thre;
	T2 = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);

	tObjMerge = Th_CreateObj( current_TList , Th_Node );
	tObjMerge->thre = pair->IntK * T1 + pair->IntL * (T2 - w);
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
	// connect fanouts , tObjMerge part
	Vec_IntForEachEntry( tObj2->Fanouts , Entry , i )
		Vec_IntPush( tObjMerge->Fanouts , Entry );
	
	return tObjMerge;
}

int
Th_ObjIsFanin( const Thre_S * tObj , int faninId )
{
   int Entry , i;

	Vec_IntForEachEntry( tObj->Fanins , Entry , i )
      if ( Entry == faninId ) return i;
	
	return -1;
}

void
Th_KLPatchFanio( const Thre_S * tObj1 , const Thre_S * tObj2 , const Thre_S * tObjMerge )
{
	Thre_S * tObjFanin , * tObjFanout;
	int nFanin , Entry , i;
	// connect fanins , tObj2 fanout part
	Vec_IntForEachEntry( tObj2->Fanouts , Entry , i )
	{
      tObjFanout = Th_GetObjById( current_TList , Entry );
	   assert(tObjFanout);
		// Unmark a node if some of its fanins are merged
		if ( tObjFanout->nId == globalRef ) --(tObjFanout->nId);
	   nFanin = Th_ObjFanoutFaninNum( tObj2 , tObjFanout );
	   Vec_IntWriteEntry( tObjFanout->Fanins , nFanin , tObjMerge->Id );
	   assert( Vec_IntSize(tObjFanout->Fanins) == Vec_IntSize(tObjFanout->weights) );
	}
	// connect fanouts , tObj1 fanin part 
	Vec_IntForEachEntry( tObj1->Fanins , Entry , i )
	{
      tObjFanin = Th_GetObjById( current_TList , Entry );
	   assert(tObjFanin);
		Vec_IntRemove( tObjFanin->Fanouts , tObj1->Id );
		Vec_IntPush  ( tObjFanin->Fanouts , tObjMerge->Id );
	}
	// connect fanouts , tObj2 fanin part 
	Vec_IntForEachEntry( tObj2->Fanins , Entry , i )
	{
	   if ( Entry == tObj1->Id ) continue;
      tObjFanin = Th_GetObjById( current_TList , Entry );
	   assert(tObjFanin);
		Vec_IntRemove( tObjFanin->Fanouts , tObj2->Id );
		Vec_IntPush  ( tObjFanin->Fanouts , tObjMerge->Id );
	}
	assert( Vec_IntSize(tObjMerge->Fanins) == Vec_IntSize(tObjMerge->weights) );
}

/**Function*************************************************************

  Synopsis    [Collapse two nodes based on the KL pair.]

  Description [Modify fanin, fanouts.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
/*
Thre_S* 
Th_KLMerge( const Thre_S * tObj1 , const Thre_S * tObj2 , const Pair_S * pair , int w , int fInvert )
{
	Thre_S * tObjMerge , * tObjFanin , * tObjFanout;
	int T1 , T2;
	int i , j , fMark , Entry1 , Entry2; // for iterator
   
	T1 = tObj1->thre;
	T2 = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);

	// start merging ...
	tObjMerge = Th_CreateObj( current_TList , Th_Node );
	// set merged threshold
	tObjMerge->thre = pair->IntK * T1 + pair->IntL * (T2 - w);
	// set merged weights , tObj1 part
	Vec_IntForEachEntry( tObj1->weights , Entry1 , i )
	{
		Vec_IntPush( tObjMerge->weights , pair->IntK * Entry1 );
	}
	// set merged fanins , tObj1 part
	Vec_IntForEachEntry( tObj1->Fanins , Entry1 , i )
	{
		Vec_IntPush( tObjMerge->Fanins , Entry1 );
	}
	// set merged weights and fanins , tObj2 part , take care of joint fanins
	Vec_IntForEachEntry( tObj2->Fanins , Entry2 , j )
	{
		if ( Entry2 == tObj1->Id ) continue;
		fMark = -1;
		Vec_IntForEachEntry( tObj1->Fanins , Entry1 , i )
		{
			if ( Entry1 == Entry2 ) { // joint fanin of tObj1 and tObj2
			   fMark = i;
				break;
			}
		}
		if ( fMark > -1 ) { // joint fanin
		   Vec_IntWriteEntry( tObjMerge->weights , fMark , Vec_IntEntry( tObjMerge->weights , fMark ) + 
					                                          pair->IntL * Vec_IntEntry( tObj2->weights , j ));
		}
		else {
			Vec_IntPush( tObjMerge->weights , pair->IntL * Vec_IntEntry( tObj2->weights , j ) );
         Vec_IntPush( tObjMerge->Fanins  , Entry2 );
		}
   }
	// connect fanouts , tObjMerge part
	Vec_IntForEachEntry( tObj2->Fanouts , Entry2 , j )
	{
		Vec_IntPush( tObjMerge->Fanouts , Entry2 );
	}
	// connect fanins , tObj2 fanout part
	Vec_IntForEachEntry( tObj2->Fanouts , Entry2 , j )
	{
      tObjFanout = Th_GetObjById( current_TList , Entry2 );
		// Unmark a node if some of its fanins are merged
		if ( tObjFanout->nId == globalRef ) --(tObjFanout->nId);
	   assert(tObjFanout);
		Vec_IntForEachEntry( tObjFanout->Fanins , Entry1 , i )
		{
			if ( Entry1 != tObj2->Id ) continue;
			else {
				Vec_IntWriteEntry( tObjFanout->Fanins , i , tObjMerge->Id );
				break;
			}
		}
	   assert( Vec_IntSize(tObjFanout->Fanins) == Vec_IntSize(tObjFanout->weights) );
	}
	// connect fanouts , tObj1 fanin part 
	Vec_IntForEachEntry( tObj1->Fanins , Entry1 , i )
	{
      tObjFanin = Th_GetObjById( current_TList , Entry1 );
	   assert(tObjFanin);
		Vec_IntRemove( tObjFanin->Fanouts , tObj1->Id );
		Vec_IntPush  ( tObjFanin->Fanouts , tObjMerge->Id );
	}
	// connect fanouts , tObj2 fanin part 
	Vec_IntForEachEntry( tObj2->Fanins , Entry2 , j )
	{
	   if ( Entry2 == tObj1->Id ) continue;
      tObjFanin = Th_GetObjById( current_TList , Entry2 );
	   assert(tObjFanin);
		Vec_IntRemove( tObjFanin->Fanouts , tObj2->Id );
		Vec_IntPush  ( tObjFanin->Fanouts , tObjMerge->Id );
	}
   
	assert( Vec_IntSize(tObjMerge->Fanins) == Vec_IntSize(tObjMerge->weights) );
	return tObjMerge;
}
*/
/**Function*************************************************************

  Synopsis    [Main collapsing procedure.]

  Description [Compute K and L (Th_CalKP); collapse two nodes(Th_KLMerge).]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Thre_S* 
Th_CalKLMerge( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin )
{
	Pair_S * pair;
	Thre_S * tObjMerge;
	int w , fInvert;
	
	tObjMerge     = NULL;
	w             = Vec_IntEntry( tObj2->weights , nFanin );
	fInvert       = 0;
	
	if ( w < 0 ) {
		tObj1        = Th_InvertObj(tObj1);
		w           *= -1;
      fInvert      = 1;
	}

#ifdef PROFILE
	//pair = Th_CalKL( tObj1 , tObj2 , nFanin , w , fInvert );
	pair = Th_CalKLIf( tObj1 , tObj2 , nFanin , w , fInvert );
	if ( pair && pair->IntK > 0 && pair->IntL > 0 ) ++thProfiler.numSingleFoutIf;
	else {
	   if ( pair ) ABC_FREE(pair);
	   pair = Th_CalKLDP( tObj1 , tObj2 , nFanin , w , fInvert );
	   if ( pair && pair->IntK > 0 && pair->IntL > 0 ) ++thProfiler.numSingleFoutIff;
		else ++thProfiler.numSingleFoutFail;
	}
#else
	//pair = Th_CalKL( tObj1 , tObj2 , nFanin , w , fInvert );
	pair = Th_CalKLIf( tObj1 , tObj2 , nFanin , w , fInvert );
	//pair = Th_CalKLDP( tObj1 , tObj2 , nFanin , w , fInvert );
#endif
   
	if ( pair && pair->IntK > 0 && pair->IntL > 0 ) 
		tObjMerge = Th_KLMerge( tObj1 , tObj2 , pair , w , fInvert );

   if ( fInvert ) Th_DeleteObjNoInsert( tObj1 ); // delete the inverted object created in this function
	ABC_FREE(pair);

	if (tObjMerge) assert( Vec_IntSize(tObjMerge->Fanins) == Vec_IntSize(tObjMerge->weights) );
	return tObjMerge;
}

/**Function*************************************************************

  Synopsis    [Starting point for collapsing network.]

  Description [Taking a threshold network, collapse two nodes if possible.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void 
mergeThreNtk_Iter( Vec_Ptr_t * TList , int fIterative )
{
    int i, j, FinId , sizeBeforeIter;
    Thre_S* tObj     = NULL;
    Thre_S* mergedTH = NULL;
	 printf( "fIterative = %d\n" , fIterative );

	 do {
		 Th_UnmarkAllNode();
       sizeBeforeIter = Vec_PtrSize( TList );
       while(1){
           int sizeBeforeMerge = Vec_PtrSize( TList );
           Vec_PtrForEachEntry( Thre_S*, TList, tObj, i ){
               // Following nodes are skipped:
               //  > PI/PO/CONST
               //  > black node : those who have nId = 1
               //  > NULL  node
               if (tObj == NULL) continue;
               if (tObj->Type != Th_Node) continue;
               if (tObj->nId == globalRef) continue;
					Th_ObjNormalCheck(tObj);
               Vec_IntForEachEntry( tObj->Fanins, FinId, j){
                   mergedTH = Th_MergeNodes( tObj, j );
                   if (mergedTH){
                       // delete tObj and its Fanin[j]
                       //th_SimplifyNode( mergedTH );
                       Thre_S* FinObj = (Thre_S*)Vec_PtrEntry(TList, FinId);
                       //printf("Merged: (%d, %d) , new node Id = %d\n", tObj->Id,FinId,mergedTH->Id);
                       deleteNode(TList, FinObj);
                       deleteNode(TList, tObj);
                       continue;
                   }
                       //printf("(%d) cannot be merged.\n", tObj->Id);
                   if( j == Vec_IntSize(tObj->Fanins) -1){
                      // non-mergable node-> color = black
                       tObj->nId = globalRef;
                   }
               }
               //printf("%d / %d\n", i, sizeBeforeMerge);
           } 
           // terminating condition: all nodes black
           //                      = no node can be merged
           //                      = the size of thList is fixed
           //                      = the size is not changed
           if( sizeBeforeMerge == Vec_PtrSize(TList) )
               break;
       }
		 // Size increase means some nodes are merged...
	 } while ( fIterative && Vec_PtrSize( TList ) > sizeBeforeIter );

    printf("merging process completed...\n");
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

