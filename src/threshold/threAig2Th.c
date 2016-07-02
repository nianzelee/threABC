/**CFile****************************************************************
 
  FileName    [threAig2Th.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [AIG to threshold network conversion.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [Mar 29, 2016.]

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
extern int Extra_ThreshCheck       ( word * , int , int * );
extern int Extra_ThreshHeuristic   ( word * , int , int * );

// main function
Vec_Ptr_t* aig2Th                  ( Abc_Ntk_t * );
// helper for aig2Th
int        Th_FindNtkMaxId         ( Abc_Ntk_t * );
void       aigThCreateConst1       ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
void       aigThCreateCi           ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
void       aigThCreateCo           ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
void       aigThCreateNode         ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
void       aigThConnectFanin       ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
void       aigThConnectFanout      ( Abc_Ntk_t * , Vec_Ptr_t * , Vec_Int_t * );
// constructor/destructor for Thre_S
Thre_S*    Th_CreateObj            ( Vec_Ptr_t * , Th_Gate_Type );
void       Th_DeleteObj            ( Thre_S * );
Thre_S*    Th_GetObjById           ( Vec_Ptr_t * , int );

//////////////////////////////////
//Threshold object constructor  //
//////////////////////////////////

/**Function*************************************************************

  Synopsis    [Thre_S constructor/destructor/find object in list by Id.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Thre_S* 
Th_CreateObj( Vec_Ptr_t * TList , Th_Gate_Type Type )
{
   Thre_S * tObj    = ABC_ALLOC( Thre_S , 1 );
	tObj->thre       = 0;
	tObj->Type       = Type;
	tObj->Ischoose   = 0;
   tObj->Id         = Vec_PtrSize( TList );
	tObj->oId        = 0;
	tObj->nId        = 0;
	tObj->cost       = 0;
	tObj->level      = 0;
	tObj->pName      = NULL;
	tObj->weights    = Vec_IntAlloc(16);
	tObj->Fanins     = Vec_IntAlloc(16);
	tObj->Fanouts    = Vec_IntAlloc(16);
	tObj->pCopy      = NULL;

   Vec_PtrPush( TList , tObj );

	return tObj;
}

void
Th_DeleteObj( Thre_S * tObj )
{
	Vec_PtrWriteEntry( current_TList , tObj->Id , NULL );
	ABC_FREE( tObj->pName );
   Vec_IntFree( tObj->weights );
   Vec_IntFree( tObj->Fanins  );
   Vec_IntFree( tObj->Fanouts );
   ABC_FREE( tObj );
}

Thre_S* 
Th_GetObjById( Vec_Ptr_t * thre_list , int Id )
{
	Thre_S * tObj;
   tObj = Vec_PtrEntry( thre_list , Id );
	if ( !tObj ) printf( "Warning : object id=%d is NULL\n" , Id );
	return tObj;
}

/**Function*************************************************************

  Synopsis    [Construct threshold network from AIG.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Vec_Ptr_t* 
aig2Th( Abc_Ntk_t * pNtk ) 
{
	assert( Abc_NtkIsStrash(pNtk) && Abc_NtkIsComb(pNtk) );
	Vec_Ptr_t * thre_list;
	Vec_Int_t * id_map;

	thre_list     = Vec_PtrAlloc( Abc_NtkObjNum( pNtk ) );
	id_map        = Vec_IntStart( Th_FindNtkMaxId( pNtk ) + 1 );
	
   aigThCreateConst1  ( pNtk , thre_list , id_map );
   aigThCreateCi      ( pNtk , thre_list , id_map );
   aigThCreateCo      ( pNtk , thre_list , id_map );
   aigThCreateNode    ( pNtk , thre_list , id_map );
	
	aigThConnectFanin  ( pNtk , thre_list , id_map );
	aigThConnectFanout ( pNtk , thre_list , id_map );
   
	Vec_IntFree(id_map);
	return thre_list;
}

int
Th_FindNtkMaxId( Abc_Ntk_t * pNtk )
{
   Abc_Obj_t * pObj;
	int max , i;

	max = 0;
	Abc_NtkForEachObj( pNtk , pObj , i )
		if ( Abc_ObjId( pObj ) > max ) max = Abc_ObjId( pObj );

	return max;
}

void 
aigThCreateConst1( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj;

	tObj = Th_CreateObj( thre_list , Th_CONST1 );
	pObj = Abc_AigConst1( pNtk );

	Vec_IntWriteEntry( id_map , Abc_ObjId( pObj ) , tObj->Id );
}

void 
aigThCreateCi( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj;
	int             i;

   Abc_NtkForEachCi( pNtk , pObj , i )
	{
      tObj        = Th_CreateObj( thre_list , Th_Pi );
      //tObj->pName = Abc_ObjName( pObj );
		Vec_IntWriteEntry( id_map , Abc_ObjId( pObj ) , tObj->Id );
	}
}

void 
aigThCreateCo( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj;
	int             i;

   Abc_NtkForEachCo( pNtk , pObj , i )
	{
      tObj        = Th_CreateObj( thre_list , Th_Po );
      //tObj->pName = Abc_ObjName( pObj );
		Vec_IntWriteEntry( id_map , Abc_ObjId( pObj ) , tObj->Id );
	}
}

void 
aigThCreateNode( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj;
	int             i;

   Abc_NtkForEachNode( pNtk , pObj , i )
	{
      tObj      = Th_CreateObj( thre_list , Th_Node );
		Vec_IntWriteEntry( id_map , Abc_ObjId( pObj ) , tObj->Id );
	}
}

/**Function*************************************************************

  Synopsis    [Construct threshold network from AIG.]

  Description [Connect fanin/fanout.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void 
aigThConnectFanin( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj;
	int             i;

   // connect PO
	Abc_NtkForEachPo( pNtk , pObj , i )
	{
		tObj = Vec_PtrEntry( thre_list , Vec_IntEntry( id_map , Abc_ObjId( pObj ) ) );

      Vec_IntPush( tObj->Fanins , Vec_IntEntry( id_map , Abc_ObjFaninId0( pObj ) ) );
		if ( Abc_ObjFaninC0( pObj ) ) {
		   tObj->thre = 0;
			Vec_IntPush( tObj->weights , -1 );
		}
		else {
		   tObj->thre = 1;
			Vec_IntPush( tObj->weights ,  1 );
		}
	}

	// connect Node
	Abc_NtkForEachNode( pNtk , pObj , i )
	{
		tObj = Vec_PtrEntry( thre_list , Vec_IntEntry( id_map , Abc_ObjId( pObj ) ) );

		Vec_IntPush( tObj->Fanins , Vec_IntEntry( id_map , Abc_ObjFaninId0( pObj ) ) );
		Vec_IntPush( tObj->Fanins , Vec_IntEntry( id_map , Abc_ObjFaninId1( pObj ) ) );
		if ( !Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj) ) {
		   tObj->thre = 2;
		   Vec_IntPush( tObj->weights , 1 );
		   Vec_IntPush( tObj->weights , 1 );
			continue;
		}
		if (  Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj) ) {
		   tObj->thre = 1;
		   Vec_IntPush( tObj->weights , -1 );
		   Vec_IntPush( tObj->weights ,  1 );
			continue;
		}
		if ( !Abc_ObjFaninC0(pObj) &&  Abc_ObjFaninC1(pObj) ) {
		   tObj->thre = 1;
		   Vec_IntPush( tObj->weights ,  1 );
		   Vec_IntPush( tObj->weights , -1 );
			continue;
		}
		if (  Abc_ObjFaninC0(pObj) &&  Abc_ObjFaninC1(pObj) ) {
		   tObj->thre = 0;
		   Vec_IntPush( tObj->weights , -1 );
		   Vec_IntPush( tObj->weights , -1 );
			continue;
		}
	}
}

void 
aigThConnectFanout( Abc_Ntk_t * pNtk , Vec_Ptr_t * thre_list , Vec_Int_t * id_map )
{
   Thre_S     * tObj;
   Abc_Obj_t  * pObj , * pFanout;
	int         i , j;

   // connect CONST1
	pObj = Abc_AigConst1( pNtk );
	tObj = Th_GetObjById( thre_list , Vec_IntEntry( id_map , Abc_ObjId( pObj ) ) );
   Abc_ObjForEachFanout( pObj , pFanout , i )
		Vec_IntPush( tObj->Fanouts , Vec_IntEntry( id_map , Abc_ObjId( pFanout ) ) );

	// connect Ci
	Abc_NtkForEachCi( pNtk , pObj , i )
	{
	   tObj = Th_GetObjById( thre_list , Vec_IntEntry( id_map , Abc_ObjId( pObj ) ) );
      Abc_ObjForEachFanout( pObj , pFanout , j )
		   Vec_IntPush( tObj->Fanouts , Vec_IntEntry( id_map , Abc_ObjId( pFanout ) ) );
	}
	
	// connect Node
	Abc_NtkForEachNode( pNtk , pObj , i )
	{
	   tObj = Th_GetObjById( thre_list , Vec_IntEntry( id_map , Abc_ObjId( pObj ) ) );
      Abc_ObjForEachFanout( pObj , pFanout , j )
		   Vec_IntPush( tObj->Fanouts , Vec_IntEntry( id_map , Abc_ObjId( pFanout ) ) );
	}
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
