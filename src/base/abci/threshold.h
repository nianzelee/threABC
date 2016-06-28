#ifndef THRESHOLD_H_
#define THRESHOLD_H_
#include"base/abc/abc.h"
#include"stdio.h"
#include<math.h>
typedef struct Thre_S
{
        int thre;
        int Type;
        int Ischoose;
        int Id;
	int oId;
        int nId;
	int cost;
	int level;
	char*  pName;
        Vec_Int_t* weights;
        Vec_Int_t* Fanins;
        Vec_Int_t* Fanouts;
}Thre_S;

typedef struct Pair_S
{
        int IntK;
        int IntL;
}Pair_S;


typedef struct Thre_Manager
{
	Vec_Ptr_t * thre_list;
	Vec_Ptr_t * vPis;
	Vec_Ptr_t * vPos;
	Vec_Ptr_t * thre_final;
}Thre_Manager;

typedef struct Th_Cut
{
	void * mCut;
	word * mTruth;
	char * name;
	Vec_Int_t * vFanins;
	Vec_Int_t * vFanouts;
}Th_Cut;

#endif
