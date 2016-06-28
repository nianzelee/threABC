/**CFile****************************************************************
 
  FileName    [threCalKL.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [if/iff conditions for collapsing.]

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
#define DEBUG
#define TH_MIN (1 << 31) // indicating no solution for subset sum

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// extern functions
extern int   MaxF                 ( Vec_Int_t * , int );
extern int   MinF                 ( Vec_Int_t * , int );
extern void  Th_DumpObj           ( const Thre_S * );
// main functions
Pair_S*    Th_CalKLIf             ( const Thre_S * , const Thre_S * , int , int , int );
Pair_S*    Th_CalKLDP             ( const Thre_S * , const Thre_S * , int , int , int );
// helper functions
void       Th_KLCheckCond         ( const Thre_S * , int , int , int , int * );
// if conditions
void       Th_IfCoeff             ( const Thre_S * , int , int * , int * );
Pair_S*    Th_IfSolveKL           ( int * , int * , int );
int        Th_IfCheckCoeff        ( int * , int * );
int        Th_IfCheckKL           ( Pair_S * , int * , int * );
// iff conditions
void       Th_DPCoeff             ( const Thre_S * , const Thre_S * , int , int , int , int * , int * );
int        Th_subSum              ( const Thre_S * , int , int , int );
Vec_Int_t* Th_subSumCollectNum    ( const Thre_S * , int );
int        Th_subSumSolveDP       ( Vec_Int_t * , int , int );
int        Th_subSumOptValue      ( int * , int , int , int , int );
int**      Th_Alloc2DArray        ( int , int );
void       Th_Free2DArray         ( int ** , int );
Pair_S*    Th_DPSolveKL           ( int * , int * , int );
int        Th_DPCheckCoeff        ( int * , int * , int );
int        Th_DPCheckKL           ( Pair_S * , int * , int * );
int        Th_DPDivRound          ( int , int );
int        Th_DPFindIntK          ( int , int , int , int , int , int );
int        Th_ObjIsConst          ( const Thre_S * );
// dumper functions
void       Th_dumpCoeff           ( int * , int * );
void       Th_dumpCond            ( int * );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Calculate K and L by if conditions.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Pair_S* 
Th_CalKLIf( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin , int w , int fInvert )
{
   if ( Th_ObjIsConst( tObj1 ) || Th_ObjIsConst( tObj2 ) ) return NULL;	
	
	Pair_S * pair;
	int cond[2] , n[4];

	cond[0] = cond[1] = 0;
	n[0] = n[1] = n[2] = n[3] = 0;
	// 1. check conditions
	Th_KLCheckCond( tObj2 , nFanin , w , fInvert , cond );
	//Th_dumpCond( cond );
	// 2. compute coefficients
	Th_IfCoeff( tObj1 , w , cond , n );
	//Th_dumpCoeff( cond , n );
	// 3. compute K and L	
	pair = Th_IfSolveKL( cond , n , w );
	return pair;
}

/**Function*************************************************************

  Synopsis    [Calculate K and L by dynamic programming.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Pair_S* 
Th_CalKLDP( const Thre_S * tObj1 , const Thre_S * tObj2 , int nFanin , int w , int fInvert )
{
	/*printf( "Collapsing id=%d and id =%d\n" , tObj1->Id , tObj2->Id );
	if ( tObj1->Id == 8272 && tObj2->Id == 8273 ) {
	   Th_DumpObj( tObj1 );
		Th_DumpObj( tObj2 );
		printf( "nFanin = %d , w = %d , fInvert = %d\n" , nFanin , w , fInvert );
	}*/
   if ( Th_ObjIsConst( tObj1 ) || Th_ObjIsConst( tObj2 ) ) return NULL;	
	
	Pair_S * pair;
	int cond[2] , n[6];

	cond[0] = cond[1] = 0;
	n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = 0;
	// 1. check conditions
	Th_KLCheckCond( tObj2 , nFanin , w , fInvert , cond );
	//Th_dumpCond( cond );
	// 2. compute coefficients
	Th_DPCoeff( tObj1 , tObj2 , nFanin , w , fInvert , cond , n );
	//Th_dumpCoeff( cond , n );
	// 3. compute K and L	
	pair = Th_DPSolveKL( cond , n , w );
	return pair;
}

int
Th_ObjIsConst( const Thre_S * tObj )
{
	int max , min;
	max = MaxF( tObj->weights , Vec_IntSize(tObj->weights) );
	min = MinF( tObj->weights , Vec_IntSize(tObj->weights) );
	return ( max < tObj->thre || min >= tObj->thre );
}


/**Function*************************************************************

  Synopsis    [Check conditions to decide constraints.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_KLCheckCond( const Thre_S * tObj2 , int nFanin , int w , int fInvert , int * cond )
{
   int T2 , minf2 , maxf2 , condition;

	T2        = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);
	maxf2     = MaxF( tObj2->weights , nFanin );
	minf2     = MinF( tObj2->weights , nFanin );
	// assume cond3 must be satisfied, OK?
	// if cond3 = 0 --> no connection between obj1 and obj2
	condition  = T2 - w - 1;
	if ( minf2 <= condition ) cond[0] = 1;
	condition  = T2;
	if ( maxf2 >= condition ) cond[1] = 1;
}

/**Function*************************************************************

  Synopsis    [Compute if coefficients based on conditions.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_IfCoeff( const Thre_S * tObj1 , int w , int * cond , int * n )
{
	int T1 , maxf1 , minf1;
	T1      = tObj1->thre;
	maxf1   = MaxF( tObj1->weights , Vec_IntSize( tObj1->weights ) ); 
	minf1   = MinF( tObj1->weights , Vec_IntSize( tObj1->weights ) ); 
   if ( cond[0] ) n[0] = maxf1 - T1; 
	if ( cond[1] ) {
		n[1]  = w;
		n[2]  = T1 - minf1;
	}
   // assume 3-rd cond must be satisfied
	n[3] = w - 1;
}

/**Function*************************************************************

  Synopsis    [Compute iff coefficients based on conditions.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_DPCoeff( const Thre_S * tObj1 , const Thre_S * tObj2 , 
		      int nFanin , int w , int fInvert , int * cond , int * n )
{
	int T1 , T2 , maxf1 , minf1 , sumDP;
	T1      = tObj1->thre;
	T2      = (fInvert) ? (tObj2->thre + w) : (tObj2->thre);
	maxf1   = MaxF( tObj1->weights , Vec_IntSize( tObj1->weights ) ); 
	minf1   = MinF( tObj1->weights , Vec_IntSize( tObj1->weights ) ); 
	sumDP   = 0; // to be computed by dynamic programming
   if ( cond[0] ) {
      sumDP = Th_subSum( tObj2 , nFanin , T2-w-1 , 1 );
		n[0]  = T2 - w - sumDP;
		n[1]  = maxf1 - T1;
	}
	if ( cond[1] ) {
      sumDP = Th_subSum( tObj2 , nFanin , T2 , 0 );
		n[2]  = sumDP + w - T2;
		n[3]  = T1 - minf1;
	}
   // assume 3-rd cond must be satisfied
	sumDP = Th_subSum( tObj1 , Vec_IntSize(tObj1->weights) , T1-1 , 1 );
	n[4]  = T1 - sumDP; 
	sumDP = Th_subSum( tObj2 , nFanin , T2-1 , 1 );
	n[5]  = sumDP + w - T2; 
}

/**Function*************************************************************

  Synopsis    [Using DP to compute subset sum.]

  Description [Interface from Thre_S* to Vec_Int_t*.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Th_subSum( const Thre_S * tObj , int nFanin , int bound , int fMax )
{
	// nFanin : unwanted fanin
	// fMax   : flag for Max(1) or min(0)
	Vec_Int_t * numbers;
	int optValue;
	numbers   = Th_subSumCollectNum( tObj , nFanin );
   optValue  = Th_subSumSolveDP( numbers , bound , fMax );
	Vec_IntFree( numbers );
	if ( optValue == TH_MIN ) {
		printf(" tObj id=%d , nFanin=%d , bound=%d , fMax=%d\n" , tObj->Id , nFanin , bound , fMax);
		assert(0); // no solutions , something wrong!
	}
	return optValue;	
}

Vec_Int_t*
Th_subSumCollectNum( const Thre_S * tObj , int nFanin )
{
	Vec_Int_t * numbers;
	int  size , Entry , i;
	size    = ( nFanin < Vec_IntSize( tObj->weights ) ) ? 
		       ( Vec_IntSize(tObj->weights)-1 ) : Vec_IntSize(tObj->weights);
	numbers = Vec_IntStart( size );
	Vec_IntForEachEntry( tObj->weights , Entry , i )
	{
		if ( i < nFanin )      Vec_IntWriteEntry( numbers , i   , Entry );
		else if ( i > nFanin ) Vec_IntWriteEntry( numbers , i-1 , Entry );
		else;
	}
	return numbers;
}

/**Function*************************************************************

  Synopsis    [Using DP to compute subset sum.]

  Description [Core solving process.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Th_subSumSolveDP( Vec_Int_t * numbers , int bound , int fMax )
{
	int ** matrix;
	int num , maxSum , minSum , range , Entry , optValue , i , j;
   
	// 1. prepare matrix
	num    = Vec_IntSize( numbers );
	maxSum = MaxF( numbers , num );
	minSum = MinF( numbers , num );
	if ( bound == 0 )     return 0;
	if ( bound > maxSum ) return fMax ? maxSum : TH_MIN;
	if ( bound < minSum ) return fMax ? TH_MIN : minSum;
	range  = maxSum - minSum + 1;
	matrix = Th_Alloc2DArray( num , range );

   // 2. compute matix
	Entry  = Vec_IntEntry( numbers , 0 );
	for ( j = 0 ; j < range ; ++j ) {
		if ( Entry == minSum + j ) matrix[0][j] = 1;
		else matrix[0][j] = 0;
	}
	for ( i = 1 ; i < num ; ++i ) {
		Entry = Vec_IntEntry( numbers , i );
	   for ( j = 0 ; j < range ; ++j ) {
		   if ( matrix[i-1][j] || Entry == minSum + j ) 
				matrix[i][j] = 1;
			else if ( j - Entry >= 0 && j - Entry <= range - 1 )
			   matrix[i][j] = matrix[i-1][j-Entry];
			else matrix[i][j] = 0;
		}
	}
	// 3. find optimal value
   optValue = Th_subSumOptValue( matrix[num-1] , minSum , range , bound , fMax );	
	Th_Free2DArray( matrix , num );
   return optValue;
}

int**
Th_Alloc2DArray( int row , int column )
{
   int ** matrix;
	int i;

	matrix = (int**) malloc( row * sizeof(int*) );
	for ( i = 0 ; i < row ; ++i )
	   matrix[i] = (int*) malloc( column * sizeof(int) );
	return matrix;
}

void
Th_Free2DArray( int ** matrix , int row )
{
	assert( matrix );
	int i;
	for ( i = 0 ; i < row ; ++i ) free( matrix[i] );
	free( matrix );
}

int
Th_subSumOptValue( int * matLastRow , int minSum , int range , int bound , int fMax )
{
	int j;
   if ( fMax ) { // maximized
      for ( j = bound - minSum ; j > -1 ; --j )
		   if ( matLastRow[j] ) return minSum + j;
	}
	else { // minimized
		for ( j = bound - minSum ; j < range ; ++j )
			if ( matLastRow[j] ) return minSum + j;
	}
	return TH_MIN;
}

/**Function*************************************************************

  Synopsis    [If : Solve for K and L analytically.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Pair_S*
Th_IfSolveKL( int * cond , int * n , int w )
{
#ifdef CHECK
	if ( !Th_IfCheckCoeff( cond , n ) ) {
		printf( "Th_IfSolveKL() : coefficient check fails...\n" );
		assert(0);
	}
#endif
	Pair_S * pair;
	pair = ABC_ALLOC( Pair_S , 1 );
	pair->IntK = pair->IntL = 0;

	if ( cond[0] && cond[1] ) {
		if ( (1 > n[0]*n[3]) && (n[1] > n[2]*n[3]) ) {
		   double x , y , slope;
	      x     = (n[0] + 1) / (1 - n[0]*n[3]);
			y     = n[3]*x + 1;
			slope = y / x;
			if ( (double)n[1] >= n[2]*slope ) {
	         pair->IntL = Th_DPDivRound( n[0]+1 , 1-n[0]*n[3] );
		      pair->IntK = pair->IntL*n[3] + 1;
			}
			else {
	         pair->IntL = Th_DPDivRound( n[2] , n[1]-n[2]*n[3] );
		      pair->IntK = pair->IntL*n[3] + 1;
			} 
		}
	}
	else if ( cond[0] ) {
      if ( 1 > n[0]*n[3] ) {
	      pair->IntL = Th_DPDivRound( n[0]+1 , 1-n[0]*n[3] );
		   pair->IntK = pair->IntL*n[3] + 1;
		}
	}
	else if ( cond[1] ) {
      if ( n[1] > n[2]*n[3] ) {
	      pair->IntL = Th_DPDivRound( n[2] , n[1]-n[2]*n[3] );
		   pair->IntK = pair->IntL*n[3] + 1;
		}
	}
	else {
		pair->IntL = 1;
		pair->IntK = n[3] + 1;
	}
#ifdef CHECK
	if ( !Th_IfCheckKL( pair , cond , n ) ) {
		printf( "Th_IfSolveKL() : KL check fails...\n" );
		assert(0);
	}
#endif
	// restrict k,l <= 10
	//if ( pair->IntK > 10 || pair->IntL > 10 )
	  // pair->IntK = pair->IntL = 0;
	return pair;
}

int
Th_IfCheckCoeff( int * cond , int * n )
{
	if ( cond[0] && !( n[0] >=0 ) )                   return 0;
	if ( cond[1] && !( n[1] >= 1 && n[2] >= 1 ) )     return 0;
	if ( !( n[3] >= 0 ) )                             return 0;
	return 1;
}

int
Th_IfCheckKL( Pair_S * pair , int * cond , int * n )
{
	int k , l;
	k = pair->IntK;
	l = pair->IntL;

	//printf( "\tTh_IfCheckKL() : k = %d , l = %d\n" , k , l );
	if ( k == 0 && l == 0 )                         return 1;
	if ( !(k > 0 && l > 0) )                        return 0;
	if ( cond[0] && !( l >= k * n[0] + 1 ) )        return 0;
	if ( cond[1] && !( l * n[1] >= k * n[2] ) )     return 0;
	if ( !( k >= l * n[3] + 1 ) )                   return 0;
	return 1;
}

/**Function*************************************************************

  Synopsis    [DP : Solve for K and L analytically.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Pair_S*
Th_DPSolveKL( int * cond , int * n , int w )
{
#ifdef CHECK
	if ( !Th_DPCheckCoeff( cond , n , w ) ) {
		printf( "Th_DPSolveKL() : coefficient check fails...\n" );
		assert(0);
	}
#endif
	Pair_S * pair;
	pair = ABC_ALLOC( Pair_S , 1 );
	pair->IntK = pair->IntL = 0;

	if ( cond[0] && cond[1] ) {
		if ( (n[0]*n[4] > n[1]*n[5]) && (n[2]*n[4] > n[3]*n[5]) ) {
		   double x , y , slope;
	      x     = (n[1] + n[4]) / (n[0]*n[4] - n[1]*n[5]);
			y     = (n[5]*x + 1) / n[4];
			slope = y / x;
			if ( (double)n[2] >= n[3]*slope ) {
	         pair->IntL = Th_DPDivRound( n[1]+n[4] , n[0]*n[4]-n[1]*n[5] );
			   pair->IntK = Th_DPFindIntK( pair->IntL , n[4] , n[5] , n[0] , n[1] , 1 );
			}
			else {
	         pair->IntL = Th_DPDivRound( n[3] , n[2]*n[4]-n[3]*n[5] );
			   pair->IntK = Th_DPFindIntK( pair->IntL , n[4] , n[5] , n[2] , n[3] , 0 );
			} 
		}
	}
	else if ( cond[0] ) {
      if ( n[0]*n[4] > n[1]*n[5] ) {
	      pair->IntL = Th_DPDivRound( n[1]+n[4] , n[0]*n[4]-n[1]*n[5] );
			pair->IntK = Th_DPFindIntK( pair->IntL , n[4] , n[5] , n[0] , n[1] , 1 );
		}
	}
	else if ( cond[1] ) {
      if ( n[2]*n[4] > n[3]*n[5] ) {
	      pair->IntL = Th_DPDivRound( n[3] , n[2]*n[4]-n[3]*n[5] );
			pair->IntK = Th_DPFindIntK( pair->IntL , n[4] , n[5] , n[2] , n[3] , 0 );
		}
	}
	else {
		pair->IntL = 1;
		pair->IntK = ( (n[5] + 1) / n[4] ) + 1;
	}
#ifdef CHECK
	if ( !Th_DPCheckKL( pair , cond , n ) ) {
		printf( "Th_DPSolveKL() : KL check fails...\n" );
		assert(0);
	}
#endif
	// restrict k,l <= 10
	//if ( pair->IntK > 10 || pair->IntL > 10 )
	  // pair->IntK = pair->IntL = 0;
	return pair;
}

int
Th_DPCheckCoeff( int * cond , int * n , int w )
{
	if ( cond[0] && !( n[0] >= 1 && n[1] >= 0 ) )     return 0;
	if ( cond[1] && !( n[2] >= w && n[3] >= 1 ) )     return 0;
	if ( !( n[4] >= 1 && n[5] <= w-1 && n[5] >= 0 ) ) return 0;
	return 1;
}

int
Th_DPCheckKL( Pair_S * pair , int * cond , int * n )
{
	int k , l;
	k = pair->IntK;
	l = pair->IntL;

	printf( "\tTh_DPCheckKL() : k = %d , l = %d\n" , k , l );
	if ( k == 0 && l == 0 )                         return 1;
	if ( !(k > 0 && l > 0) )                        return 0;
	if ( cond[0] && !( l * n[0] >= k * n[1] + 1 ) ) return 0;
	if ( cond[1] && !( l * n[2] >= k * n[3] ) )     return 0;
	if ( !( k * n[4] >= l * n[5] + 1 ) )            return 0;
	return 1;
}

int
Th_DPDivRound( int num1 , int num2 )
{
	return ( num1 % num2 == 0 ) ? (num1 / num2) : (num1 / num2)+1;
}

int
Th_DPFindIntK( int IntL , int n4 , int n5 , int a , int b , int c )
{
	int IntK;
	while ( 1 ) {
		if ( (n5*IntL+1) % n4 == 0 ) return (n5*IntL+1) / n4;
	   IntK = Th_DPDivRound( n5*IntL+1 , n4 );
		if ( IntK <= (double)(a*IntL-c)/(double)(b) ) return IntK;
		else ++IntL;
	}
}

/**Function*************************************************************

  Synopsis    [Dump data for debugging.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_dumpCond( int * cond )
{
   if ( cond[0] ) printf("cond1 is sat.\n");
   if ( cond[1] ) printf("cond2 is sat.\n");
   printf("cond3 is sat.\n");
}

void
Th_dumpCoeff( int * cond , int * n )
{
   if ( cond[0] ) {
	   printf("\tcond1 is sat:\n");
		printf("\tn[%d] = %d , n[%d] = %d\n" , 0 , n[0] , 1 , n[1] );
	}
   if ( cond[1] ) {
	   printf("\tcond2 is sat:\n");
		printf("\tn[%d] = %d , n[%d] = %d\n" , 2 , n[2] , 3 , n[3] );
	}
	printf("\tcond3 is sat:\n");
   printf("\tn[%d] = %d , n[%d] = %d\n" , 4 , n[4] , 5 , n[5] );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
