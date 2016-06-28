#include "base/abc/abc.h"
#include <stdio.h>
#include "threshold.h"
/*********************************************
 * Function: Equiv-Checking: Using minisat+  *
 *************************** *****************/
void        func_EC_writePB(Abc_Ntk_t *, Vec_Ptr_t *, char*);
Vec_Ptr_t*  thre_PB(FILE*, Vec_Ptr_t *);
Vec_Ptr_t*  alan_PB(FILE*, Abc_Ntk_t *);
void        miter_PB(FILE*, Vec_Ptr_t*, Vec_Ptr_t *);
int         Max_Thre(Thre_S *);
int         min_Thre(Thre_S *);

//////////////////////////////////////////////////////////////////
void func_EC_writePB(Abc_Ntk_t *pNtk, Vec_Ptr_t *thList, char *fileName)
{
    FILE* oFile = fopen(fileName, "w");
    printf("\tchecking Equalivance of ABC_ntk and threshold_ntk...\n");
    printf("\tOutputFile: %s\n", fileName);
    fprintf(oFile, "min: -1*Z;\n");
    Vec_Ptr_t* aigPO = alan_PB(oFile, pNtk);
    Vec_Ptr_t* thPO  = thre_PB(oFile, thList);
    miter_PB(oFile, aigPO, thPO);
    
    Vec_PtrFree(aigPO);
    Vec_PtrFree(thPO);
    fclose(oFile);
}
/////////////////////////////////////////////////////////////////
void miter_PB(FILE* oFile, Vec_Ptr_t *aig, Vec_Ptr_t * th){
    // VAR naming:
    // aig_PO  : PO_<id>
    // th_PO   : TO_<id>
    // miterXOR: M_<iter>
    // OR      : Z
    if(Vec_PtrSize(aig) != Vec_PtrSize(th)){
        printf("\tERROR: two network have different # of POs\n");
        printf("\tEC_check : stopped\n");
        return;
    }
    int i;
    Abc_Obj_t* aigObj;
    Thre_S*    thObj;
    Vec_Str_t * sAig = Vec_StrAlloc(5);
    Vec_Str_t * sTh  = Vec_StrAlloc(5);
    Vec_Str_t * sMit = Vec_StrAlloc(5);
    Vec_Str_t * sOR = Vec_StrAlloc(5);
    
    Vec_PtrForEachEntry( Abc_Obj_t*, aig, aigObj, i){
        thObj = (Thre_S *)Vec_PtrEntry( th, i);
        
        Vec_StrClear(sAig);
        Vec_StrPrintStr(sAig, "PO_");
        Vec_StrPrintNum(sAig, Abc_ObjId( aigObj ));
        Vec_StrPush(sAig, '\0');
        Vec_StrClear(sTh);
        Vec_StrPrintStr(sTh, "TO_");
        Vec_StrPrintNum(sTh, thObj->Id);
        Vec_StrPush(sTh, '\0');
        Vec_StrClear(sMit);
        Vec_StrPrintStr(sMit, "M_");
        Vec_StrPrintNum(sMit, i);
        Vec_StrPush(sMit, '\0');

        fprintf(oFile, "+1*%s -1*%s +1*%s >= 0;\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s +1*%s -1*%s >= 0;\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s -1*%s -1*%s <= 0;\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
        fprintf(oFile, "+1*%s +1*%s +1*%s <= 2;\n", 
               sAig->pArray, sTh->pArray, sMit->pArray);
    }
    Vec_StrPrintStr(sOR, "+1*M_0");
    fprintf(oFile, "-1*M_0 +1*Z >= 0;\n");
    for( i = 1; i < Vec_PtrSize(aig); i++){
        Vec_StrAppend(sOR, " +1*M_");
        Vec_StrPrintNum(sOR, i);
        fprintf(oFile, "-1*M_%d +1*Z >= 0;\n", i);
    }
    Vec_StrAppend(sOR, " -1*Z >= 0;\n");
    Vec_StrPush(sOR, '\0');
    fprintf(oFile, sOR->pArray);
    
    printf("\tdone\n");
    Vec_StrFree(sAig);
    Vec_StrFree(sTh);
    Vec_StrFree(sMit);
    Vec_StrFree(sOR);
}
//////////////////////////////////////////////////////////////////
Vec_Ptr_t* thre_PB(FILE* oFile, Vec_Ptr_t *thList){
    Vec_Ptr_t * thPOList = Vec_PtrAlloc(10);
    /* Reminder:
     * Con1: (M-T+1)y - sigma_N ( wixi ) >= 1-T
     * Con2: (m-T)  y + sigma_N ( wixi ) >= m
     * VAR naming:
     * const: CONST1 
     * PI: I_<id>  
     * PO: TO_<id>
     * TH: th_<id>
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
            Vec_StrPrintStr( sNode, "TO_");
            Vec_PtrPush( thPOList, tObj);
        }
        else{
            Vec_StrPrintStr( sNode, "th_");
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
                Vec_StrPrintStr(sFin, "th_");
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
//////////////
int Max_Thre(Thre_S* tObj){
    int sum = 0;
    int i,num;
    Vec_IntForEachEntry( tObj->weights, num, i ){
        if(num > 0) sum += num;
    }
    return sum;
}
/////////////
int min_Thre(Thre_S* tObj){
    int sum = 0;
    int i,num;
    Vec_IntForEachEntry( tObj->weights, num, i ){
        if(num < 0) sum += num;
    }
    return sum;
}
//////////////////////////////////////////////////////////////////
Vec_Ptr_t* alan_PB(FILE* oF, Abc_Ntk_t *pNtk)
{
    /* VAR naming:
     * PI: I_<id>
     * PO: PO_<id>
     * aig: N_<id>
     */
    Vec_Str_t * sNode = Vec_StrAlloc(5);
    Vec_Str_t * sFin0 = Vec_StrAlloc(5);
    Vec_Str_t * sFin1 = Vec_StrAlloc(5);
    Vec_Ptr_t * aigPOList = Vec_PtrAlloc(10);
    int i;
    Abc_Obj_t * pObj;
    Abc_NtkForEachObj( pNtk, pObj, i){
        //printf("Node: %d, Type: %d\n", Abc_ObjId(pObj), Abc_ObjType(pObj));
        if( pObj->Type == ABC_OBJ_PI || pObj->Type == ABC_OBJ_CONST1 )
            continue;
        
        if( pObj->Type == ABC_OBJ_PO ){
            Vec_PtrPush( aigPOList, pObj );
            Abc_Obj_t * pFin0 = Abc_ObjFanin0(pObj);
            int NodeId = Abc_ObjId(pObj);
            int Fin0Id = Abc_ObjId(pFin0);
            
            Vec_StrClear( sNode );
            Vec_StrPrintStr( sNode, "PO_" );
            Vec_StrPrintNum( sNode, NodeId);
            Vec_StrPush(sNode, '\0');
            
            Vec_StrClear( sFin0 );
            if (Abc_ObjIsNode(pFin0)){
                Vec_StrPrintStr( sFin0, "N_" );
                Vec_StrPrintNum( sFin0, Fin0Id );
            }
            else if (Abc_ObjIsPi(pFin0)){
                Vec_StrPrintStr( sFin0, "I_" );
                Vec_StrPrintNum( sFin0, Fin0Id);
            }
            else
                Vec_StrPrintStr( sFin0, "CONST1" );
            Vec_StrPush(sFin0, '\0');
            if(Abc_ObjFaninC0(pObj)){
               // printf( "!%s -> %s\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s >= -1;\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"+1*%s +1*%s >=  1;\n", sFin0->pArray, sNode->pArray);
            }
            else{
               // printf( " %s -> %s\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"-1*%s +1*%s >= 0;\n", sFin0->pArray, sNode->pArray );
                fprintf(oF,"+1*%s -1*%s >= 0;\n", sFin0->pArray, sNode->pArray );
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
            Vec_StrPrintStr( sNode, "N_" );
            Vec_StrPrintNum( sNode, NodeId);
            Vec_StrPush(sNode, '\0');
            
            Vec_StrClear( sFin0 );
            if (Abc_ObjIsNode(pFin0)){
                Vec_StrPrintStr( sFin0, "N_" );
                Vec_StrPrintNum( sFin0, Fin0Id );
            }
            else if (Abc_ObjIsPi(pFin0)){
                Vec_StrPrintStr( sFin0, "I_" );
                Vec_StrPrintNum( sFin0, Fin0Id);
            }
            else
                Vec_StrPrintStr( sFin0, "CONST1" );
            Vec_StrPush(sFin0, '\0');
            
            Vec_StrClear( sFin1 );
            if (Abc_ObjIsNode(pFin1)){
                Vec_StrPrintStr( sFin1, "N_" );
                Vec_StrPrintNum( sFin1, Fin1Id );
            }
            else if (Abc_ObjIsPi(pFin1)){
                Vec_StrPrintStr( sFin1, "I_" );
                Vec_StrPrintNum( sFin1, Fin1Id);
            }
            else
                Vec_StrPrintStr( sFin1, "CONST1" );
            Vec_StrPush(sFin1, '\0');
            
            if( Abc_ObjFaninC0(pObj) == 0 && Abc_ObjFaninC1(pObj) == 0){
                //printf(" %s  &  %s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oF,"+1*%s -1*%s >= 0;\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"+1*%s -1*%s >= 0;\n", sFin1->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s +1*%s >= -1;\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);

            }
            else if( Abc_ObjFaninC0(pObj) == 1 && Abc_ObjFaninC1(pObj) == 0){
                //printf("!%s  &  %s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s >= -1;\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"+1*%s -1*%s >= 0;\n", sFin1->pArray, sNode->pArray);
                fprintf(oF,"+1*%s -1*%s +1*%s >= 0;\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
            else if( Abc_ObjFaninC0(pObj) == 0 && Abc_ObjFaninC1(pObj) == 1){
                //printf(" %s  & !%s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oF,"+1*%s -1*%s >= 0;\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s >= -1;\n", sFin1->pArray, sNode->pArray);
                fprintf(oF,"-1*%s +1*%s +1*%s >= 0;\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
            else {
                //printf("!%s  & !%s -> %s\n", sFin0->pArray, sFin1->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s >= -1;\n", sFin0->pArray, sNode->pArray);
                fprintf(oF,"-1*%s -1*%s >= -1;\n", sFin1->pArray, sNode->pArray);
                fprintf(oF,"+1*%s +1*%s +1*%s >= +1;\n",
                        sFin0->pArray, sFin1->pArray, sNode->pArray);
            }
        }
    }
    ///
    Abc_NtkForEachObj( pNtk, pObj, i){
        //printf("Node: %d, Type: %d\n", Abc_ObjId(pObj), Abc_ObjType(pObj));
    } 
    ///
    // Dectructor
    Vec_StrFree( sNode );
    Vec_StrFree( sFin0 );
    Vec_StrFree( sFin1 );
    return aigPOList;
}
