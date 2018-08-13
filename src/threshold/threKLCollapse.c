/**CFile****************************************************************
 
  FileName    [threKLCollapse.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Collapse threshold network (multi-fanout prototyping).]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [Mar 12, 2016.]

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
//#define PROFILE

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern Pair_S*    Th_CalKLIf                  ( const Thre_S * , const Thre_S * , int , int , int );
extern Pair_S*    Th_CalKLDP                  ( const Thre_S * , const Thre_S * , int , int , int );
extern int        Th_ObjIsConst               ( const Thre_S * );
extern Thre_S*    Th_GetObjById               ( Vec_Ptr_t * , int );
extern int        Th_Check2FoutCollapse       ( const Thre_S * , const Thre_S * , int );
extern int        Th_CheckMultiFoutCollapse   ( const Thre_S * , int );
extern int        Th_ObjFanoutFaninNum        ( const Thre_S * , const Thre_S * );
// main functions
void       Th_CollapseNtk         ( Vec_Ptr_t * , int , int );
// main helper functions for collapse
int        Th_CollapseNodes       ( const Thre_S * , int , int );
int        Th_CalKLCollapse       ( const Thre_S * );
void       Th_CollapsePair        ( const Thre_S * , const Thre_S * , int );
Pair_S*    Th_CalKL               ( const Thre_S * , const Thre_S * , int , int , int );
Thre_S*    Th_KLCollapse          ( const Thre_S * , const Thre_S * , const Pair_S * , int , int );
Thre_S*    Th_KLCreateClpObj      ( const Thre_S * , const Thre_S * , const Pair_S * , int , int );
int        Th_ObjIsFanin          ( const Thre_S * , int );
void       Th_KLPatchFanio        ( const Thre_S * , const Thre_S * , const Thre_S * );
// dumper functions
void       Th_DumpObj             ( const Thre_S * );
void       Th_DumpMergeObj        ( const Thre_S * , const Thre_S * , const Thre_S *);
// constructor/destructor
Thre_S*    Th_CreateObjNoInsert   ( Th_Gate_Type );
void       Th_DeleteObjNoInsert   ( Thre_S * );
void       Th_DeleteNode          ( Thre_S * );
// other helper functions
void       Th_UnmarkAllNode       ();
Vec_Ptr_t* Th_CopyList            ( Vec_Ptr_t * );
Thre_S*    Th_InvertObj           ( const Thre_S* );
Thre_S*    Th_CopyObj             ( const Thre_S* );
int        MaxF                   ( Vec_Int_t * , int );
int        MinF                   ( Vec_Int_t * , int );
int        Th_ObjNormalCheck      ( const Thre_S * );
void       Th_DeleteClpObj        ( Thre_S * , int );
int        Th_NtkMaxFanout        ();
// Dfs helper
void       Th_NtkDfs              ();
void       Th_NtkDfs_rec          ( Thre_S * , Vec_Ptr_t * );
void       Th_NtkDfsUpdateId      ( Vec_Int_t * );

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
	Vec_IntForEachEntry( tObj->Fanouts , Entry , i ) printf( "%d " , Entry );
	printf( "\n\tDump fanins...\n" );
	Vec_IntForEachEntry( tObj->Fanins , Entry , i ) printf( "%d " , Entry );
	printf( "\n\tDump weights and threshold...\n" );
	Vec_IntForEachEntry( tObj->weights , Entry , i ) printf( "%d " , Entry );
	printf( "%d\n" , tObj->thre );
   fflush(stdout);
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
Th_DeleteNode( Thre_S * tObj )
{  
   if ( tObj->pName ) ABC_FREE( tObj->pName );
   Vec_IntFree( tObj->weights );
   Vec_IntFree( tObj->Fanins  );
   Vec_IntFree( tObj->Fanouts );
   Vec_PtrWriteEntry( current_TList , tObj->Id , NULL );
   ABC_FREE( tObj );
}

/**Function*************************************************************

  Synopsis    [Starting point to collapse two nodes (interface).]

  Description [Multi-fanout prototyping.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Th_CollapseNodes( const Thre_S * tObj2 , int nFanin , int fOutBound )
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
		return 0;
	}
	if ( Vec_IntSize(tObj1->Fanouts) > 1 ) {
	   ++thProfiler.numMultiFout;
      //if ( Vec_IntSize(tObj1->Fanouts) > 2 && Th_CheckMultiFoutCollapse(tObj1) ) ++thProfiler.numMultiFoutOk;
      if ( Th_CheckMultiFoutCollapse(tObj1 , 10) ) ++thProfiler.numMultiFoutOk;
		/*if ( Vec_IntSize(tObj1->Fanouts) == 2 ) {
			++thProfiler.numTwoFout;
			if ( Th_Check2FoutCollapse( tObj1 , tObj2 , nFanin ) ) ++thProfiler.numTwoFoutOk;
		}*/
		return 0;
	}
	else return Th_CalKLMerge( tObj1 , tObj2 , nFanin );
#else
	if ( tObj1->Type != Th_Node || !Th_CheckMultiFoutCollapse( tObj1 , fOutBound ) )
		return 0;
	else return Th_CalKLCollapse( tObj1 );
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
	// copy pName (type: char*)
   tObjCopy->pName = NULL;
   if ( tObj->pName && strlen(tObj->pName) ) {
      tObjCopy->pName = malloc(strlen(tObj->pName) + 1);
      strcpy(tObjCopy->pName, tObj->pName);
   }
	// copy weights, fanins, and fanouts
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
		Vec_IntWriteEntry( tObjInvert->weights , i , -Entry );
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
      if ( i != nFanin && Entry > 0 ) sum += Entry;
	return sum;
}

int 
MinF(Vec_Int_t * weights , int nFanin )
{
	int Entry , i , sum;
	sum = 0;

   Vec_IntForEachEntry( weights , Entry , i )
      if ( i != nFanin && Entry < 0 ) sum += Entry;
	return sum;
}

void 
Th_UnmarkAllNode()
{
   ++globalRef;
}

int
Th_ObjNormalCheck( const Thre_S * tObj )
{
	int Entry , i;
	if ( Th_ObjIsConst(tObj) ) {
		printf( "tObj id=%d is const\n" , tObj->Id );
		return 0;
	}
   Vec_IntForEachEntry( tObj->weights , Entry , i ) 
	{
		if ( Entry == 0 ) {
			printf( "tObj(id=%d) %d-th fanin has 0 weight\n" , tObj->Id , i );
			return 0;
		}
	}
	return 1;
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
Th_KLCollapse( const Thre_S * tObj1 , const Thre_S * tObj2 , const Pair_S * pair , int w , int fInvert )
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
		Vec_IntPushUnique  ( tObjFanin->Fanouts , tObjMerge->Id );
	}
	assert( Vec_IntSize(tObjMerge->Fanins) == Vec_IntSize(tObjMerge->weights) );
}

/**Function*************************************************************

  Synopsis    [Th_CalKLCollapse(interface); Th_CollapsePair(internal).]

  Description [Multi-fanout : tObj1 can be collapsed to all its fanouts.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Th_CalKLCollapse( const Thre_S * tObj1 )
{
	Thre_S * tObj2;
	int nFanin , Entry , i;
   Vec_IntForEachEntry( tObj1->Fanouts , Entry , i )
	{
      tObj2  = Th_GetObjById( current_TList , Entry );
		assert( tObj2 && tObj2->Type == Th_Node );
	   nFanin = Th_ObjFanoutFaninNum( tObj1 , tObj2 );
		assert( Th_CheckPairCollapse( tObj1 , tObj2 , nFanin ) );
		Th_CollapsePair( tObj1 , tObj2 , nFanin );
	}
	return 1;
}

void
Th_CollapsePair( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin )
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
	pair = Th_CalKLIf( tObj1 , tObj2 , nFanin , w , fInvert );
	//pair = Th_CalKLDP( tObj1 , tObj2 , nFanin , w , fInvert );
   
	assert( pair->IntK > 0 && pair->IntL > 0 );
	tObjMerge = Th_KLCollapse( tObj1 , tObj2 , pair , w , fInvert );
   
   if ( fInvert ) Th_DeleteObjNoInsert( tObj1 ); // delete the inverted object created in this function
	ABC_FREE(pair);
	if (tObjMerge) assert( Vec_IntSize(tObjMerge->Fanins) == Vec_IntSize(tObjMerge->weights) );
}

/**Function*************************************************************

  Synopsis    [Starting point for collapsing network.]

  Description [Taking a threshold network, collapse two nodes if possible.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_CollapseNtk( Vec_Ptr_t * TList , int fIterative , int fOutBound )
{
   Thre_S * tObj;
	int i , j , FinId , sizeBeforeIter , sizeBeforeCollapse;
	
	//printf( "Th_CollapseNtk() : fIterative = %d\n" , fIterative );
	//printf( "Th_CollapseNtk() : fOutBound  = %d\n" , fOutBound  );
   do {
	   Th_UnmarkAllNode();
      sizeBeforeIter = Vec_PtrSize( TList );
      while ( 1 ) {
         sizeBeforeCollapse = Vec_PtrSize( TList );
         Vec_PtrForEachEntry( Thre_S* , TList , tObj , i ) 
			{
            // Following nodes are skipped:
            if ( !tObj )                    continue; // NULL  node
            if ( tObj->Type != Th_Node )    continue; // PI/PO/CONST
            if ( tObj->nId == globalRef )   continue; // black node : those who have nId = 1
			   if ( !Th_ObjNormalCheck(tObj) ) continue; // Abnormal node : const or 0-weight, collect and clean
               
		      Vec_IntForEachEntry( tObj->Fanins , FinId , j )
			   {
               if ( Th_CollapseNodes( tObj , j , fOutBound ) ) {
						// delete tObj`s j-fanin and all its fanouts
						Th_DeleteClpObj( tObj , j );
                  break;
               }
               //printf("(%d) cannot be merged.\n", tObj->Id);
               // non-mergable node-> color = black
               if ( j == Vec_IntSize( tObj->Fanins ) - 1 ) tObj->nId = globalRef;
            }
            //printf("%d / %d\n", i, sizeBeforeMerge);
         } 
         if ( sizeBeforeCollapse == Vec_PtrSize(TList) ) break;
      }
	} while ( fIterative && Vec_PtrSize( TList ) > sizeBeforeIter );
    
	//printf("merging process completed...\n");
}

void
Th_DeleteClpObj( Thre_S * tObj2 , int nFanin )
{
	// tObj1 = tObj2`s nFanin
	// delete tObj1 and all its fanouts
	Thre_S * tObj1 , * tObjFout;
	int Entry , i;

	tObj1 = Th_GetObjById( current_TList , Vec_IntEntry( tObj2->Fanins , nFanin ) );
	assert( tObj1 );
	Vec_IntForEachEntry( tObj1->Fanouts , Entry , i )
	{
      tObjFout = Th_GetObjById( current_TList , Entry );
		assert( tObjFout && tObjFout->Type == Th_Node );
		Th_DeleteNode( tObjFout );
	}
   Th_DeleteNode( tObj1 );
}

int
Th_NtkMaxFanout()
{
	Thre_S * tObj;
	int max , i;
	
	max = 0;
	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
	{
		if ( tObj && tObj->Type == Th_Node ) {
		   if ( Vec_IntSize( tObj->Fanouts ) > max )
				max = Vec_IntSize( tObj->Fanouts );
		}
	}
	return max;
}

/**Function*************************************************************

  Synopsis    [Sort TList in topological order.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_NtkDfs()
{
	Vec_Ptr_t * newTList;
	Vec_Int_t * idMap;
	Thre_S    * tObj;
	int i;
	
	newTList = Vec_PtrAlloc( Vec_PtrSize( current_TList ) );
	idMap    = Vec_IntStart( Vec_PtrSize( current_TList ) );
	Th_UnmarkAllNode();

   // push CONST1
   Vec_PtrPush( newTList , Vec_PtrEntry( current_TList , 0 ) );
   ((Thre_S*)Vec_PtrEntry( newTList , 0 ))->nId = globalRef;
#if 1
	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
	{
		if ( tObj ) {
         if ( tObj->Type == Th_Pi ) {	
            Vec_PtrPush( newTList , tObj );
            tObj->nId = globalRef;
         }
         if ( tObj->Type == Th_Po ) Th_NtkDfs_rec( tObj , newTList );
      }
	}
#else
	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
	{
		if ( tObj && tObj->Type == Th_Pi )
		   Th_NtkDfs_rec( tObj , newTList );
	}
#endif
  
   Vec_PtrFree( current_TList );
   current_TList = newTList;
	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
	{
		Vec_IntWriteEntry( idMap , tObj->Id , i );
      tObj->Id = i;
	}
	Th_NtkDfsUpdateId( idMap );
	Vec_IntFree( idMap );
}

void
Th_NtkDfs_rec( Thre_S * tObj , Vec_Ptr_t * newTList )
{
   Thre_S * tObjFin , * tObjFout;
	int Entry , i;

	if ( tObj->nId == globalRef ) return;

#if 1
	Vec_IntForEachEntry( tObj->Fanins , Entry , i )
	{
      tObjFin = Th_GetObjById( current_TList , Entry );
      Th_NtkDfs_rec( tObjFin , newTList );
	}
#else
	Vec_IntForEachEntry( tObj->Fanouts , Entry , i )
	{
      tObjFout = Th_GetObjById( current_TList , Entry );
      Th_NtkDfs_rec( tObjFout , newTList );
	}
#endif
   tObj->nId = globalRef;
   Vec_PtrPush( newTList , tObj );
   //if ( Vec_PtrPushUnique( newTList , tObj ) ) printf( "[Error] repeat object! (Id = %d)\n" , tObj->Id );
}

void
Th_NtkDfsUpdateId( Vec_Int_t * idMap )
{
	Thre_S * tObj;
	int Entry , i , j;

	Vec_PtrForEachEntry( Thre_S * , current_TList , tObj , i )
	{
      Vec_IntForEachEntry( tObj->Fanins , Entry , j )
      {
			Vec_IntWriteEntry( tObj->Fanins , j , Vec_IntEntry( idMap , Entry ) );
		}
      Vec_IntForEachEntry( tObj->Fanouts , Entry , j )
      {
			Vec_IntWriteEntry( tObj->Fanouts , j , Vec_IntEntry( idMap , Entry ) );
		}
	}
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

