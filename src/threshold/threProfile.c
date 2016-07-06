/**CFile****************************************************************
 
  FileName    [threProfile.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [threshold.]
  
  Synopsis    [Profiling threshold network.]

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
// main functions
void Th_ProfileInit      ();
void Th_ProfilePrint     ();
// helper functions
void Th_ProfilePrint     ();
int  Th_ProfileCheck     ();

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Initialize global profiler.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_ProfileInit()
{
   int i;
   thProfiler.numTotal           = 0;
	thProfiler.numSingleFoutIf    = 0;
	thProfiler.numSingleFoutIff   = 0;
	thProfiler.numSingleFoutFail  = 0;
	thProfiler.numMultiFout       = 0;
	thProfiler.numMultiFoutOk     = 0;
	thProfiler.numTwoFout         = 0;
	thProfiler.numTwoFoutOk       = 0;
	thProfiler.numNotThNode       = 0;
	// threshold --> mux redundancy test
   thProfiler.numRedundancy      = 0;
   for ( i = 0 ; i < 50 ; ++i ) thProfiler.redund[i] = 0;
}

/**Function*************************************************************

  Synopsis    [Initialize global profiler.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_ProfilePrint()
{
	if ( !Th_ProfileCheck() ) {
	   printf( "Th_ProfilePrint() : check fail ...\n" );
		assert(0);
	}
   printf( "Threshold collapse profiling results:\n" );
	printf( "\tNumber of trials             = %d\n" , thProfiler.numTotal            );
	printf( "\tNumber of if-cond            = %d\n" , thProfiler.numSingleFoutIf     );
	printf( "\tNumber of iff-cond           = %d\n" , thProfiler.numSingleFoutIff    );
	printf( "\tNumber of single fanout fail = %d\n" , thProfiler.numSingleFoutFail   );
	printf( "\tNumber of multi fanouts      = %d\n" , thProfiler.numMultiFout        );
	printf( "\tNumber of multi fanouts Ok   = %d\n" , thProfiler.numMultiFoutOk      );
	printf( "\tNumber of two fanouts        = %d\n" , thProfiler.numTwoFout          );
	printf( "\tNumber of two fanouts Ok     = %d\n" , thProfiler.numTwoFoutOk        );
	printf( "\tNumber of none Th nodes      = %d\n" , thProfiler.numNotThNode        );
}

int
Th_ProfileCheck()
{
   return ( thProfiler.numTotal == thProfiler.numSingleFoutIf   +
			                          thProfiler.numSingleFoutIff  +
											  thProfiler.numSingleFoutFail + 
											  thProfiler.numMultiFout      +
											  thProfiler.numNotThNode      );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
