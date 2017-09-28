#include "base/abc/abc.h"
#include <stdio.h>
#include "threshold.h"

void    DeleteTList     (Vec_Ptr_t *);
Thre_S* newThre         (int , char*);
int     getIdByName     (Vec_Ptr_t* , char* );
Thre_S* getPtrByName    (Vec_Ptr_t* , char* );
void    printNodeStats  (Vec_Ptr_t* );
void    dumpTh2FileNZ   ( Vec_Ptr_t * , char * );

/**************************
 * Function: Delete tgList*
 *************************/
void DeleteTList(Vec_Ptr_t * tList)
{
    int i;
    Thre_S * tObj;
    Vec_PtrForEachEntry( Thre_S * , tList , tObj , i ) {
       //printf( " > Delete %d-th object , ptr = %p\n" , i  , tObj );
       //fflush( stdout );
       if ( tObj != NULL ) {
          if ( tObj->pName ) ABC_FREE( tObj->pName );
          Vec_IntFree( tObj->weights );
          Vec_IntFree( tObj->Fanins  );
          Vec_IntFree( tObj->Fanouts );
          ABC_FREE( tObj );
       }
    }
    //printf( "tList = %p\n" , tList );
    Vec_PtrFree( tList );
}

/**********************
 * Function: Read File*
 *********************/


Vec_Ptr_t* func_readFileOAO(char* fileName)
{
    FILE *inFile;
    inFile = fopen(fileName, "r");
    if (inFile == NULL){
        printf("read failed\n");
        return NULL;
    }
    Vec_Ptr_t* vTG;    // total TG container
    Vec_Int_t* iCount; // counting Fanins
    iCount = Vec_IntAlloc( 10 );   
    
    char buf[1000], buf2[1000];
    int piCount = 0;
    int poCount = 0;
    int tgCount = 0;
    Vec_Str_t*  moduleName = Vec_StrAlloc(10);

    /*** parse gate counts ***/
    // first line: ignore
    fgets( buf, 1000, inFile);
    // get modelName
    fscanf( inFile, "%s%s", buf, buf2);
    if ( !strncmp(buf, ".model", 6) )
        Vec_StrPushBuffer( moduleName, buf2, 1000 );
    printf("\tmoduleName:%s\n", moduleName->pArray);
    // PI count    
    fscanf( inFile , "%s", buf );
    if (strncmp(buf, ".inputs", 6)){
        printf("no INPUT detected\n");
        return NULL;
    }
    // PO count
    while ( fscanf(inFile, "%s", buf) ){
        if( !strncmp(buf, ".outputs", 6 ))
            break;
        piCount++;
    }
    // thresholdGate count
    while ( fscanf(inFile, "%s", buf) ){
        if( !strncmp(buf, ".threshold", 10) ){
            tgCount++;
            break;
        }
       poCount++;
    }
    int counter = 0;
    while (1){
        if(fscanf(inFile,"%s", buf) == EOF){
            Vec_IntPush( iCount, counter/2 );
            break;
        }
        if( !strncmp(buf, ".threshold", 10) ){
            tgCount++;
            Vec_IntPush( iCount, counter/2 );
            counter = 0;
        }
        else counter++;
    }
    printf("\tPIcount:%d\n\tPOcount:%d\n\tTGcount:%d\n", piCount, poCount, tgCount);

    // reRead inFile
    rewind(inFile);
    // initialize  containers
    vTG = Vec_PtrAlloc( tgCount + piCount );
    fgets( buf, 1000, inFile ); // Threshold logic gates synthesis
    fgets( buf, 1000, inFile ); // .model <modelName>

    int i, j, intBuf;
    int idCount = 0;

    // Const1 gate, id = 0, type = 4 
    Thre_S* newGate = newThre( idCount++, "CONST1" );
    newGate->Type = 4;
    Vec_PtrPush( vTG, newGate );

    fscanf( inFile, "%s", buf); // .inputs
    for( i = 0; i < piCount; ++i){
        fscanf( inFile, "%s", buf);
        Thre_S* newGate = newThre( idCount++, buf );
        newGate->Type = 1;
        Vec_PtrPush( vTG, newGate );
    }
    
    fscanf( inFile, "%s", buf); // .outputs
    for( i = 0; i < poCount; ++i){
        fscanf( inFile, "%s", buf);
        Thre_S* newGate = newThre( idCount++, buf );
        newGate->Type = 2;
        Vec_PtrPush( vTG, newGate );
    }
    fgets( buf, 1000, inFile ); // read '\n' at the bottom of the line
    for( i = 0; i < tgCount; ++i ){
        if ( i < poCount ){
            fgets( buf, 1000, inFile );// ignore 2 lines
            fgets( buf, 1000, inFile );
        }
        else{
            for( j = 0; j < Vec_IntEntry( iCount, i );++j ){
                fscanf( inFile, "%s", buf);
            }
            fscanf( inFile, "%s", buf2);
            Thre_S* newGate = newThre( idCount++, buf2);
            newGate->Type = 3;
            Vec_PtrPush( vTG, newGate );
            fgets( buf, 1000, inFile ); // read '\n' at the bottom of the line
            fgets( buf, 1000, inFile ); // ignore 1 line
        }
    }
    // reRead inFile
    rewind(inFile);
    fgets( buf, 1000, inFile ); // Threshold logic gates synthesis
    fgets( buf, 1000, inFile ); // .model <modelName>
    for ( i = 0; i < piCount + poCount +2; ++i)
        fscanf( inFile, "%s", buf );
    fgets( buf, 1000, inFile ); // read '\n' at the bottom of the line
   
    for (i = 0; i < tgCount; ++i){
                                                // + 1 for constGate
        Thre_S* thisGate = (Thre_S*)Vec_PtrEntry(vTG, piCount + i + 1);
        // connect fanins fanouts
        fscanf( inFile, "%s", buf ); //.threshold
        for( j = 0; j < Vec_IntEntry(iCount,i) -1; ++j ){
            fscanf( inFile, "%s", buf );
            int finId = getIdByName( vTG, buf );
            Thre_S* finGate = getPtrByName( vTG, buf );
            Vec_IntPush( thisGate->Fanins, finId );
            Vec_IntPush( finGate->Fanouts, thisGate->Id );
        }
        // parse weight and threshold
        fscanf( inFile, "%s", buf); // <thisGate->pName>
        for( j = 0; j < Vec_IntEntry(iCount,i) -1; ++j){
            fscanf( inFile, "%d", &intBuf );
            Vec_IntPush( thisGate->weights, intBuf );
        }
        fscanf( inFile, "%d", &intBuf );
        thisGate->thre = intBuf;
    }
   // check stats
   // printNodeStats(vTG);
    Vec_IntFree( iCount );
    Vec_StrFree( moduleName );
    return vTG;
}


Thre_S* newThre(int id, char* name){
    Thre_S* newGate = ABC_ALLOC( Thre_S, 1);
    newGate->pName  = ABC_ALLOC( char, strlen(name)+1 );
    newGate->weights = Vec_IntAlloc( 10 );
    newGate->Fanins  = Vec_IntAlloc( 10 );
    newGate->Fanouts = Vec_IntAlloc( 10 );
    newGate-> Id = id;
    strcpy( newGate->pName, name );
    newGate->Type = 0;
    newGate->Ischoose = 0;
    newGate->oId   = 0;
    newGate->nId   = 0;
    newGate->cost  = 0;
    newGate->level = 0;
    return newGate;
}
int     getIdByName(Vec_Ptr_t* v, char* name)
{
    int i;
    Thre_S* thres;
    Vec_PtrForEachEntry(Thre_S*, v, thres, i){
        if( strcmp(thres->pName, name) == 0)
            return thres->Id;
    }
    return -1;
}
Thre_S* getPtrByName(Vec_Ptr_t* v, char* name){
    int i;
    Thre_S* thres;
    Vec_PtrForEachEntry(Thre_S*, v, thres, i){
        if( strcmp(thres->pName, name) == 0)
            return thres;
    }
    return NULL;
}
void  printNodeStats(Vec_Ptr_t* vTG){ 
    int i;
    Thre_S* thres;
    Vec_PtrForEachEntry(Thre_S*, vTG, thres,i){
        printf(" #%d: ID=%d, name=%s, type=%d\n",
               i, thres->Id, thres->pName, thres->Type);
    }
}


/***********************
 * Function: Write File*
 ***********************/

void dumpTh2FileNZ( Vec_Ptr_t * thre_list , char * name )
{   
    FILE * pFile;
    char filename[30];
    Thre_S * tObj;
    Vec_Ptr_t * vPi , * vPo , * vTh; 
    int i;
    int j = 0;
   
    while (1) {
        filename[j] = name[j];
        if ( name[j] == '\0' ) break;
        else ++j;
    }

    pFile = fopen( name , "w" );
    vPi   = Vec_PtrAlloc(16); 
    vPo   = Vec_PtrAlloc(16); 
    vTh   = Vec_PtrAlloc(16); 

    for ( i = 0 ; i < Vec_PtrSize( thre_list ) ; ++i ) {
        tObj = Vec_PtrEntry( thre_list , i );
        if ( tObj == NULL ) continue;
        if ( tObj-> Type == Th_Pi ) Vec_PtrPush( vPi , tObj );
        else if ( tObj-> Type == Th_Po ) Vec_PtrPush( vPo , tObj );
        else if ( tObj-> Type == Th_Node ) Vec_PtrPush( vTh , tObj );
    }

    // write header
    fprintf( pFile , "Threshold logic gate list written by NZ.\n" );
    // write model
    fprintf( pFile , ".model %s\n" , name );
    // write input
    fprintf( pFile , ".input");
    for ( i = 0 ; i < Vec_PtrSize( vPi ) ; ++i ) {
        tObj = Vec_PtrEntry( vPi , i);
        fprintf( pFile , " %d" , tObj->Id );
    }
    fprintf( pFile , "\n");
    // write output
    fprintf( pFile , ".output");
    for ( i = 0 ; i < Vec_PtrSize( vPo ) ; ++i ) {
        tObj = Vec_PtrEntry( vPo , i);
        fprintf( pFile , " %d" , tObj->Id );
    }
    fprintf( pFile , "\n");
    // write threshold gate
    for ( i = 0 ; i < Vec_PtrSize( vPo ) ; ++i ) {
        tObj = Vec_PtrEntry( vPo , i );
		  if ( Vec_IntEntry( tObj->Fanins , 0 ) == 0 ) fprintf( pFile , ".threshold %s %d\n" , "CONST1" , tObj->Id );
		  else fprintf( pFile , ".threshold %d %d\n" , Vec_IntEntry( tObj->Fanins , 0 ) , tObj->Id );
        fprintf( pFile , "%d %d\n" , Vec_IntEntry( tObj->weights , 0 ) , tObj->thre );
    }
    for ( i = 0 ; i < Vec_PtrSize( vTh ) ; ++i ) {
        tObj = Vec_PtrEntry( vTh , i );
        fprintf( pFile , ".threshold" );
        for ( j = 0 ; j < Vec_IntSize( tObj->Fanins ) ; ++j ) {
           fprintf( pFile , " %d" , Vec_IntEntry( tObj->Fanins , j ) );
        }
        fprintf( pFile , " %d\n" , tObj->Id );
        for ( j = 0 ; j < Vec_IntSize( tObj->weights ) ; ++j ) {
           fprintf( pFile , "%d " , Vec_IntEntry( tObj->weights , j ) );
        }
        fprintf( pFile , "%d\n" , tObj->thre );
    }

	 Vec_PtrFree(vPi);
	 Vec_PtrFree(vPo);
	 Vec_PtrFree(vTh);

	 fclose(pFile);
}


