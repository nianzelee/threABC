/**CFile****************************************************************
 
  FileName    [threTh2Blif.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Threshold network to blif conversion.]

  Author      [Nian-Ze Lee]
   
  Affiliation [NTU]

  Date        [April 20, 2016.]

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
extern Thre_S * slow_sortByWeights         ( Thre_S * );
extern Thre_S * slow_sortByAbsWeights      ( Thre_S * );
extern void     delete_sortedNode          ( Thre_S * );
extern int      Thre_LocalMax              (Thre_S * , int );
extern int      Thre_LocalMin              (Thre_S * , int );

// main function
void   Th_WriteBlif            ( Vec_Ptr_t * thre_list , const char * );
void   Th_WriteBlifInput       ( FILE * , Vec_Ptr_t * );
void   Th_WriteBlifOutput      ( FILE * , Vec_Ptr_t * );
void   Th_WriteBlifName        ( FILE * , Vec_Ptr_t * );
void   Th_WriteBlifOneName     ( FILE * , Thre_S * );
void   Th_ObjBuildSop          ( FILE * , Thre_S * , Vec_Str_t * , int , int );
void   Th_StrOnFinalize        ( Vec_Str_t * , int );


/**Function*************************************************************

  Synopsis    [Main function to write threshold ntk as blif.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_WriteBlif( Vec_Ptr_t * thre_list , const char * name )
{
	FILE * out;
	Vec_Ptr_t * vPi , * vPo , * vTh;
	Thre_S * tObj;
	int i;

	printf( "Write out threshold network as blif file...\n" );
	
	out = fopen( name , "w" );
	vPi = Vec_PtrAlloc( 100 );
	vPo = Vec_PtrAlloc( 100 );
	vTh = Vec_PtrAlloc( 100 );
   
	Vec_PtrForEachEntry( Thre_S * , thre_list , tObj , i )
	{
      if      ( !tObj ) continue;
      if      ( tObj-> Type == Th_Pi   )  Vec_PtrPush( vPi , tObj );
      else if ( tObj-> Type == Th_Po   )  Vec_PtrPush( vPo , tObj );
      else if ( tObj-> Type == Th_Node )  Vec_PtrPush( vTh , tObj );
	}

   fprintf( out , ".model %s\n" , name );
	Th_WriteBlifInput     ( out , vPi );
	Th_WriteBlifOutput    ( out , vPo );
	Th_WriteBlifName      ( out , vTh );
   fprintf( out , ".end\n" );
	
	Vec_PtrFree(vPi);
	Vec_PtrFree(vPo);
	Vec_PtrFree(vTh);
	fclose( out );
}

void 
Th_WriteBlifInput( FILE * out , Vec_Ptr_t * vPi )
{
	Thre_S * tObj;
	int i;

	Vec_PtrForEachEntry( Thre_S * , vPi , tObj , i )
		fprintf( out , ".inputs %d\n" , tObj->Id );
}

void 
Th_WriteBlifOutput( FILE * out , Vec_Ptr_t * vPo )
{
	Thre_S * tObj;
	int i;

	Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
		fprintf( out , ".outputs %d\n" , tObj->Id );
	
	Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
	{
      if ( Vec_IntEntry( tObj->Fanins , 0 ) == 0 ) {
			//printf( "[Warning] constant output!\n" );
			fprintf( out , ".names %d\n" , tObj->Id );
			if ( tObj->thre == 1 ) fprintf( out , " 1\n" );
			else if ( tObj->thre == 0 )  fprintf( out , " 0\n" );
			else printf( "[Error] wrong output threshold!\n" );
		}
		else {
			fprintf( out , ".names %d %d\n" , Vec_IntEntry( tObj->Fanins , 0 ) , tObj->Id );
			if ( tObj->thre == 1 )       fprintf( out , "1 1\n" );
			else if ( tObj->thre == 0 )  fprintf( out , "0 1\n" );
			else printf( "[Error] wrong output threshold!\n" );
		}
	}
}

void 
Th_WriteBlifName( FILE * out , Vec_Ptr_t * vTh )
{
	Thre_S * tObj;
	int i;

	Vec_PtrForEachEntry( Thre_S * , vTh , tObj , i )
		Th_WriteBlifOneName( out , tObj );
}

/**Function*************************************************************

  Synopsis    [Write one threshold gate into SOP]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_WriteBlifOneName( FILE * out , Thre_S * tObj )
{
	Vec_Str_t * Sop;
	Thre_S * tObjSort;
	int Entry , i;

	Sop      = Vec_StrAlloc( 10 );
   //tObjSort = slow_sortByWeights( tObj );
   tObjSort = slow_sortByAbsWeights( tObj );

	fprintf( out , ".names" );
   Vec_IntForEachEntry( tObjSort->Fanins , Entry , i ) fprintf( out , " %d" , Entry );
	fprintf( out , " %d\n" , tObjSort->Id );

   Vec_StrClear( Sop );
   Th_ObjBuildSop( out , tObjSort , Sop , tObjSort->thre , 0 );
   delete_sortedNode( tObjSort );
	Vec_StrFree( Sop );
}

void
Th_ObjBuildSop( FILE * out , Thre_S * tObj , Vec_Str_t * Sop , int thre , int lvl )
{
   assert( lvl < Vec_IntSize( tObj->Fanins ) );

	Vec_Str_t * posSop , * negSop;
	int curW , maxF , minF , posT , negT;

	posSop = Vec_StrDup( Sop );
   Vec_StrPrintNum( posSop , 1 ); 
	negSop = Vec_StrDup( Sop );
   Vec_StrPrintNum( negSop , 0 ); 

	curW  = Vec_IntEntry( tObj->weights , lvl );
   maxF  = Thre_LocalMax( tObj , lvl );
   minF  = Thre_LocalMin( tObj , lvl );

	posT = thre - curW;
   if ( posT <= minF ) {
      // on-set
		Th_StrOnFinalize( posSop , Vec_IntSize( tObj->Fanins )-lvl-1 );
      fprintf( out , "%s\n" , posSop->pArray );
   }
   else if ( maxF < posT ) {
      // off-set : do nothing
   }
   else Th_ObjBuildSop( out , tObj , posSop , posT , lvl+1 );
   
	negT = thre;
   if ( negT <= minF ) {
      // on-set
		Th_StrOnFinalize( negSop , Vec_IntSize( tObj->Fanins )-lvl-1 );
      fprintf( out , "%s\n" , negSop->pArray );
   }
   else if ( maxF < negT ) {
      // off-set : do nothing
   }
   else Th_ObjBuildSop( out , tObj , negSop , negT , lvl+1 );
    
	Vec_StrFree( posSop );
   Vec_StrFree( negSop );
}

void
Th_StrOnFinalize( Vec_Str_t * Sop , int numDc )
{
	int i;

	for ( i = 0 ; i < numDc ; ++i ) 
      Vec_StrAppend( Sop , "-" );
	
	Vec_StrAppend   ( Sop , " " );
   Vec_StrPrintNum ( Sop , 1 );
   Vec_StrPush     ( Sop , '\0');
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
