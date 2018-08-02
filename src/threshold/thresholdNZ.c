#include <stdio.h>
#include <math.h>
#include "base/abc/abc.h"
#include "map/if/if.h"
#include "map/if/ifCount.h"
#include "threshold.h"
#include "bdd/extrab/extraBdd.h"
#include "misc/extra/extra.h"
#include "misc/vec/vecPtr.h"

/*************************************************************
Compare two threNtk, PB
 *************************************************************/


void       func_EC_compareTH( Vec_Ptr_t*, Vec_Ptr_t*);
Vec_Ptr_t* thre1_PB( FILE*, Vec_Ptr_t *);
Vec_Ptr_t* thre2_PB( FILE*, Vec_Ptr_t *);
void       comp_miter_PB(FILE*, Vec_Ptr_t*, Vec_Ptr_t*);
///////////////////////////////

void func_EC_compareTH( Vec_Ptr_t * tList_1, Vec_Ptr_t * tList_2 )
{
    char* fileName = "compTH.opb";
    FILE* oFile = fopen(fileName, "w");
    printf("\tchecking Equalivance of cut_TList and current_TList...\n");
    printf("\tOutputFile: %s\n", fileName);
    fprintf(oFile, "min: -1*Z;\n");
    Vec_Ptr_t* thPO_1  = thre1_PB(oFile, tList_1);
    Vec_Ptr_t* thPO_2  = thre2_PB(oFile, tList_2);
    comp_miter_PB(oFile, thPO_1, thPO_2);
    
    Vec_PtrFree(thPO_1);
    Vec_PtrFree(thPO_2);
    fclose(oFile);
}
//////////////////////////////////////////////////////////////////
void comp_miter_PB(FILE* oFile, Vec_Ptr_t *th1, Vec_Ptr_t * th2){
    // VAR naming:
    // th1_PO  : O1_<id>
    // th2_PO  : O2_<id>
    // miterXOR: M_<iter>
    // OR      : Z
    if(Vec_PtrSize(th1) != Vec_PtrSize(th2)){
        printf("\tERROR: two network have different # of POs\n");
        printf("\tEC_check : stopped\n");
        return;
    }
    int i;
    Thre_S*    t1Obj;
    Thre_S*    t2Obj;
    Vec_Str_t * sTh1 = Vec_StrAlloc(5);
    Vec_Str_t * sTh2 = Vec_StrAlloc(5);
    Vec_Str_t * sMit = Vec_StrAlloc(5);
    Vec_Str_t * sOR  = Vec_StrAlloc(5);
    
    Vec_PtrForEachEntry( Thre_S*, th1, t1Obj, i){
        t2Obj = (Thre_S *)Vec_PtrEntry( th2, i);
        
        Vec_StrClear(sTh1);
        Vec_StrPrintStr(sTh1, "O1_");
        Vec_StrPrintNum(sTh1, t1Obj->Id);
        Vec_StrPush(sTh1, '\0');
        Vec_StrClear(sTh2);
        Vec_StrPrintStr(sTh2, "O2_");
        Vec_StrPrintNum(sTh2, t2Obj->Id);
        Vec_StrPush(sTh2, '\0');
        Vec_StrClear(sMit);
        Vec_StrPrintStr(sMit, "M_");
        Vec_StrPrintNum(sMit, i);
        Vec_StrPush(sMit, '\0');

        fprintf(oFile, "+1*%s -1*%s +1*%s >= 0;\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s +1*%s -1*%s >= 0;\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s -1*%s -1*%s <= 0;\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s +1*%s +1*%s <= 2;\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
    }
    Vec_StrPrintStr(sOR, "+1*M_0");
    fprintf(oFile, "-1*M_0 +1*Z >= 0;\n");
    for( i = 1; i < Vec_PtrSize(th1); i++){
        Vec_StrAppend(sOR, " +1*M_");
        Vec_StrPrintNum(sOR, i);
        fprintf(oFile, "-1*M_%d +1*Z >= 0;\n", i);
    }
    Vec_StrAppend(sOR, " -1*Z >= 0;\n");
    Vec_StrPush(sOR, '\0');
    fprintf(oFile, sOR->pArray);
    
    printf("\tdone\n");
    Vec_StrFree(sTh1);
    Vec_StrFree(sTh2);
    Vec_StrFree(sMit);
    Vec_StrFree(sOR);
}

//////////////////////////////////////////////////////////////////

Vec_Ptr_t* thre1_PB(FILE* oFile, Vec_Ptr_t *thList){
    Vec_Ptr_t * thPOList = Vec_PtrAlloc(10);
    /* Reminder:
     * Con1: (M-T+1)y - sigma_N ( wixi ) >= 1-T
     * Con2: (m-T)  y + sigma_N ( wixi ) >= m
     * VAR naming:
     * const: CONST1 
     * PI: I_<id>  
     * PO: O1_<id>
     * TH: t1_<id>
     */
    int i, j, finId;
    Thre_S *tObj, *finObj;
    // node names
    Vec_Str_t* sNode = Vec_StrAlloc(5);
    Vec_Str_t* sFin  = Vec_StrAlloc(5); 
    // constraint string
    Vec_Str_t* sCon1 = Vec_StrAlloc(5);
    Vec_Str_t* sCon2 = Vec_StrAlloc(5);
    Vec_PtrForEachEntry( Thre_S*, thList, tObj, i){
        if( tObj == NULL ) continue;
        if( tObj->Type == 1 || tObj->Type == 4 )  continue;
        
        int NodeId = tObj->Id;
        Vec_StrClear( sNode );
        if( tObj->Type == 2 ){
            Vec_StrPrintStr( sNode, "O1_");
            Vec_PtrPush( thPOList, tObj);
        }
        else{
            Vec_StrPrintStr( sNode, "t1_");
        }
        Vec_StrPrintNum( sNode, NodeId );
        Vec_StrPush( sNode, '\0' );
        

        int MaxF = Max_Thre(tObj);
        int minF = min_Thre(tObj);
        int T    = tObj->thre;
        // c1: (M-T+1)y
        // c2: (m-T) y
        Vec_StrClear( sCon1 );
        Vec_StrClear( sCon2 );
        if (MaxF - T + 1 > 0)
            Vec_StrAppend(sCon1, "+");
        Vec_StrPrintNum( sCon1, MaxF - T + 1);
        Vec_StrAppend( sCon1, "*");
        Vec_StrAppend( sCon1, sNode->pArray );
        
        if (minF - T > 0)
            Vec_StrAppend(sCon2, "+");
        Vec_StrPrintNum( sCon2, minF - T );
        Vec_StrAppend( sCon2, "*");
        Vec_StrAppend( sCon2, sNode->pArray);
        
        Vec_IntForEachEntry( tObj->Fanins, finId,j ){
            finObj = (Thre_S *) Vec_PtrEntry( thList, finId );
            int w  = Vec_IntEntry( tObj->weights, j );
            Vec_StrClear( sFin );
            if( finObj->Type == 1){
                Vec_StrPrintStr(sFin, "I_");
                Vec_StrPrintNum(sFin, finId);
            }
            else if ( finObj->Type == 3 ){
                Vec_StrPrintStr(sFin, "t1_");
                Vec_StrPrintNum(sFin, finId);
            }
            else{
                // const gate
                Vec_StrPrintStr(sFin, "CONST1");
            }
            Vec_StrPush(sFin, '\0');
            // c1: -wixi
            // c2: wixi
            Vec_StrAppend(sCon1, " ");
            if (-w > 0)
                Vec_StrAppend(sCon1, "+");
            Vec_StrPrintNum(sCon1, -w);
            Vec_StrAppend(sCon1, "*");
            Vec_StrAppend(sCon1, sFin->pArray);
            
            Vec_StrAppend(sCon2, " ");
            if (w > 0)
                Vec_StrAppend(sCon2, "+");
            Vec_StrPrintNum(sCon2, w);
            Vec_StrAppend(sCon2, "*");
            Vec_StrAppend(sCon2, sFin->pArray);
        }
        // c1: >= 1-T
        // c2: >= m
        Vec_StrAppend(sCon1, " >= ");
        Vec_StrPrintNum(sCon1, 1-T);
        Vec_StrAppend(sCon1, ";\n");
        Vec_StrPush(sCon1, '\0');
        
        Vec_StrAppend(sCon2, " >= ");
        Vec_StrPrintNum(sCon2, minF);
        Vec_StrAppend(sCon2, ";\n");
        Vec_StrPush(sCon2, '\0');

        fprintf(oFile, "%s%s",sCon1->pArray ,sCon2->pArray);
    }
    Vec_StrFree( sNode );
    Vec_StrFree( sFin  );
    Vec_StrFree( sCon1 );
    Vec_StrFree( sCon2 );
    return thPOList;
}

Vec_Ptr_t* thre2_PB(FILE* oFile, Vec_Ptr_t *thList){
    Vec_Ptr_t * thPOList = Vec_PtrAlloc(10);
    /* Reminder:
     * Con1: (M-T+1)y - sigma_N ( wixi ) >= 1-T
     * Con2: (m-T)  y + sigma_N ( wixi ) >= m
     * VAR naming:
     * const: CONST1 
     * PI: I_<id>  
     * PO: O2_<id>
     * TH: t2_<id>
     */
    int i, j, finId;
    Thre_S *tObj, *finObj;
    // node names
    Vec_Str_t* sNode = Vec_StrAlloc(5);
    Vec_Str_t* sFin  = Vec_StrAlloc(5); 
    // constraint string
    Vec_Str_t* sCon1 = Vec_StrAlloc(5);
    Vec_Str_t* sCon2 = Vec_StrAlloc(5);
    Vec_PtrForEachEntry( Thre_S*, thList, tObj, i){
        if( tObj == NULL ) continue;
        if( tObj->Type == 1 || tObj->Type == 4 )  continue;
        
        int NodeId = tObj->Id;
        Vec_StrClear( sNode );
        if( tObj->Type == 2 ){
            Vec_StrPrintStr( sNode, "O2_");
            Vec_PtrPush( thPOList, tObj);
        }
        else{
            Vec_StrPrintStr( sNode, "t2_");
        }
        Vec_StrPrintNum( sNode, NodeId );
        Vec_StrPush( sNode, '\0' );
        

        int MaxF = Max_Thre(tObj);
        int minF = min_Thre(tObj);
        int T    = tObj->thre;
        // c1: (M-T+1)y
        // c2: (m-T) y
        Vec_StrClear( sCon1 );
        Vec_StrClear( sCon2 );
        if (MaxF - T + 1 > 0)
            Vec_StrAppend(sCon1, "+");
        Vec_StrPrintNum( sCon1, MaxF - T + 1);
        Vec_StrAppend( sCon1, "*");
        Vec_StrAppend( sCon1, sNode->pArray );
        
        if (minF - T > 0)
            Vec_StrAppend(sCon2, "+");
        Vec_StrPrintNum( sCon2, minF - T );
        Vec_StrAppend( sCon2, "*");
        Vec_StrAppend( sCon2, sNode->pArray);
        
        Vec_IntForEachEntry( tObj->Fanins, finId,j ){
            finObj = (Thre_S *) Vec_PtrEntry( thList, finId );
            int w  = Vec_IntEntry( tObj->weights, j );
            Vec_StrClear( sFin );
            if( finObj->Type == 1){
                Vec_StrPrintStr(sFin, "I_");
                Vec_StrPrintNum(sFin, finId);
            }
            else if ( finObj->Type == 3 ){
                Vec_StrPrintStr(sFin, "t2_");
                Vec_StrPrintNum(sFin, finId);
            }
            else{
                // const gate
                Vec_StrPrintStr(sFin, "CONST1");
            }
            Vec_StrPush(sFin, '\0');
            // c1: -wixi
            // c2: wixi
            Vec_StrAppend(sCon1, " ");
            if (-w > 0)
                Vec_StrAppend(sCon1, "+");
            Vec_StrPrintNum(sCon1, -w);
            Vec_StrAppend(sCon1, "*");
            Vec_StrAppend(sCon1, sFin->pArray);
            
            Vec_StrAppend(sCon2, " ");
            if (w > 0)
                Vec_StrAppend(sCon2, "+");
            Vec_StrPrintNum(sCon2, w);
            Vec_StrAppend(sCon2, "*");
            Vec_StrAppend(sCon2, sFin->pArray);
        }
        // c1: >= 1-T
        // c2: >= m
        Vec_StrAppend(sCon1, " >= ");
        Vec_StrPrintNum(sCon1, 1-T);
        Vec_StrAppend(sCon1, ";\n");
        Vec_StrPush(sCon1, '\0');
        
        Vec_StrAppend(sCon2, " >= ");
        Vec_StrPrintNum(sCon2, minF);
        Vec_StrAppend(sCon2, ";\n");
        Vec_StrPush(sCon2, '\0');

        fprintf(oFile, "%s%s",sCon1->pArray ,sCon2->pArray);
    }
    Vec_StrFree( sNode );
    Vec_StrFree( sFin  );
    Vec_StrFree( sCon1 );
    Vec_StrFree( sCon2 );
    return thPOList;
}

/*************************************************************
Compare two threNtk, CNF
*************************************************************/

void       func_CNF_compareTH(Vec_Ptr_t*, Vec_Ptr_t*);
Vec_Ptr_t* thre1_CNF( FILE*, Vec_Ptr_t *);
Vec_Ptr_t* thre2_CNF( FILE*, Vec_Ptr_t *);
void       comp_miter_CNF(FILE*, Vec_Ptr_t*, Vec_Ptr_t*);
/////
void       Recurrsive_TH1(FILE*, Thre_S*, Vec_Str_t*, int, int);
void       Recurrsive_TH2(FILE*, Vec_Ptr_t*, Thre_S*, Vec_Str_t*, int, int);
/////

///////////////////////////////////////////////////////

void func_CNF_compareTH( Vec_Ptr_t * tList_1, Vec_Ptr_t * tList_2 )
{
    char* fileName = "compTH.dimacs";
    FILE* oFile = fopen(fileName, "w");
    printf("\tchecking Equalivance of cut_TList and current_TList...\n");
    printf("\tOutputFile: %s\n", fileName);
    fprintf(oFile, "c CNF file for th<->th equiv checking\n");
    Vec_Ptr_t* thPO_1  = thre1_CNF(oFile, tList_1);
    Vec_Ptr_t* thPO_2  = thre2_CNF(oFile, tList_2);
    comp_miter_CNF(oFile, thPO_1, thPO_2);
    
    Vec_PtrFree(thPO_1);
    Vec_PtrFree(thPO_2);
    fclose(oFile);
}
///////////////////////////////////////////////////////////
void comp_miter_CNF(FILE* oFile, Vec_Ptr_t* th1, Vec_Ptr_t * th2){
    // VAR naming:
    // th1_PO    : 3*id
    // th2_PO    : 3*id+1
    // miter_XOR : 3*<iter>+2
    // OR        : 3*<PO_size> +2
    if(Vec_PtrSize(th1) != Vec_PtrSize(th2)){
        printf("\tERROR: two network have different # of POs\n");
        printf("\tEC_check : stopped\n");
        return;
    }
    int i;
    Thre_S      *t1Obj;
    Thre_S      *t2Obj;
    Vec_Str_t * sTh1 = Vec_StrAlloc(5);
    Vec_Str_t * sTh2 = Vec_StrAlloc(5);
    Vec_Str_t * sMit = Vec_StrAlloc(5);
    Vec_Str_t * sOR  = Vec_StrAlloc(5);
    
    Vec_PtrForEachEntry( Thre_S*, th1, t1Obj, i){
        t2Obj = (Thre_S*)Vec_PtrEntry( th2, i );
        
        Vec_StrClear(sTh1);
        my_Vec_StrPrintNum( sTh1, t1Obj->Id *3 );
        Vec_StrClear(sTh2);
        my_Vec_StrPrintNum( sTh2, t2Obj->Id *3+1 );
        Vec_StrClear(sMit);
        my_Vec_StrPrintNum( sMit, i*3 + 2 );
        
        fprintf(oFile, "-%s  %s  %s 0\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, " %s -%s  %s 0\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, " %s  %s -%s 0\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
        fprintf(oFile, "-%s -%s -%s 0\n", 
               sTh1->pArray, sTh2->pArray, sMit->pArray);
    }
    int PO_size = Vec_PtrSize(th1);
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
    Vec_StrFree(sTh1);
    Vec_StrFree(sTh2);
    Vec_StrFree(sMit);
    Vec_StrFree(sOR);
    
}
///////////////////////////////////////////////////////////
Vec_Ptr_t* thre1_CNF(FILE* oFile, Vec_Ptr_t* TList )
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
        //tObj = slow_sortByWeights( tObj_unsort );
        tObj = slow_sortByAbsWeights( tObj_unsort );
        Vec_StrClear(sCNF);
        //printf("--- ID = %d ---\n", tObj->Id);
        
        Recurrsive_TH1(oFile, tObj, sCNF, tObj->thre, 0);
        
        // delete tObj
        delete_sortedNode( tObj );
    }
    Vec_StrFree( sCNF );
    return thPOList;
}

void Recurrsive_TH1(FILE* oFile, Thre_S* t, Vec_Str_t* sCNF, int thre, int lvl)
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
        Recurrsive_TH1(oFile, t, pos_cof_CNF, pos_cof_thre, lvl+1);
    
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
        Recurrsive_TH1(oFile, t, neg_cof_CNF, neg_cof_thre, lvl+1);
    Vec_StrFree(pos_cof_CNF);
    Vec_StrFree(neg_cof_CNF);
    
} 

Vec_Ptr_t* thre2_CNF(FILE* oFile, Vec_Ptr_t* TList )
{
    Vec_Ptr_t * thPOList = Vec_PtrAlloc(10);
    /*
     * VAR naming:
     * PI: 3*id
     * PO: 3*id +1
     * TH: 3*id +1
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
        //tObj = slow_sortByWeights( tObj_unsort );
        tObj = slow_sortByAbsWeights( tObj_unsort );
        Vec_StrClear(sCNF);
        //printf("--- ID = %d ---\n", tObj->Id);
        
        Recurrsive_TH2(oFile, TList, tObj, sCNF, tObj->thre, 0);
        
        // delete tObj
        delete_sortedNode( tObj );
    }
    Vec_StrFree( sCNF );
    return thPOList;
}

void Recurrsive_TH2(FILE* oFile, Vec_Ptr_t* tList, Thre_S* t, Vec_Str_t* sCNF, int thre, int lvl)
{
    //if (lvl >= Vec_IntSize(t->Fanins))  return;
    assert(lvl < Vec_IntSize(t->Fanins));
   
    Vec_Str_t * pos_cof_CNF = Vec_StrDup( sCNF );
    Vec_Str_t * neg_cof_CNF = Vec_StrDup( sCNF );
    Vec_StrAppend( pos_cof_CNF, "-" );
    // <others = (id * 3) +1> <PI = 3*id>
    int finId = Vec_IntEntry(t->Fanins, lvl);
    if ( ((Thre_S*)Vec_PtrEntry(tList,finId))->Type == 1){
        Vec_StrPrintNum( pos_cof_CNF, 3 * finId);
        Vec_StrPrintNum( neg_cof_CNF, 3 * finId);
    }
    else{
        Vec_StrPrintNum( pos_cof_CNF, 3 * finId+1);
        Vec_StrPrintNum( neg_cof_CNF, 3 * finId+1);
    }
    Vec_StrAppend( pos_cof_CNF, " ");
    Vec_StrAppend( neg_cof_CNF, " ");
    
    int current_w = Vec_IntEntry(t->weights, lvl);
    int maxF  = Thre_LocalMax(t, lvl);
    int minF  = Thre_LocalMin(t, lvl);
    
    int pos_cof_thre = thre - current_w;
    if ( pos_cof_thre <= minF) {
        // on-set
        Vec_StrPush( pos_cof_CNF, '\0');
        fprintf(oFile,"%s%d 0\n", pos_cof_CNF->pArray, 3*t->Id+1);
    }
    else if ( maxF < pos_cof_thre ){
        // off-set
        Vec_StrPush( pos_cof_CNF, '\0');
        fprintf(oFile,"%s-%d 0\n", pos_cof_CNF->pArray, 3*t->Id+1);
    }
    else
        Recurrsive_TH2(oFile, tList, t, pos_cof_CNF, pos_cof_thre, lvl+1);
    
    int neg_cof_thre = thre;
    if( neg_cof_thre <= minF){
        // on-set
        Vec_StrPush( neg_cof_CNF, '\0');
        fprintf(oFile,"%s%d 0\n", neg_cof_CNF->pArray, 3*t->Id+1);
    }
    else if ( maxF < neg_cof_thre ){
        // off-set
        Vec_StrPush( neg_cof_CNF, '\0');
        fprintf(oFile,"%s-%d 0\n", neg_cof_CNF->pArray, 3*t->Id+1);
    }
    else
        Recurrsive_TH2(oFile, tList, t, neg_cof_CNF, neg_cof_thre, lvl+1);
    Vec_StrFree(pos_cof_CNF);
    Vec_StrFree(neg_cof_CNF);
    
} 


