/* Coding: ANSI */
#ifndef LABEL_H
#define LABEL_H

#include <assert.h>
#include <iostream>

#include "tiger_type.h"

namespace tiger{

class Label{
    friend class LabelNode;
    friend class LabelTable;
public:
    enum{
        kLabel_Max_Len=32
    };
    Label();
    char* GetId(){return m_id;}
    s32   GetPos(){return m_pos;}
    static void Init(char* prefix,s32 max_len);
    static void Exit();
    ~Label();
private:
    char* m_id;
    s32   m_pos;
    
    static char* Label_Prefix;
    static s32   Label_Next_Id;
    static char* Label_Buf;
    static s32   Label_Max_Len;
};

class LabelNode{
    friend class LabelTable;
    friend class LabelNodeIter;
public:
    LabelNode(){m_label=0;m_next=0;}
    LabelNode(Label* label){m_label=label;m_next=0;}
    s32 GetIdx(){return m_idx;}
    Label* GetLabel(){return m_label;}
    ~LabelNode(){delete m_label;}
private:
    Label*     m_label;
    LabelNode* m_next;
    s32        m_idx;
};
class LabelList{
    friend class IR;
public:
    LabelList(){m_label=0;m_next=0;}
    LabelList(Label* l,LabelList* list,s32 bindtype){m_label=l;m_next=list;m_bindtype=bindtype;}
    LabelList* GetNext(){return m_next;}
    Label*     GetLabel(){return m_label;}
    s32        GetBindtype(){return m_bindtype;}
    ~LabelList(){}
private:
    Label*     m_label;
    LabelList* m_next;
    s32        m_bindtype;
};
class LabelNodeIter{
public:
    LabelNodeIter(){m_next=0;m_orig=0;}
    LabelNodeIter(LabelNode* label){m_next=label;m_orig=label;}
    LabelNode* Next(){
        LabelNode* n = m_next;
        if(n)
            m_next = n->m_next;
        return n;
    }
    void Reset(){
        m_next = m_orig;
    }
private:
    LabelNode* m_next;
    LabelNode* m_orig;
};

class LabelTable{
public:
    enum{
        kLabelTable_Hash_Size=32
    };
    LabelTable(){
        m_tab = new LabelNode*[kLabelTable_Hash_Size];
        m_next_idx = new s32[kLabelTable_Hash_Size];
        for(s32 i=0;i<kLabelTable_Hash_Size;i++){
            m_tab[i]=0;
            m_next_idx[i]=1;/* default from 1 */
        }
    }
    bool Find(char* label_str,LabelNode**out){
        LabelNode* lab;
        s32 idx = hash(label_str);
        lab = m_tab[idx];
        LabelNodeIter iter(lab);
        for(lab=iter.Next();lab;lab=iter.Next()){
            if(strcmp(lab->m_label->m_id,label_str)==0){
                if(out)
                    *out = lab;
                return true;
            }
        }
        return false;
    }
    bool FindByIdx(s32 idx,LabelNode** out){
        LabelNode* p;
        LabelNode* c = m_tab[idx>>8];
        LabelNodeIter iter(c);
        for(p=iter.Next();p;p=iter.Next())
        {
            if(p->m_idx==idx){
                if(out)
                    *out = p;
                return true;
            }
        }
        return false;
    }
    bool Insert(Label* label){
        s32 idx;
        LabelNode* l;
        LabelNode* n;
        
        
        if(Find(label->GetId(),0))
            return false;
        
        idx = hash(label->GetId());
        l = m_tab[idx];
        n = new LabelNode(label);
        /*
        table index
        */
        n->m_idx = (idx<<8) + m_next_idx[idx];
        
        m_next_idx[idx]++;
        
        n->m_next = m_tab[idx];
        m_tab[idx] = n;
        return true;
    }
    bool Dump(){
        LabelNode* t;
        for(s32 i=0;i<kLabelTable_Hash_Size;i++){
            if(m_tab[i]){
                LabelNodeIter iter(m_tab[i]);
                for(t=iter.Next();t;t=iter.Next())
                {
                    //to be continue
                    //...
                    std::cout<<t->m_label->m_id<<"---"<<t->m_label->m_pos<<std::endl;
                }
            }
        }
        return true;    
    }
    ~LabelTable(){
        for(s32 i=0;i<kLabelTable_Hash_Size;i++)
            FreeLabelNode(m_tab[i]);
        
        delete[] m_tab;
        
        delete[] m_next_idx;
    }
private:
    s32 hash(char* label_str){
        u32 idx = 0;
        s32 len = strlen(label_str);
        for(s32 i=0;i<len;i++)
            idx+=(*(label_str+i))*13;
        return idx%kLabelTable_Hash_Size;
    }
    void FreeLabelNode(LabelNode* l){
        LabelNode* p,*q;
        p = q = l;
        while(p){
            q = p->m_next;
            delete p;
            p = q;
        }
    }
    LabelNode** m_tab;
    s32*        m_next_idx;
};

}// namespace tiger

#endif
