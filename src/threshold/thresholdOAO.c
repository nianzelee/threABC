#include "base/abc/abc.h"
#include <stdio.h>
#include "threshold.h"

void    test_OAO(Vec_Ptr_t *);
void    reportSOP(Thre_S*, Vec_Str_t*, Vec_Str_t* );
void    Recurrsive_SOP(Thre_S*, int, Vec_Str_t*, Vec_Str_t*, Vec_Str_t*, int);
void    SortNode(Thre_S*);
void    th_SimplifyNode(Thre_S*);
int     check_SOP(Vec_Str_t*, Vec_Str_t*, Vec_Str_t*, Vec_Str_t*);
////////////////////////////////////////////////////////////////////////

void test_OAO(Vec_Ptr_t * tList)
{
    printf("testOAO\n");
    int i;
    Thre_S* tObj;
    Vec_PtrForEachEntry(Thre_S*, tList, tObj, i){
        if ( tObj == NULL ) continue;
        if ( tObj->Type == 1 || tObj->Type == 4) continue;
       /*
        Vec_Str_t* onSet  = Vec_StrAlloc(10);
        Vec_Str_t* offSet = Vec_StrAlloc(10);
        SortNode(tObj);
        reportSOP(tObj, onSet, offSet);
        
        printf("id = %d\n\tonset : %s\n\toffset: %s\n",
                tObj->Id, onSet->pArray, offSet->pArray);

        Vec_StrFree(onSet);
        Vec_StrFree(offSet);
        */
        //if ( Vec_IntSize(tObj->Fanins) <= 8 )
        th_SimplifyNode( tObj );
        
        printf(" %d / %d\n", i, Vec_PtrSize(tList) );
    }
}

void th_SimplifyNode( Thre_S* tObj )
{
    if (Vec_IntSize(tObj->Fanins) > 16) return;
    Vec_Str_t* onSet  = Vec_StrAlloc(10);
    Vec_Str_t* offSet = Vec_StrAlloc(10);
    Vec_Str_t* newOn  = Vec_StrAlloc(10);
    Vec_Str_t* newOff = Vec_StrAlloc(10);
    reportSOP( tObj, onSet, offSet );
    
    // TODO:
    Thre_S* newGate = Th_CopyObj(tObj);
    int i, j;
    int size = Vec_IntSize(tObj->Fanins);
    
    for ( i = 0; i < size; ++i ){
        while(1){
            Vec_StrClear(newOn);
            Vec_StrClear(newOff);
            int         t_backup = newGate->thre;
            Vec_Int_t*  w_backup = Vec_IntDup(newGate->weights);
            int current_w = Vec_IntEntry( newGate->weights, i);
            if ( current_w > 1 ){
                if(newGate->thre != 0)
                    newGate->thre -= 1;
                for( j = i; j < size; ++j )
                    if( Vec_IntEntry( newGate->weights, j ) == current_w )
                        Vec_IntWriteEntry( newGate->weights, j, current_w -1 );
            }
            else if( current_w < -1 ){
                if(newGate->thre != 0)
                    newGate->thre += 1;
                for( j = i; j < size; ++j )
                    if( Vec_IntEntry( newGate->weights, j ) == current_w )
                        Vec_IntWriteEntry( newGate->weights, j, current_w +1 );
            }
            else{
                Vec_IntFree( w_backup );
                break;
            }
            reportSOP( newGate, newOn, newOff );
            if( check_SOP( onSet, offSet, newOn, newOff )  == 0){
                // restore back-up
                newGate->thre = t_backup;
                Vec_IntFree(newGate->weights);
                newGate->weights = Vec_IntDup( w_backup );
                Vec_IntFree( w_backup );
                break;
            }
            Vec_IntFree( w_backup );
        }
    }
    if (Vec_IntEqual( tObj->weights, newGate->weights) == 0){
        Vec_IntFree(tObj->weights);
        tObj->weights = Vec_IntDup(newGate->weights);
        tObj->thre    = newGate->thre;
    }
    Th_DeleteObjNoInsert(newGate);  
    Vec_StrFree(onSet);
    Vec_StrFree(offSet);
    Vec_StrFree(newOn);
    Vec_StrFree(newOff);
}

void reportSOP( Thre_S* tObj, Vec_Str_t* onSet, Vec_Str_t* offSet)
{
    Vec_Str_t* sCube  = Vec_StrAlloc(10);
    Recurrsive_SOP( tObj, tObj->thre, sCube, onSet, offSet, 0 );
    Vec_StrPush( onSet, '\0' );
    Vec_StrPush( offSet,'\0' );
    Vec_StrFree(sCube);
}

int check_SOP(Vec_Str_t* on1, Vec_Str_t* off1, Vec_Str_t* on2, Vec_Str_t* off2)
{
    return Vec_StrEqual(on1, on2) && Vec_StrEqual(off1, off2);
}


void Recurrsive_SOP
( Thre_S* t, int thre, Vec_Str_t* cube, Vec_Str_t* on, Vec_Str_t* off, int lvl )
{
    assert( lvl < Vec_IntSize(t->Fanins) );
    Vec_Str_t * pos_cof_cube = Vec_StrDup(cube);
    Vec_Str_t * neg_cof_cube = Vec_StrDup(cube);
    
    int FinId = Vec_IntEntry( t->Fanins, lvl);
    Vec_StrAppend ( neg_cof_cube, "!" );
    Vec_StrPrintNum( neg_cof_cube, FinId );
    Vec_StrAppend  ( neg_cof_cube, "x" );

    Vec_StrPrintNum( pos_cof_cube, FinId );
    Vec_StrAppend  ( pos_cof_cube, "x" );

    int current_w = Vec_IntEntry( t->weights, lvl );
    int maxF      = Thre_LocalMax( t, lvl );
    int minF      = Thre_LocalMin( t, lvl );
    
    int pos_cof_thre = thre - current_w;
    if ( pos_cof_thre <= minF ){
        // onset
        Vec_StrWriteEntry( pos_cof_cube, Vec_StrSize(pos_cof_cube)-1,'\0' );
        Vec_StrAppend( on, pos_cof_cube->pArray );
        Vec_StrAppend( on, " " );
    }
    else if ( maxF < pos_cof_thre ){
        // offset
        Vec_StrWriteEntry( pos_cof_cube, Vec_StrSize(pos_cof_cube)-1,'\0' );
        Vec_StrAppend( off, pos_cof_cube->pArray );
        Vec_StrAppend( off, " ");
    }
    else
        Recurrsive_SOP(t, pos_cof_thre, pos_cof_cube, on, off, lvl+1);
    
    int neg_cof_thre = thre;
    if ( neg_cof_thre <= minF ){
        // onset
        Vec_StrWriteEntry( neg_cof_cube, Vec_StrSize(neg_cof_cube)-1,'\0' );
        Vec_StrAppend( on, neg_cof_cube->pArray );
        Vec_StrAppend( on, " " );
    }
    else if ( maxF < neg_cof_thre ){
        // offset
        Vec_StrWriteEntry( neg_cof_cube, Vec_StrSize(neg_cof_cube)-1,'\0' );
        Vec_StrAppend( off, neg_cof_cube->pArray );
        Vec_StrAppend( off, " ");
    }
    else
        Recurrsive_SOP(t, neg_cof_thre, neg_cof_cube, on, off, lvl+1);
    
    Vec_StrFree(pos_cof_cube);
    Vec_StrFree(neg_cof_cube);
}

void SortNode(Thre_S* tObj)
{
    // this function will REPLACE (tObj->Fanins, tObj->weights) 
    int i,j;
    int size = Vec_IntSize(tObj->Fanins);
    for( i = 0; i < size-1; ++i ){
        for ( j = i; j < size; ++j){
            Vec_Int_t * w = tObj->weights;
            Vec_Int_t * f = tObj->Fanins;
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
}
