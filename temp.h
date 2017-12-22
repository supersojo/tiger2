/* Coding:ANSI */
#ifndef TEMP_H
#define TEMP_H

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tiger_type.h"
#include "symtable.h"

namespace tiger{
    
class Temp{
    friend class TempNode;
    friend class TempTable;
public:
    Temp(){
        assert(IsInitialized==1);
        
        
        memset(Temp_Buf,0,Temp_Max_Len);
        sprintf(Temp_Buf,"%s%04d",Temp_Prefix,Temp_Id);
        
        Temp_Id++;
        
        m_id = strdup(Temp_Buf);
        
        m_val = 0;
        
        m_idx = 0;
    }
    void Bind(Tval* val){
        m_val = val;
    }
    char* Id(){return m_id;}
    s32 GetIdx(){return m_idx;}
    static void Init(char* prefix,int max_len);
    static void Exit();
    ~Temp(){
        free(m_id);
        delete m_val;
    }
private:
    static char* Temp_Prefix;
    static s32   Temp_Id;
    static s32   Temp_Max_Len;
    static s32   IsInitialized;//
    static char* Temp_Buf;
    
    char* m_id;
    Tval* m_val;
    s32   m_idx;
};
class TempNode{
    friend class TempTable; 
    friend class TempNodeIter;
public:
    TempNode(){m_temp=0;m_next=0;}
    TempNode(Temp* temp){m_temp=temp;m_next=0;}
    ~TempNode(){
        if(m_temp)
            delete m_temp;
    }
private:
    Temp* m_temp;
    TempNode* m_next;
};
class TempNodeIter{
public:
    TempNodeIter(TempNode* t){m_next=t;m_orig=t;}
    TempNode* Next(){
        TempNode* t = m_next;
        if(t)
            m_next = t->m_next;
        return t;
    }
    void Reset(){
        m_next = m_orig;
    }
private:
    TempNode* m_next;
    TempNode* m_orig;
};
class TempTable{
public:
    enum{
        kTempTable_Hash_Size=32
    };
    TempTable(){
        m_tab = new TempNode*[kTempTable_Hash_Size];
        m_next_idx = new s32[kTempTable_Hash_Size];
        for(s32 i=0;i<kTempTable_Hash_Size;i++){
            m_tab[i]=0;
            m_next_idx[i] = 1;/* default idx from 1 */
        }
        m_size = 0;

    }
    bool Find(char* id,Temp**out){
        TempNode* t = m_tab[hash(id)];
        TempNodeIter iter(t);
        for(t=iter.Next();t;t=iter.Next())
        {
            if(strcmp(t->m_temp->Id(),id)==0)
            {
                if(out)
                    *out = t->m_temp;
                return true;
            }
        }
        return false;
    }
    bool FindByIdx(s32 idx,Temp** out){
        TempNode* p;
        TempNode* c = m_tab[idx>>8];
        TempNodeIter iter(c);
        for(p=iter.Next();p;p=iter.Next())
        {
            if(p->m_temp->m_idx==idx){
                if(out)
                    *out = p->m_temp;
                return true;
            }
        }
        return false;
    }
    bool Insert(Temp* t){
        if(Find(t->Id(),0))
            return false;
        TempNode* n = new TempNode(t);
        s32 idx = hash(t->Id());
        n->m_next = m_tab[idx];
        n->m_temp->m_idx = (idx<<8)+m_next_idx[idx];
        
        m_next_idx[idx]++;
        
        m_tab[idx] = n;
        
        m_size++;
        
        return true;
    }
    s32 Size(){return m_size;}
    bool Dump(){
        TempNode* t;
        for(s32 i=0;i<kTempTable_Hash_Size;i++){
            if(m_tab[i]){
                TempNodeIter iter(m_tab[i]);
                for(t=iter.Next();t;t=iter.Next())
                {
                    std::cout<<t->m_temp->Id()<<"--"<<t->m_temp->GetIdx()<<std::endl;
                }
            }
        }
        return true;
    }
    ~TempTable(){
        for(s32 i=0;i<kTempTable_Hash_Size;i++)
            FreeTempNode(m_tab[i]);
        delete[] m_tab;
        delete[] m_next_idx;
    }
private:
    void FreeTempNode(TempNode* c){
        TempNode* p,*q;
        p = q = c;
        while(p){
            q = p->m_next;
            delete p;
            p = q;
        }
    }
    s32 hash(char* id){
        u32 idx = 0;
        s32 len = strlen(id);
        for(s32 i=0;i<len;i++)
            idx+=(*(id+i))*13;
        return idx%kTempTable_Hash_Size;
    }
    TempNode** m_tab;
    s32* m_next_idx;
    s32  m_size;
};

} // namespace tiger

#endif
