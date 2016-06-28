#include <stdio.h>
#include <math.h>
#include "base/abc/abc.h"
#include "map/if/if.h"
#include "map/if/ifCount.h"
#include "threshold.h"
#include "bdd/extrab/extraBdd.h"
#include "misc/extra/extra.h"
#include "misc/vec/vecPtr.h"


void       Th_PrintStat          ( Vec_Ptr_t * );
int        Th_CountGate          ( Vec_Ptr_t * , Th_Gate_Type );
int        Th_CountLevel         ( Vec_Ptr_t * );
void       Th_SetLevel           ( Vec_Ptr_t * );
void       Th_ComputeLevel       ( Vec_Ptr_t * );
void       Th_ComputeLevel_rec   ( Vec_Ptr_t * , Thre_S * );
int        Th_FindMaxLevel       ( Vec_Ptr_t * );
Vec_Int_t* Th_CountFanio         ( Vec_Ptr_t * , int );
void       Th_PrintData          ( int , int , int , int , Vec_Int_t * , Vec_Int_t * );


/*************************************************************
NZ codes start here!
*************************************************************/

Vec_Int_t * Th_CountFanio( Vec_Ptr_t * tList , int fFanin )
{
	// fFanin = 1 --> count fanin
	// fFanin = 0 --> count fanout
	assert(fFanin == 1 || fFanin == 0);

   Vec_Int_t * fanSizeList;
	int i , numFan , Entry , maxFanNum;
	Thre_S * tObj;
   
	maxFanNum = 0;
   
	// find max fanio number
	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj && ( tObj->Type == Th_Node ) ) {
		   numFan = (fFanin) ? Vec_IntSize( tObj->Fanins ) : Vec_IntSize( tObj->Fanouts );
			if ( numFan > maxFanNum ) maxFanNum = numFan;
		}
	}

	fanSizeList = Vec_IntStart( maxFanNum + 1 );

	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj && ( tObj->Type == Th_Node ) ) {
		   numFan = (fFanin) ? Vec_IntSize( tObj->Fanins ) : Vec_IntSize( tObj->Fanouts );
		   Entry  = Vec_IntEntry( fanSizeList , numFan ); 
         Vec_IntWriteEntry( fanSizeList , numFan , Entry + 1 );
		}
	}

	return fanSizeList;
}

void Th_SetLevel( Vec_Ptr_t * tList )
{
	int i;
	Thre_S * tObj;

	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj ) tObj->level = 0;
	}
}

void Th_ComputeLevel_rec( Vec_Ptr_t * tList , Thre_S * tObj )
{
   Thre_S * tObjFanin;
	int i , Entry;
  
	//printf( "Dump tObj: \n");
	//Th_DumpObj( tObj );
	if ( tObj->Type == Th_Po || tObj->Type == Th_Node ) {
	   Vec_IntForEachEntry( tObj->Fanins , Entry , i ) 
		{
			tObjFanin = Th_GetObjById( tList , Entry );
	      //printf( "Dump tObjFanin: \n");
	      //Th_DumpObj( tObjFanin );
			assert(tObjFanin);
			if ( tObjFanin-> level < tObj->level + 1 ) {
			   tObjFanin->level = tObj->level + 1;
				Th_ComputeLevel_rec( tList , tObjFanin );
			}
		}
	}
}

void Th_ComputeLevel( Vec_Ptr_t * tList )
{
	Thre_S * tObj;
	Vec_Ptr_t * vPo;
	int i;

	vPo = Vec_PtrAlloc(16);

	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj && tObj->Type == Th_Po )
			Vec_PtrPush( vPo , tObj );
	}
   
	Vec_PtrForEachEntry( Thre_S * , vPo , tObj , i )
	{
		Th_ComputeLevel_rec( tList , tObj );
	}

	Vec_PtrFree(vPo);
}

int Th_FindMaxLevel( Vec_Ptr_t * tList )
{
	int level , i;
	Thre_S * tObj;

	level = 0;

	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj && tObj->Type == Th_Pi ) {
		   if ( tObj->level > level ) level = tObj->level;
		}
	}
	return level;
}

int Th_CountLevel( Vec_Ptr_t * tList )
{
	Th_SetLevel  ( tList );
	Th_ComputeLevel( tList );
	return Th_FindMaxLevel( tList );
}

int Th_CountGate( Vec_Ptr_t * tList , Th_Gate_Type type )
{
	int numGate , i;
	Thre_S * tObj;

	numGate = 0;

	Vec_PtrForEachEntry( Thre_S * , tList , tObj , i )
	{
		if ( tObj && tObj->Type == type ) numGate++;
	}

	return numGate;
}

Vec_Int_t * Th_FanNumStat( Vec_Int_t * fanSizeList )
{
	//assert( Vec_IntSize(fanSizeList) < 1000 );
	Vec_Int_t * fList;
	int Entry , i;

	fList = Vec_IntAlloc(16);

	Vec_IntForEachEntry( fanSizeList , Entry , i )
	{
		if ( i < 10 ) 
			Vec_IntPush( fList , Entry );
		else if ( i < 100 ) {
			if ( i % 10 == 0 )
				Vec_IntPush( fList , Entry );
			else 
				Vec_IntWriteEntry( fList , 9 + (i/10) , Vec_IntEntry( fList , 9 + (i/10) ) + Entry ) ;
		}
		else if ( i < 1000 ) {
			if ( i % 100 == 0 )
				Vec_IntPush( fList , Entry );
			else 
				Vec_IntWriteEntry( fList , 18 + (i/100) , Vec_IntEntry( fList , 18 + (i/100) ) + Entry );
		}
	}
	return fList;
}

void Th_PrintData( int nPi , int nPo , int nNode , int nLevel , Vec_Int_t * finSizeList , Vec_Int_t * foutSizeList)
{
	int i , Entry;
	Vec_Int_t * finList , * foutList;
	
	printf("\tPI     : %d\n" , nPi);
	printf("\tPO     : %d\n" , nPo);
	printf("\tNode   : %d\n" , nNode);
	printf("\tLevel  : %d\n" , nLevel-1);
   
	printf("----------------------------------------\n");
	printf("\tFanin numbers\n");

   finList = Th_FanNumStat(finSizeList);

	Vec_IntForEachEntry( finList , Entry , i )
	{
		if ( Entry != 0 ) {
			if ( i < 10 ) printf( "\t#%d        : %d\n" , i , Entry );
			else if ( i < 19 ) 
				printf( "\t#%d-%d    : %d\n" , ((i%10)+1)*10 , ((i%10)+1)*10+9 , Entry  );
			else if ( i < 29 ) {
				if ( i == 19 ) printf( "\t#%d-%d  : %d\n" , 100 , 199 , Entry  );
			   else printf( "\t#%d-%d  : %d\n" , ((i%10)+2)*100 , ((i%10)+2)*100+99 , Entry  );
			}
		}
	}
	printf( "Maximum fanin number: %d\n" , Vec_IntSize(finSizeList)-1 );

	printf("----------------------------------------\n");
	printf("\tFanout numbers\n");
   
	foutList = Th_FanNumStat(foutSizeList);

	Vec_IntForEachEntry( foutList , Entry , i )
	{
		if ( Entry != 0 ) {
			if ( i < 10 ) printf( "\t#%d        : %d\n" , i , Entry );
			else if ( i < 19 ) 
				printf( "\t#%d-%d    : %d\n" , ((i%10)+1)*10 , ((i%10)+1)*10+9 , Entry  );
			else if ( i < 29 ) {
				if ( i == 19 ) printf( "\t#%d-%d  : %d\n" , 100 , 199 , Entry  );
			   else printf( "\t#%d-%d  : %d\n" , ((i%10)+2)*100 , ((i%10)+2)*100+99 , Entry  );
			}
		}
	}
	printf( "Maximum fanout number: %d\n" , Vec_IntSize(foutSizeList)-1 );

   Vec_IntFree(finList);
	Vec_IntFree(foutList);
}

void Th_PrintStat( Vec_Ptr_t * tList )
{
	int numPi , numPo , numNode , numLevel;
	Vec_Int_t * faninSizeList , * fanoutSizeList;

	numPi          = Th_CountGate( tList , Th_Pi   );
	numPo          = Th_CountGate( tList , Th_Po   );
	numNode        = Th_CountGate( tList , Th_Node );
	
	numLevel       = Th_CountLevel( tList );
	
	faninSizeList  = Th_CountFanio( tList , 1 );
	fanoutSizeList = Th_CountFanio( tList , 0 );

	Th_PrintData( numPi , numPo , numNode , numLevel , faninSizeList , fanoutSizeList );
   Vec_IntFree(faninSizeList);
   Vec_IntFree(fanoutSizeList);
}
