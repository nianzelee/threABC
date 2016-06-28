#include <stdio.h>
#include <math.h>
#include "base/abc/abc.h"
#include "map/if/if.h"
#include "map/if/ifCount.h"
#include "threshold.h"
#include "bdd/extrab/extraBdd.h"
#include "misc/extra/extra.h"
#include "misc/vec/vecPtr.h"

int Extra_ThreshCheckNZ       ( word * , int , int * , int * );
int Extra_ThreshCheck         ( word * , int , int * );
int Extra_ThreshHeuristic     ( word * , int , int * );
void cutThCreateConst1   ( If_Man_t * , Vec_Ptr_t * , Vec_Int_t * );
void cutThCreateCi       ( If_Man_t * , Vec_Ptr_t * , Vec_Int_t * );
void cutThCreateCo       ( If_Man_t * , Vec_Ptr_t * , Vec_Int_t * );
void cutThCreateNode     ( If_Man_t * , Vec_Ptr_t * , Vec_Int_t * );
void cutThConnectFanio   ( If_Man_t * , Vec_Ptr_t * , Vec_Int_t * );
void Th_Map2LeavesCut    ( If_Obj_t * , Vec_Ptr_t * , Vec_Int_t * );
int  Th_InvertFanin      ( int * , int , int * , int );
int  Th_InvertCut        ( int * , int , int * , int );
/*************************************************************
NZ codes start here!
*************************************************************/

Vec_Ptr_t * cut2Th( If_Man_t * pIfMan )
{
	Vec_Ptr_t * thre_list;
	Vec_Int_t * id_map;

	thre_list     = Vec_PtrAlloc( If_ManObjNum(pIfMan) );
	id_map        = Vec_IntStart( If_ManObjNum(pIfMan) );
	
   cutThCreateConst1  ( pIfMan , thre_list , id_map );
   cutThCreateCi      ( pIfMan , thre_list , id_map );
   cutThCreateCo      ( pIfMan , thre_list , id_map );
   cutThCreateNode    ( pIfMan , thre_list , id_map );
	
	cutThConnectFanio  ( pIfMan , thre_list , id_map );
   
	Vec_IntFree(id_map);
	return thre_list;
}

void cutThCreateConst1( If_Man_t * pIfMan , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   If_Obj_t   * pObj;

	tObj = Th_CreateObj( thre_list , Th_CONST1 );
	pObj = If_ManConst1( pIfMan );

	Vec_IntWriteEntry( id_map , If_ObjId(pObj) , tObj->Id );
}

void cutThCreateCi( If_Man_t * pIfMan , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   If_Obj_t   * pObj;
	int             i;

   If_ManForEachCi( pIfMan , pObj , i )
	{
      tObj = Th_CreateObj( thre_list , Th_Pi );
		Vec_IntWriteEntry( id_map , If_ObjId(pObj) , tObj->Id );
	}
}

void cutThCreateCo( If_Man_t * pIfMan , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   If_Obj_t   * pObj;
	int             i;

   If_ManForEachCo( pIfMan , pObj , i )
	{
      tObj = Th_CreateObj( thre_list , Th_Po );
		Vec_IntWriteEntry( id_map , If_ObjId(pObj) , tObj->Id );
	}
}

void cutThCreateNode( If_Man_t * pIfMan , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   If_Obj_t   * pObj;
	int             i;

   If_ManForEachNode( pIfMan , pObj , i )
	{
		if ( pObj->nRefs == 0 ) continue;
      tObj = Th_CreateObj( thre_list , Th_Node );
		Vec_IntWriteEntry( id_map , If_ObjId(pObj) , tObj->Id );
	}
}

void cutThConnectFanio( If_Man_t * pIfMan , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
	If_Obj_t * pObj;
	If_Cut_t * pCut;
	Thre_S   * tObj , * tObjFanin;
   word     * ptruth;	
	int T , i , j , nLeaves;
	int * pLeaves;
	int Weights[16];

	// connect PO and its fanin
	If_ManForEachCo( pIfMan , pObj , i )
	{
      tObj      = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId(pObj) ) );
		tObjFanin = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId( If_ObjFanin0(pObj) ) ) );
		Vec_IntPush( tObj->Fanins , tObjFanin->Id );
		Vec_IntPush( tObjFanin->Fanouts , tObj->Id );
		
		if ( If_ObjFaninC0(pObj) ) {
			tObj->thre = 0;
		   Vec_IntPush( tObj->weights , -1 );
		}
		else {
		   tObj->thre = 1;
		   Vec_IntPush( tObj->weights , 1 );
		}
	}

   // connect nodes and its fanins
	If_ManForEachNode( pIfMan , pObj , i )
	{
		if ( pObj->nRefs == 0 ) continue;
		tObj    = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId(pObj) ) );
		pCut    = If_ObjCutBest(pObj);
		nLeaves = If_CutLeaveNum(pCut);

		/*if ( nLeaves == 2 ) {
			//printf("2 leaves cut map...\n");
         Th_Map2LeavesCut( pObj , thre_list , id_map );
			continue;
		}*/

		pLeaves = If_CutLeaves(pCut);
	   ptruth  = If_CutTruthW( pIfMan , pCut );
	   T       = Extra_ThreshCheckNZ( ptruth , nLeaves , Weights , pLeaves );
		T       = Th_InvertFanin( pLeaves , nLeaves , Weights , T );
		if ( pCut-> fCompl )
			T    = Th_InvertCut( pLeaves , nLeaves , Weights , T );

		tObj->thre = T;

		for ( j = 0 ; j < nLeaves ; ++j ) {
		   assert( pLeaves[j] >= 0 );
			if ( Weights[j] == 0 ) continue;
			tObjFanin = Th_GetObjById( thre_list , Vec_IntEntry( id_map , pLeaves[j] ) );
			Vec_IntPush( tObj->Fanins       , tObjFanin->Id );
			Vec_IntPush( tObj->weights      , Weights[j] );
			Vec_IntPush( tObjFanin->Fanouts , tObj->Id );
		}
	}
}

void Th_Map2LeavesCut( If_Obj_t * pObj , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
	assert( pObj->nRefs > 0 );

	Thre_S * tObj , * tObjFanin0 , * tObjFanin1;

	if ( If_ObjFanin0(pObj)->nRefs == 0 ) { 
      tObjFanin0 = Th_CreateObj( thre_list , Th_Node );
		Vec_IntWriteEntry( id_map , If_ObjId( If_ObjFanin0(pObj) ) , tObjFanin0->Id );
		((If_ObjFanin0(pObj))->nRefs)++;
		Th_Map2LeavesCut( If_ObjFanin0(pObj) , thre_list , id_map );
	}

	if ( If_ObjFanin1(pObj)->nRefs == 0 ) { 
      tObjFanin1 = Th_CreateObj( thre_list , Th_Node );
		Vec_IntWriteEntry( id_map , If_ObjId( If_ObjFanin1(pObj) ) , tObjFanin1->Id );
		((If_ObjFanin1(pObj))->nRefs)++;
		Th_Map2LeavesCut( If_ObjFanin1(pObj) , thre_list , id_map );
	}

	tObj       = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId( pObj ) ) );
   tObjFanin0 = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId( If_ObjFanin0(pObj) ) ) );
   tObjFanin1 = Th_GetObjById( thre_list , Vec_IntEntry( id_map , If_ObjId( If_ObjFanin1(pObj) ) ) );
	
   if ( tObj->Id == 0 )       assert(0);
   if ( tObjFanin0->Id == 0 ) assert(0);
   if ( tObjFanin1->Id == 0 ) assert(0);

	Vec_IntPush( tObj->Fanins , tObjFanin0->Id );
	Vec_IntPush( tObj->Fanins , tObjFanin1->Id );
	Vec_IntPush( tObjFanin0->Fanouts , tObj->Id );
	Vec_IntPush( tObjFanin1->Fanouts , tObj->Id );

	if ( !If_ObjFaninC0(pObj) && !If_ObjFaninC1(pObj) ) {
      tObj->thre = 2;
		Vec_IntPush( tObj->weights , 1 );
		Vec_IntPush( tObj->weights , 1 );
	}
   else if ( If_ObjFaninC0(pObj) && !If_ObjFaninC1(pObj) ) {
      tObj->thre = 1;
		Vec_IntPush( tObj->weights , -1 );
		Vec_IntPush( tObj->weights ,  1 );
	}
	else if ( !If_ObjFaninC0(pObj) &&  If_ObjFaninC1(pObj) ) {
		tObj->thre = 1;
		Vec_IntPush( tObj->weights ,  1 );
		Vec_IntPush( tObj->weights , -1 );
	}
   else {
	   tObj->thre = 0;
		Vec_IntPush( tObj->weights , -1 );
		Vec_IntPush( tObj->weights , -1 );
	}
}

int Th_InvertFanin( int * pLeaves , int nLeaves , int * Weights , int T )
{
	int i;

	for ( i = 0 ; i < nLeaves ; ++i ) {
	   if ( pLeaves[i] < 0 ) {
		   pLeaves[i] = -pLeaves[i];
			T          = T - Weights[i];
			Weights[i] = -Weights[i];
		}
	}
	return T;
}

int Th_InvertCut( int * pLeaves , int nLeaves , int * Weights , int T )
{
	int i;

	for ( i = 0 ; i < nLeaves ; ++i ) {
	   Weights[i] = -Weights[i];
	}
	return (1 - T);
}
