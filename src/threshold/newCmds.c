/**CFile****************************************************************
 
  FileName    [threCmd.c] 

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Threshold Logic Commands.]
  
  Synopsis    [Command file.]

  Author      [AlcomLab, NTU ]


***********************************************************************/

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "threshold.h"

/**********************************
 * Threshold Command Definition   *
 * ****************************** */
static int Abc_CommandThIf             ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandThreshold        ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandAig2Th           ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandMerge            ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandPrintThreshold   ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandOAO              ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandNZ               ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandReadThreshold    ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandWriteThreshold   ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandEC_Threshold     ( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Abc_CommandCNF_Threshold    ( Abc_Frame_t * pAbc, int argc, char ** argv );
/////////////////////////////
// Command Initialization //
////////////////////////////
void Threshold_Init( Abc_Frame_t *pAbc){
    Cmd_CommandAdd( pAbc, "z Alcom", "OAO"         , Abc_CommandOAO,            0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "NZ"          , Abc_CommandNZ,             1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "aig2th"      , Abc_CommandAig2Th,         1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "merge_th"    , Abc_CommandMerge,          1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "print_th"    , Abc_CommandPrintThreshold, 0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "thif"        , Abc_CommandThIf,           1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "threshold"   , Abc_CommandThreshold,      1 );
    Cmd_CommandAdd( pAbc, "z Alcom", "read_th"     , Abc_CommandReadThreshold,  0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "write_th"    , Abc_CommandWriteThreshold, 0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "EC_th"       , Abc_CommandEC_Threshold,   0 );
    Cmd_CommandAdd( pAbc, "z Alcom", "CNF_th"      , Abc_CommandCNF_Threshold,   0 );
}
void Threshold_End( Abc_Frame_t *pAbc){}

///////////////////////////////////////////////////////////
//        Are you ready?              Let's GO~         ///
///////////////////////////////////////////////////////////
int Abc_CommandOAO( Abc_Frame_t * pAbc, int argc, char ** argv )
{    
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk, * pNtkRes;
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
        goto usage;
    }

    if ( pNtk == NULL )
    {
        fprintf( pErr, "Empty network.\n" );
        return 1;
    }

    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }

    if ( !Abc_NtkIsStrash( pNtk ) )
    {
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
    if (current_TList == NULL)
    {
        fprintf(pErr, "ERROR: current thresholdList is empty!!\n\n");
        return 1;
    }
    func_EC_writeCNF(pNtk, current_TList);
    return 0;
usage:
    fprintf( pErr, "usage:  CNF_check [-h]\n" );
    fprintf( pErr, "\t       given strashNtk and thresholdNtk, print out CNF file for minisat to solve EC.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

int Abc_CommandNZ( Abc_Frame_t * pAbc, int argc, char ** argv )
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
        goto usage;
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
    if ( current_TList != NULL ) {
		 fprintf( pErr , "\tOriginal TList destroyed...\n" );
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
	 Mapping for threshold cut 
***********************************************************************/
int Abc_CommandThIf(Abc_Frame_t * pAbc,int argc,char ** argv)
{   
    return 1;
}

int Abc_CommandThreshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    return 1;
}

/**Function*************************************************************
	 Merging threshold netlist 
***********************************************************************/

int Abc_CommandMerge( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pErr;
    int c;

    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        goto usage;
    }

	 if ( current_TList == NULL ) {
        fprintf( pErr, "\tEmpty threshold network.\n" );
        return 1;
	 }
    
	 mergeThreNtk_Iter(current_TList);
    
	 return 0;

usage:
    fprintf( pErr, "usage:    merge [-h]\n" );
    fprintf( pErr, "\t        naive merging process for TList.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
	 return 1;
}

/**Function*************************************************************
    Convert AIG to TH network
***********************************************************************/

int Abc_CommandAig2Th( Abc_Frame_t * pAbc, int argc, char ** argv )
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
        goto usage;
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
    if ( current_TList != NULL) {
		 fprintf( pErr , "\tOriginal TList destroyed...\n" );
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
    Print Th network statistics
***********************************************************************/

int Abc_CommandPrintThreshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    FILE * pErr;
    int c;
    pErr = Abc_FrameReadErr(pAbc);

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        goto usage;
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

//////////////////////////////////////
// 
////////////////////////////////////
int Abc_CommandReadThreshold( Abc_Frame_t * pAbc, int argc, char ** argv )
{   
    FILE * pFile;
    char ** pArgvNew;
    char * FileName, * pTemp;
    int nArgcNew;
    int c;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        goto usage;
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
    if ( (pFile = fopen( FileName, "r" )) == NULL )
    {
        Abc_Print( -1, "Cannot open input file \"%s\". ", FileName );
        if ( (FileName = Extra_FileGetSimilarName( FileName, ".blif", NULL, NULL, NULL, NULL )) )
            Abc_Print( 1, "Did you mean \"%s\"?", FileName );
        Abc_Print( 1, "\n" );
        return 1;
    }
    fclose( pFile );
    if (current_TList != NULL)
        DeleteTList(current_TList);
    current_TList = func_readFileOAO(FileName);
    return 0;

usage:
    Abc_Print( -2, "usage: read_th [-h] <file>\n" );
    Abc_Print( -2, "\t         a reader for threshold gate '.th' files\n" );
    Abc_Print( -2, "\t-h     : print the command usage\n");
    Abc_Print( -2, "\t<file> : the file name\n");
    return 1;
}

int Abc_CommandWriteThreshold( Abc_Frame_t * pAbc,int argc, char ** argv )
{
    FILE * pFile;
    char ** pArgvNew;
    char * FileName, * pTemp;
    int nArgcNew;
    int c;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        goto usage;
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
    if (current_TList == NULL){
        printf("ERROR: current threshold gateList is empty!!\n\n");
        return 1;
    }

    dumpTh2FileNZ (current_TList, FileName);
    return 0;

usage:
    Abc_Print( -2, "usage: write_th [-h] <file>\n" );
    Abc_Print( -2, "\t         dump function  for threshold gate '.th' files\n" );
    Abc_Print( -2, "\t-h     : print the command usage\n");
    Abc_Print( -2, "\t<file> : the file name\n");
    return 1;
}
///////////////////////////////////////////////////////////
int Abc_CommandEC_Threshold( Abc_Frame_t * pAbc,int argc, char ** argv )
{
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk, * pNtkRes;
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
        goto usage;
    }

    if ( pNtk == NULL )
    {
        fprintf( pErr, "Empty network.\n" );
        return 1;
    }

    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }

    if ( !Abc_NtkIsStrash( pNtk ) )
    {
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
    if (current_TList == NULL)
    {
        fprintf(pErr, "ERROR: current thresholdList is empty!!\n\n");
        return 1;
    }
    func_EC_writePB(pNtk, current_TList);
    return 0;
usage:
    fprintf( pErr, "usage:  EC_th [-h]\n" );
    fprintf( pErr, "\t       given strashNtk and thresholdNtk, print out Pseudo Boolean constraints for minisat+ to solve EC.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}

int Abc_CommandCNF_Threshold( Abc_Frame_t * pAbc,int argc, char ** argv )
{    
    FILE * pOut, * pErr;
    Abc_Ntk_t * pNtk, * pNtkRes;
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
        goto usage;
    }

    if ( pNtk == NULL )
    {
        fprintf( pErr, "Empty network.\n" );
        return 1;
    }

    if ( !Abc_NtkIsComb( pNtk ) ) {
        fprintf(pErr, "\tnetwork is not comb, Make comb...\n");
		  Abc_NtkMakeComb( pNtk , fRemoveLatches ); 
	 }

    if ( !Abc_NtkIsStrash( pNtk ) )
    {
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
    if (current_TList == NULL)
    {
        fprintf(pErr, "ERROR: current thresholdList is empty!!\n\n");
        return 1;
    }
    func_EC_writeCNF(pNtk, current_TList);
    return 0;
usage:
    fprintf( pErr, "usage:  CNF_th [-h]\n" );
    fprintf( pErr, "\t       given strashNtk and thresholdNtk, print out CNF file for minisat to solve EC.\n");
    fprintf( pErr, "\t-h    : print the command usage\n");
    return 1;
}


