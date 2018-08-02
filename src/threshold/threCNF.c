#include <stdio.h>
#include <math.h>
#include "base/abc/abc.h"
#include "threshold.h"

extern void func_EC_writeCNF(Abc_Ntk_t*, Vec_Ptr_t*, char*);
void        my_Vec_StrPrintNum( Vec_Str_t *, int );
Vec_Ptr_t*  alan_CNF(FILE*, Abc_Ntk_t *);
Vec_Ptr_t*  thre_CNF(FILE*, Vec_Ptr_t *);
void        miter_CNF(FILE*, Vec_Ptr_t *, Vec_Ptr_t *);
///
Thre_S*     slow_sortByWeights(Thre_S*);
void        delete_sortedNode(Thre_S*);
void        Recurrsive_CNF(FILE*,Thre_S*, Vec_Str_t*, int, int);
int         Thre_LocalMax(Thre_S*, int);
int         Thre_LocalMin(Thre_S*, int);
///

void func_EC_writeCNF(Abc_Ntk_t * pNtk, Vec_Ptr_t* TList, char* fileName)
{
    //char* fileName = "EC.cnf";
    FILE* oFile = fopen(fileName, "w");
    printf("\tchecking Equalivance of ABC_ntk and threshold_ntk...\n");
    printf("\tOutputFile: %s\n", fileName);
    fprintf(oFile, "c CNF file for aig<->th equiv checking\n");
    
    Vec_Ptr_t* aigPO = alan_CNF( oFile, pNtk );
    Vec_Ptr_t* thPO  = thre_CNF( oFile, TList );
    miter_CNF( oFile, aigPO, thPO );

    Vec_PtrFree(aigPO);
    Vec_PtrFree(thPO);
    fclose(oFile);
}
/////////////////////////////////////////////////////

void miter_CNF(FILE* oFile, Vec_Ptr_t* aig, Vec_Ptr_t * th){
    // VAR naming:
    // aig_PO    : 3*id+1
    // th_PO     : 3*id
    // miter_XOR : 3*<iter>+2
    // OR        : 3*<PO_size> +2
    if(Vec_PtrSize(aig) != Vec_PtrSize(th)){
        printf("\tERROR: two network have different # of POs\n");
        printf("\tEC_check : stopped\n");
        return;
    }
    int i;
    Thre_S      *tObj;
    Abc_Obj_t   *aObj;
    Vec_Str_t * sAig = Vec_StrAlloc(5);
    Vec_Str_t * sTh  = Vec_StrAlloc(5);
    Vec_Str_t * sMit = Vec_StrAlloc(5);
    Vec_Str_t * sOR  = Vec_StrAlloc(5);
    
    Vec_PtrForEachEntry( Abc_Obj_t*, aig, aObj, i){
        tObj = (Thre_S*)Vec_PtrEntry( th, i );
        
        Vec_StrClear(sAig);
        my_Vec_StrPrintNum( sAig, Abc_ObjId( aObj )*3 +1 );
        Vec_StrClear(sTh);
        my_Vec_StrPrintNum( sTh, tObj->Id *3 );
        Vec_StrClear(sMit);
        my_Vec_StrPrintNum( sMit, i*3 + 2 );
        
        fprintf(oFile, "-%s  %s  %s 0\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, " %s -%s  %s 0\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, " %s  %s -%s 0\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, "-%s -%s -%s 0\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
    }
    int PO_size = Vec_PtrSize(aig);
    Vec_StrPrintStr(sOR, " 2 ");
    fprintf(oFile, "-2 %d 0\n" , PO_size*3 +2 );
    for( i = 1; i < PO_size; ++i ){
        Vec_StrPrintNum(sOR, i*3+2);
        Vec_StrAppend(sOR, " ");
        fprintf(oFile, "-%d %d 0\n", i*3+2, PO_size*3+2);
    }
    Vec_StrAppend(sOR, "-");
    Vec_StrPrintNum(sOR, PO_size*3 + 2);
    Vec_StrAppend(sOR, " 0\n");
    Vec_StrPush(sOR, '\0');
    fprintf(oFile,sOR->pArray);
    fprintf(oFile," %d 0\n", PO_size*3 +2);

    printf("\tdone\n");
    Vec_StrFree(sAig);
    Vec_StrFree(sTh);
    Vec_StrFree(sMit);
    Vec_StrFree(sOR);
    
}
/////////////////////////////

Vec_Ptr_t* thre_CNF(FILE* oFile, Vec_Ptr_t* TList )
{
    Vec_Ptr_t * thPOList = Vec_PtrAlloc(10);
    /*
     * VAR naming:
     * PI: 3*id
     * PO: 3*id 
     * TH: 3*id 
     * be care of CONST1 gates (only @ PO)
     *
     */
    int i, j, finId;
    Thre_S *tObj_unsort, *tObj, *finObj;
    Vec_Str_t * sCNF = Vec_StrAlloc(10);

    Vec_PtrForEachEntry( Thre_S*, TList, tObj_unsort, i ){
        if ( tObj_unsort == NULL ) continue;
        if ( tObj_unsort->Type == 1 || tObj_unsort->Type == 4)
            continue;
        if ( tObj_unsort->Type == 2 )
            Vec_PtrPush( thPOList, tObj_unsort );
        

        // construst a sorted version of this THnode
        tObj = slow_sortByWeights( tObj_unsort );
        Vec_StrClear(sCNF);
        //printf("--- ID = %d ---\n", tObj->Id);
        
        Recurrsive_CNF(oFile, tObj, sCNF, tObj->thre, 0);
        
        // delete tObj
        delete_sortedNode( tObj );
    }
    Vec_StrFree( sCNF );
    return thPOList;
}

void Recurrsive_CNF(FILE* oFile, Thre_S* t, Vec_Str_t* sCNF, int thre, int lvl)
{
    //if (lvl >= Vec_IntSize(t->Fanins))  return;
    assert(lvl < Vec_IntSize(t->Fanins));
   
    Vec_Str_t * pos_cof_CNF = Vec_StrDup( sCNF );
    Vec_StrAppend( pos_cof_CNF, "-" );
    // ID X 3
    Vec_StrPrintNum( pos_cof_CNF, 3 * Vec_IntEntry(t->Fanins, lvl));
    Vec_StrAppend( pos_cof_CNF, " ");

    Vec_Str_t * neg_cof_CNF = Vec_StrDup( sCNF );
    // ID X3
    Vec_StrPrintNum( neg_cof_CNF, 3 * Vec_IntEntry(t->Fanins, lvl));
    Vec_StrAppend( neg_cof_CNF, " ");
    
    
    int current_w = Vec_IntEntry(t->weights, lvl);
    int maxF  = Thre_LocalMax(t, lvl);
    int minF  = Thre_LocalMin(t, lvl);
    
    int pos_cof_thre = thre - current_w;
    if ( pos_cof_thre <= minF) {
        // on-set
        Vec_StrPush( pos_cof_CNF, '\0');
        fprintf(oFile,"%s%d 0\n", pos_cof_CNF->pArray, 3*t->Id);
    }
    else if ( maxF < pos_cof_thre ){
        // off-set
        Vec_StrPush( pos_cof_CNF, '\0');
        fprintf(oFile,"%s-%d 0\n", pos_cof_CNF->pArray, 3*t->Id);
    }
    else
        Recurrsive_CNF(oFile, t, pos_cof_CNF, pos_cof_thre, lvl+1);
    
    int neg_cof_thre = thre;
    if( neg_cof_thre <= minF){
        // on-set
        Vec_StrPush( neg_cof_CNF, '\0');
        fprintf(oFile,"%s%d 0\n", neg_cof_CNF->pArray, 3*t->Id);
    }
    else if ( maxF < neg_cof_thre ){
        // off-set
        Vec_StrPush( neg_cof_CNF, '\0');
        fprintf(oFile,"%s-%d 0\n", neg_cof_CNF->pArray, 3*t->Id);
    }
    else
        Recurrsive_CNF(oFile, t, neg_cof_CNF, neg_cof_thre, lvl+1);
    Vec_StrFree(pos_cof_CNF);
    Vec_StrFree(neg_cof_CNF);
    
} 


int Thre_LocalMax(Thre_S* t, int lvl)
{
    int max = 0;
    int i;
    int size = Vec_IntSize(t->weights);
    for( i = lvl+1 ; i < size; ++i )
        if(Vec_IntEntry( t->weights, i) > 0)
            max += Vec_IntEntry( t->weights, i );
    return max;
}

int Thre_LocalMin(Thre_S* t, int lvl)
{
    int min = 0;
    int i;
    int size = Vec_IntSize(t->weights);
    for( i = lvl+1 ; i < size; ++i )
        if(Vec_IntEntry( t->weights, i) < 0)
            min += Vec_IntEntry( t->weights, i );
    return min;
}

// NZ : including current level
int Th_LocalMax( Thre_S * t , int head , int tail )
{
    int max = 0;
    int i;
    //int size = Vec_IntSize(t->weights);
    for ( i = head ; i <= tail ; ++i )
        if ( Vec_IntEntry( t->weights , i ) > 0 )
            max += Vec_IntEntry( t->weights , i );
    return max;
}

int Th_LocalMin( Thre_S * t , int head , int tail )
{
    int min = 0;
    int i;
    //int size = Vec_IntSize(t->weights);
    for ( i = head ; i <= tail ; ++i )
        if ( Vec_IntEntry( t->weights , i) < 0 )
            min += Vec_IntEntry( t->weights , i );
    return min;
}
// NZ : including current level

Thre_S* slow_sortByWeights( Thre_S* tObj )
{
    Thre_S* newNode = ABC_ALLOC( Thre_S, 1 );
    newNode-> Fanouts = NULL;
    newNode-> Fanins  = Vec_IntDup(tObj->Fanins);
    newNode-> weights = Vec_IntDup(tObj->weights); 
    newNode-> Id      = tObj->Id;
    newNode-> Type    = tObj->Type;
    newNode-> thre    = tObj->thre;
    
    // newNode: bubble sort by weights
    int i, j;
    int size = Vec_IntSize(newNode->Fanins);
    for( i = 0; i < size -1; ++i) {
        for( j = i+1; j < size; ++j ){
            Vec_Int_t * w = newNode->weights;
            Vec_Int_t * f = newNode->Fanins;
            if(Vec_IntEntry(w, i) < Vec_IntEntry(w, j)){
                // swap ( w[i], w[j] ) and ( f[i], f[j] )
                int tmpW = Vec_IntEntry( w, i );
                Vec_IntWriteEntry(w, i, Vec_IntEntry(w, j));
                Vec_IntWriteEntry(w, j, tmpW);
                int tmpF = Vec_IntEntry( f, i );
                Vec_IntWriteEntry(f, i, Vec_IntEntry(f, j));
                Vec_IntWriteEntry(f, j, tmpF);
            }
        }
    }
    return newNode;
}

Thre_S* slow_sortByAbsWeights( Thre_S* tObj )
{
    Thre_S* newNode = ABC_ALLOC( Thre_S, 1 );
    newNode-> Fanouts = NULL;
    newNode-> Fanins  = Vec_IntDup(tObj->Fanins);
    newNode-> weights = Vec_IntDup(tObj->weights); 
    newNode-> Id      = tObj->Id;
    newNode-> Type    = tObj->Type;
    newNode-> thre    = tObj->thre;
    
    // newNode: bubble sort by weights
    int i, j;
    int size = Vec_IntSize(newNode->Fanins);
    for( i = 0; i < size -1; ++i) {
        for( j = i+1; j < size; ++j ){
            Vec_Int_t * w = newNode->weights;
            Vec_Int_t * f = newNode->Fanins;
            if( abs(Vec_IntEntry(w, i)) < abs(Vec_IntEntry(w, j)) ){
                // swap ( w[i], w[j] ) and ( f[i], f[j] )
                int tmpW = Vec_IntEntry( w, i );
                Vec_IntWriteEntry(w, i, Vec_IntEntry(w, j));
                Vec_IntWriteEntry(w, j, tmpW);
                int tmpF = Vec_IntEntry( f, i );
                Vec_IntWriteEntry(f, i, Vec_IntEntry(f, j));
                Vec_IntWriteEntry(f, j, tmpF);
            }
        }
    }
    return newNode;
}

void delete_sortedNode( Thre_S* tObj )
{
    Vec_IntFree( tObj->Fanins );
    Vec_IntFree( tObj->weights );
    ABC_FREE( tObj );
}

/////////////////////////////////////////////////////


void my_Vec_StrPrintNum( Vec_Str_t* s, int n)
{
    Vec_StrPrintNum( s, n );
    Vec_StrPush( s, '\0' );
}

Vec_Ptr_t* alan_CNF(FILE* oFile, Abc_Ntk_t * pNtk)
{
    /* VAR naming:
     * PI:  3*id
     * PO:  3*id +1
     * aig: 3*id +1
     * be care of const fanins (only @ PO)
     */

    Vec_Str_t * sNode = Vec_StrAlloc(5);
    Vec_Str_t * sFin0 = Vec_StrAlloc(5);
    Vec_Str_t * sFin1 = Vec_StrAlloc(5);
    Vec_Ptr_t * aigPOList = Vec_PtrAlloc(10);
    int i;
    Abc_Obj_t * pObj;
    Abc_NtkForEachObj( pNtk, pObj, i ){
        if( pObj->Type == ABC_OBJ_PI || pObj->Type == ABC_OBJ_CONST1 )
            continue;

        if( pObj->Type == ABC_OBJ_PO ){
            Vec_PtrPush( aigPOList, pObj);
            Abc_Obj_t * pFin0 = Abc_ObjFanin0(pObj);
            int NodeId = Abc_ObjId(pObj);
            int Fin0Id = Abc_ObjId(pFin0);
           
            Vec_StrClear( sNode );
            my_Vec_StrPrintNum( sNode, NodeId*3 +1);

            //TODO: consider CONST fanins
            Vec_StrClear( sFin0 );
            if ( pFin0->Type == ABC_OBJ_PI )
                my_Vec_StrPrintNum( sFin0, Fin0Id*3 );
            else
                my_Vec_StrPrintNum( sFin0, Fin0Id*3 +1);

            if(Abc_ObjFaninC0(pObj)){
            //    printf( "!%s -> %s\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile, "-%s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile, " %s  %s 0\n", sFin0->pArray, sNode->pArray);
            }
            else{
            //    printf( " %s -> %s\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile, " %s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile, "-%s  %s 0\n", sFin0->pArray, sNode->pArray);
            }

        }
        else{ 
            // pObj->Type == ABC_OBJ_NODE
            Abc_Obj_t * pFin0 = Abc_ObjFanin0(pObj);
            Abc_Obj_t * pFin1 = Abc_ObjFanin1(pObj);
            int NodeId = Abc_ObjId(pObj);
            int Fin0Id = Abc_ObjId(pFin0);
            int Fin1Id = Abc_ObjId(pFin1);
            
            Vec_StrClear( sNode );
            my_Vec_StrPrintNum( sNode, NodeId*3 +1);
            
            // TODO: consider CONST fanins
            //  need to check faninNode is aig or PI
            Vec_StrClear( sFin0 );
            if ( pFin0->Type == ABC_OBJ_PI )
                my_Vec_StrPrintNum( sFin0, Fin0Id*3 );
            else
                my_Vec_StrPrintNum( sFin0, Fin0Id*3 +1);
            
            Vec_StrClear( sFin1 );
            if ( pFin1->Type == ABC_OBJ_PI )
                my_Vec_StrPrintNum( sFin1, Fin1Id*3 );
            else
                my_Vec_StrPrintNum( sFin1, Fin1Id*3 +1);
            
            if( Abc_ObjFaninC0(pObj) == 0 && Abc_ObjFaninC1(pObj) == 0){
                //printf(" %s  &  %s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oFile," %s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile," %s -%s 0\n", sFin1->pArray, sNode->pArray);
                fprintf(oFile,"-%s -%s  %s 0\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);

            }
            else if( Abc_ObjFaninC0(pObj) == 1 && Abc_ObjFaninC1(pObj) == 0){
                //printf("!%s  &  %s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oFile,"-%s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile," %s -%s 0\n", sFin1->pArray, sNode->pArray);
                fprintf(oFile," %s -%s  %s 0\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
            else if( Abc_ObjFaninC0(pObj) == 0 && Abc_ObjFaninC1(pObj) == 1){
                //printf(" %s  & !%s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oFile," %s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile,"-%s -%s 0\n", sFin1->pArray, sNode->pArray);
                fprintf(oFile,"-%s  %s  %s 0\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
            else {
                //printf("!%s  & !%s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oFile,"-%s -%s 0\n", sFin0->pArray, sNode->pArray);
                fprintf(oFile,"-%s -%s 0\n", sFin1->pArray, sNode->pArray);
                fprintf(oFile," %s  %s  %s 0\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
        }
    }
    // Destructor
    Vec_StrFree( sNode );
    Vec_StrFree( sFin0 );
    Vec_StrFree( sFin1 );
    return aigPOList;
}
