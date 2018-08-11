/**CFile****************************************************************
 
  FileName    [threCmd.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Threshold Logic Commands.]
  
  Synopsis    [Command file.]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [Ver. 1.0. Started - Mar. 11, 2016.]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "threshold.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// I/O commands
static int Abc_CommandReadThreshold    ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandWriteThreshold   ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandPrintThreshold   ( Abc_Frame_t * pAbc, int argc, char ** argv );
// Synthesis commands
static int Abc_CommandAig2Th           ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandMerge            ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandTh2Mux           ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandTh2Blif          ( Abc_Frame_t * pAbc, int argc, char ** argv );
// Verification commands
static int Abc_CommandPB_Threshold     ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandCNF_Threshold    ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandThVerify         ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandNZ               ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandOAO              ( Abc_Frame_t * pAbc, int argc, char ** argv );
// misc commands
static int Abc_CommandTestTH           ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandProfileTh        ( Abc_Frame_t * pAbc, int argc, char ** argv );
static void Th_GlobalInit();

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Initialize threshold package]

  Description [Initialize global variables and add commands.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Th_GlobalInit()
{
	 globalRef = 0;
    current_TList = NULL;
    //another_TList = NULL;
    cut_TList     = NULL;
	 Th_ProfileInit();
}

void 
Threshold_Init( Abc_Frame_t *pAbc)
{
	 Th_GlobalInit();
    Cmd_CommandAdd( pAbc, "z Alcom", "read_th"     , Abc_CommandReadThreshold,  1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "write_th"    , Abc_CommandWriteThreshold, 0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "print_th"    , Abc_CommandPrintThreshold, 0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "aig2th"      , Abc_CommandAig2Th,         1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "merge_th"    , Abc_CommandMerge,          1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "th2blif"     , Abc_CommandTh2Blif,        0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "th2mux"      , Abc_CommandTh2Mux,         1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "PB_th"       , Abc_CommandPB_Threshold,   0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "CNF_th"      , Abc_CommandCNF_Threshold,  0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "thverify"    , Abc_CommandThVerify,       0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "NZ"          , Abc_CommandNZ,             1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "OAO"         , Abc_CommandOAO,            0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "test_th"     , Abc_CommandTestTH,         1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "profile_th"  , Abc_CommandProfileTh,      0 );
}

void 
Threshold_End( Abc_Frame_t *pAbc )
{
	if ( current_TList ) { DeleteTList( current_TList ); current_TList = NULL; }
	//if ( another_TList ) { DeleteTList( another_TList ); another_TList = NULL; }
	if ( cut_TList )     { DeleteTList( cut_TList ); cut_TList = NULL; }
}

/**Function*************************************************************

  Synopsis    [Threshold network reader.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandReadThreshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{   
    FILE * pFile;
    char ** pArgvNew;
    char * FileName;
    int nArgcNew;
    int c;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    pArgvNew = argv + globalUtilOptind;
    nArgcNew = argc - globalUtilOptind;
    if ( nArgcNew != 1 )
    {
        Abc_Print( -1, "There is no file name.\n" );
        return 1;
    }

    // get the input file name
    FileName = pArgvNew[0];
    if ( (pFile = fopen( FileName, "r" )) == NULL ) {
        Abc_Print( -1, "Cannot open input file \"%s\". ", FileName );
        if ( (FileName = Extra_FileGetSimilarName( FileName, ".blif", NULL, NULL, NULL, NULL )) )
            Abc_Print( 1, "Did you mean \"%s\"?", FileName );
        Abc_Print( 1, "\n" );
        return 1;
    }
    fclose( pFile );
    if (current_TList != NULL) {
       DeleteTList(current_TList);
       current_TList = NULL;
       Abc_Print( 0, "Original current_TList destroyed.\n" );
       //if (another_TList != NULL) DeleteTList(another_TList);
       //another_TList = func_readFileOAO(FileName);
    }
    current_TList = func_readFileOAO(FileName);
    return 0;

usage:
    Abc_Print( -2, "usage: read_th [-h] <file>\n" );
    Abc_Print( -2, "\t         a reader for threshold gate '.th' files\n" );
    Abc_Print( -2, "\t-h     : print the command usage\n");
    Abc_Print( -2, "\t<file> : the file name\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Threshold network writer.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandWriteThreshold( Abc_Frame_t * pAbc,int argc, char ** argv )
{
    char ** pArgvNew;
    char * FileName;
    int nArgcNew;
    int c;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    pArgvNew = argv + globalUtilOptind;
    nArgcNew = argc - globalUtilOptind;
    if ( nArgcNew != 1 )
    {
        Abc_Print( -1, "There is no file name.\n" );
        return 1;
    }

    // get the input file name
    FileName = pArgvNew[0];
    if ( !current_TList ) {
       printf("[Error] current threshold gateList is empty!!\n");
       return 1;
    }
    dumpTh2FileNZ( current_TList , FileName );
    return 0;

usage:
    Abc_Print( -2, "usage: write_th [-h] <file>\n" );
    Abc_Print( -2, "\t         dump function  for threshold gate '.th' files\n" );
    Abc_Print( -2, "\t-h     : print the command usage\n");
    Abc_Print( -2, "\t<file> : the file name\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Print statistics of a network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandPrintThreshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pErr;
    int c;
    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }

    if ( current_TList == NULL )
    {
        fprintf( pErr, "\tEmpty threshold network.\n" );
        return 1;
    }

    Th_PrintStat(current_TList);
    
	 return 0;
usage:
    fprintf( pErr, "usage:    print_th [-h]\n" );
    fprintf( pErr, "\t        print TH network statistics\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Convert AIG to threshold network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandAig2Th( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk , * pNtkRes;
    int c;
    int fAllNodes      = 0;
    int fRecord        = 1;
    int fCleanup       = 0;
	 int fRemoveLatches = 0;
    pNtk = Abc_FrameReadNtk(pAbc);
    pOut = Abc_FrameReadOut(pAbc);
    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }

    if ( pNtk == NULL )
    {
        fprintf( pErr, "\tEmpty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }
    if ( !Abc_NtkIsStrash( pNtk ) )
    {
        fprintf(pErr, "\tnetwork is not AIG, Strashing...\n");
        // get the new network
        pNtkRes = Abc_NtkStrash( pNtk, fAllNodes, fCleanup, fRecord );
        if ( pNtkRes == NULL )
        {
            fprintf( pErr, "\tStrashing has failed.\n" );
            return 1;
        }
        // replace the current network
        Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
        pNtk = pNtkRes;
    }
    if ( current_TList ) {
		 fprintf( pErr , "\tOriginal current_TList destroyed.\n" );
		 DeleteTList( current_TList );
	 }
    current_TList = aig2Th( pNtk );
	 fprintf( pErr , "\tTList constructed from AIG.\n" );

    return 0;
usage:
    fprintf( pErr, "usage:    aig2th [-h]\n" );
    fprintf( pErr, "\t        naive conversion from AIG to TList.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Iteratively collapse a threshold network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandMerge( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pErr;
	 int fIterative;
    int c , i , fOutBound;
	 abctime clk;
    pErr = Abc_FrameReadErr(pAbc);
	 fIterative = 0;
	 fOutBound  = -1;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "Bih" ) ) != EOF )
    {
       switch ( c )
		 {
           case 'B':
               if ( globalUtilOptind >= argc ) {
                  Abc_Print( -1, "Command line switch \"-B\" should be followed by an integer.\n" );
                  goto usage;
               }
               fOutBound = atoi(argv[globalUtilOptind]);
               globalUtilOptind++;
               if ( fOutBound < 1 ) goto usage;
               break;
		    case 'i':
			    fIterative ^= 1;
			    break;
		    default:
             goto usage;
		 }
    }
	 if ( current_TList == NULL ) {
        fprintf( pErr, "\tEmpty threshold network.\n" );
        return 1;
	 }
	 clk = Abc_Clock();
	 if ( fOutBound == -1 ) Th_CollapseNtk( current_TList , fIterative , fOutBound );
	 else {
	    for ( i = 1 ; i <= fOutBound ; ++i )
	       Th_CollapseNtk( current_TList , fIterative , i );
	 }
    // sort current_TList and clean up NULL objects
    Th_NtkDfs();
	 Abc_PrintTime( 1 , "collapse time : " , Abc_Clock()-clk );
	 return 0;
usage:
    fprintf( pErr, "usage:    merge_th [-B <num>] [-ih]\n" );
    fprintf( pErr, "\t        merging process for TList.\n");
    fprintf( pErr, "\t-B num   : collapse from single fanout to num fanout\n");
    fprintf( pErr, "\t-i       : toggle iterative collapse\n");
    fprintf( pErr, "\t-h       : print the command usage\n");
	 return 1;
}

/**Function*************************************************************

  Synopsis    [Write out threshold network as blif file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandTh2Blif( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pOut, * pErr;
    char ** pArgvNew;
	 char *  FileName;
	 int     nArgcNew , c;
	 abctime clk;
	 
	 pOut = Abc_FrameReadOut(pAbc);
    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    pArgvNew = argv + globalUtilOptind;
    nArgcNew = argc - globalUtilOptind;
    if ( nArgcNew != 1 ) {
       Abc_Print( -1, "There is no file name.\n" );
       return 1;
    }
    // get the output file name
    FileName = pArgvNew[0];
    if ( !current_TList ) {
		 fprintf( pErr , "Empty threshold network.\n" );
		 return 1;
	 }
	 clk = Abc_Clock();
    Th_WriteBlif( current_TList , FileName );
	 Abc_PrintTime( 1 , "blif gen time " , Abc_Clock()-clk );
	 fprintf( pErr , "File %s is written.\n" , FileName );
    return 0;
usage:
    fprintf( pErr, "usage:    th2blif [-h] <file>\n" );
    fprintf( pErr, "\t        write out threshold network as blif file\n");
    fprintf( pErr, "\tfile  : output file name\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Convert threshold network to mux trees.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandTh2Mux( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pOut, * pErr;
	 Abc_Ntk_t * pNtk , * pNtkRes;
    int fDynamic , fAhead , c;
	 abctime clk;
	 
    fDynamic = 1;
    fAhead   = 0;
	 pNtk = Abc_FrameReadNtk(pAbc);
	 pOut = Abc_FrameReadOut(pAbc);
    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "adh" ) ) != EOF )
    {
       switch ( c )
		 {
		    case 'a':
			    fAhead ^= 1;
			    break;
		    case 'd':
			    fDynamic ^= 1;
			    break;
		    case 'h':
             goto usage;
		    default:
             goto usage;
		 }
    }

    if ( !current_TList ) {
		 fprintf( pErr , "Empty threshold network.\n" );
		 return 1;
	 }

	 clk     = Abc_Clock();
    pNtkRes = Th_Ntk2Mux( current_TList , fDynamic , fAhead );
    if ( !pNtkRes ) {
       Abc_Print( -1 , "Construct mux trees from threshold fail\n" );
       return 1;
    }
    if ( pNtk ) Abc_Print( 1 , "Current network is replaced\n" );
    Abc_FrameReplaceCurrentNetwork( pAbc , pNtkRes );
	 Abc_PrintTime( 1 , "mux convert time " , Abc_Clock()-clk );

    return 0;
usage:
    fprintf( pErr, "usage:    th2mux [-adh]\n" );
    fprintf( pErr, "\t        convert threshold network to mux trees\n");
    fprintf( pErr, "\t-a    : toggles look-ahead dynamic variable selection [default = %s]\n" , fAhead ? "yes" : "no" );
    fprintf( pErr, "\t-d    : toggles dynamic variable selection [default = %s]\n" , fDynamic ? "yes" : "no" );
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Write PB for equivalence checking.]

  Description [AIG v.s. TH]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int Abc_CommandPB_Threshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk, * pNtkRes;
    char ** pArgvNew;
    char * FileName;
    int nArgcNew;
    int c;
    int fAllNodes      = 0;
    int fRecord        = 1;
    int fCleanup       = 0;
	 int fRemoveLatches = 0;
	 abctime clk;
    pNtk = Abc_FrameReadNtk(pAbc);
    pOut = Abc_FrameReadOut(pAbc);
    pErr = Abc_FrameReadErr(pAbc);
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    pArgvNew = argv + globalUtilOptind;
    nArgcNew = argc - globalUtilOptind;
    if ( nArgcNew != 1 ) {
        Abc_Print( -1, "There is no file name.\n" );
        return 1;
    }
    // get the input file name
    FileName = pArgvNew[0];
    if ( pNtk == NULL ) {
        fprintf( pErr, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }
    if ( !Abc_NtkIsStrash( pNtk ) ) {
        fprintf(pErr, "network is not AIG, Strashing...\n");
        // get the new network
        pNtkRes = Abc_NtkStrash( pNtk, fAllNodes, fCleanup, fRecord );
        if ( pNtkRes == NULL )
        {
            fprintf( pErr, "Strashing has failed.\n" );
            return 1;
        }
        // replace the current network
        Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
        pNtk = pNtkRes;
    }
    if (current_TList == NULL) {
        fprintf(pErr, "ERROR: current thresholdList is empty!!\n\n");
        return 1;
    }
	 clk = Abc_Clock();
    func_EC_writePB(pNtk, current_TList, FileName);
	 Abc_PrintTime( 1 , "pb gen time : " , Abc_Clock()-clk );
    return 0;
usage:
    fprintf( pErr, "usage:  PB_th [-h] <fileName> \n" );
    fprintf( pErr, "\t       given strashNtk and thresholdNtk, print out Pseudo Boolean constraints for minisat+ to solve EC.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Write CNF for equivalence checking.]

  Description [AIG v.s. TH]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandCNF_Threshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{    
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk, * pNtkRes;
    char ** pArgvNew;
    char * FileName;
    int nArgcNew;
    int c;
    int fAllNodes      = 0;
    int fRecord        = 1;
    int fCleanup       = 0;
	 int fRemoveLatches = 0;
	 abctime clk;
    pNtk = Abc_FrameReadNtk(pAbc);
    pOut = Abc_FrameReadOut(pAbc);
    pErr = Abc_FrameReadErr(pAbc);
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    pArgvNew = argv + globalUtilOptind;
    nArgcNew = argc - globalUtilOptind;
    if ( nArgcNew != 1 )
    {
        Abc_Print( -1, "There is no file name.\n" );
        return 1;
    }
    // get the input file name
    FileName = pArgvNew[0];
    if ( pNtk == NULL ) {
        fprintf( pErr, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }
    if ( !Abc_NtkIsStrash( pNtk ) ) {
        fprintf(pErr, "network is not AIG, Strashing...\n");
        // get the new network
        pNtkRes = Abc_NtkStrash( pNtk, fAllNodes, fCleanup, fRecord );
        if ( pNtkRes == NULL )
        {
            fprintf( pErr, "Strashing has failed.\n" );
            return 1;
        }
        // replace the current network
        Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
        pNtk = pNtkRes;
    }
    if (current_TList == NULL) {
        fprintf(pErr, "ERROR: current thresholdList is empty!!\n\n");
        return 1;
    }
	 clk = Abc_Clock();
    func_EC_writeCNF(pNtk, current_TList, FileName);
	 Abc_PrintTime( 1 , "ec gen time : " , Abc_Clock()-clk );
    return 0;
usage:
    fprintf( pErr, "usage:  CNF_th [-h] <fileName>\n" );
    fprintf( pErr, "\t       given strashNtk and thresholdNtk, print out CNF for minisat to solve EC.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Threshold logic verification.]

  Description [Generate a CNF/PB formula for eq checking for 2 TH ntks.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandThVerify( Abc_Frame_t * pAbc, int argc, char ** argv )
{
   char ** pArgvNew;
   int nArgcNew, fVer, c;
   fVer = 0;
   Extra_UtilGetoptReset();
   while ( ( c = Extra_UtilGetopt( argc , argv , "Vh" ) ) != EOF )
   {
       switch ( c )
       {
       case 'V':
          if ( globalUtilOptind >= argc ) {
              Abc_Print( -1 , "Command line switch \"-V\" should be followed by a positive integer.\n" );
              goto usage;
          }
          fVer = atoi( argv[globalUtilOptind] );
          globalUtilOptind++;
          if ( fVer < 0 || fVer > 2 ) goto usage;
          break;
       case 'h':
       default:
           goto usage;
       }
   }
   pArgvNew = argv + globalUtilOptind;
   nArgcNew = argc - globalUtilOptind;
   if ( nArgcNew != 2 ) {
      Abc_Print( -1, "Extra/missing files are given: expected %d, received %d!\n" , 2 , nArgcNew );
      goto usage;
   }
   if ( current_TList ) {
      Abc_Print( 0, "Original current_TList destroyed.\n" );
	   DeleteTList( current_TList ); 
      current_TList = NULL;
   }
   current_TList = func_readFileOAO( pArgvNew[0] );
   if ( !current_TList ) goto usage;
   if ( cut_TList ) {
      Abc_Print( 0, "Original cut_TList destroyed.\n" );
	   DeleteTList( cut_TList ); 
      cut_TList = NULL;
   }
   cut_TList = func_readFileOAO( pArgvNew[1] );
   if ( !cut_TList ) { DeleteTList(current_TList); goto usage; }
   if ( fVer == 0 ) func_EC_compareTH( current_TList, cut_TList );
   else if ( fVer == 1 ) func_CNF_compareTH( current_TList, cut_TList );
   else assert(0);
   return 0;
usage:
    Abc_Print( -2, "usage:  thverify [-V <num>] [-h] <file1> <file2>\n" );
    Abc_Print( -2, "\t          eq check between file1 and file2 via PB/CNF (output file name: compTH.opb/dimacs)\n");
    Abc_Print( -2, "\t-V <num> :toggling verification methods (0: PB; 1: CNF), default = %d\n", fVer );
    Abc_Print( -2, "\t<file1>  :the first TH file to be verified\n");
    Abc_Print( -2, "\t<file2>  :the second TH file to be verified\n");
    Abc_Print( -2, "\t-h       :print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Equivalence checking.]

  Description [TH v.s. TH]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandNZ( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c;
    abctime clk;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    if ( !cut_TList ) {
        Abc_Print( -1, "cut_TList is empty!\n" );
        goto usage;
    }
    if ( !current_TList ) {
        Abc_Print( -1, "current_TList is empty!\n" );
        goto usage;
    }
    clk = Abc_Clock();
    func_EC_compareTH( current_TList, cut_TList );
    Abc_PrintTime( 1 , "PB translation time : " , Abc_Clock() - clk );
    return 0;
usage:
    Abc_Print( -2, "usage:  NZ [-h]\n" );
    Abc_Print( -2, "\t        eq check between cut_TList and current_TList by PB (filename: compTH.opb).\n");
    Abc_Print( -2, "\t-h    : print the command usage\n");
    return 1;
}

int 
Abc_CommandOAO( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    int c;
    abctime clk;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
       switch(c)
       {
          case 'h':
             goto usage;
       }
    }
    if ( !cut_TList ) {
        Abc_Print( -1, "cut_TList is empty!\n" );
        goto usage;
    }
    if ( !current_TList ) {
        Abc_Print( -1, "current_TList is empty!\n" );
        goto usage;
    }
    clk = Abc_Clock();
    func_CNF_compareTH( current_TList, cut_TList );
    Abc_PrintTime( 1 , "CNF translation time : " , Abc_Clock() - clk );
    return 0;
usage:
    Abc_Print( -2, "usage:  OAO [-h]\n" );
    Abc_Print( -2, "\t        eq check between cut_TList and current_TList by CNF (filename: compTH.dimacs).\n");
    Abc_Print( -2, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [Testing interface.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandTestTH( Abc_Frame_t * pAbc, int argc, char ** argv )
{
   dumpTh2FileNZ( cut_TList , "compTH_1.th" );
   dumpTh2FileNZ( current_TList , "compTH_2.th" );
   return 0;
}

/**Function*************************************************************

  Synopsis    [Print profiling information.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int 
Abc_CommandProfileTh( Abc_Frame_t * pAbc, int argc, char ** argv )
{
	Th_ProfilePrint();
	return 0;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
