/* Coding: ANSI */
#ifndef CONST_H
#define CONST_H

#include <iostream>

namespace tiger{
    
class Const{
friend class ConstTable;
friend class ConstTableNum;
friend class ConstIter;
public:
    Const(){m_val=0;m_next=0;}
    Const(Tval* val){m_val = val;m_next=0;}
    Tval* Val(){return m_val;}
    s32 GetIdx(){return m_idx;}
    ~Const(){delete m_val;}
private:
    Tval* m_val;
    Const* m_next;
    s32 m_idx;
}; 
class ConstIter{
public:
    ConstIter(){m_next=0;m_orig=0;}
    ConstIter(Const* c){m_next = c;m_orig = c;}
    Const* Next(){
        Const* n = m_next; 
        if(n)
            m_next = n->m_next;
        return n;
    }
    void Reset(){
        m_next = m_orig;
    }
private:
    Const* m_next;
    Const* m_orig;
};    
class ConstTable{
public:
    enum{
        kConstTable_Num,
        kConstTable_Str,
        kConstTable_Invalid
    };
    enum{
        kConstTableHashSize=32
    };
    ConstTable(){m_kind=kConstTable_Invalid;m_tab_hash_size=0;}
    ConstTable(s32 kind){m_kind = kind;m_tab_hash_size=kConstTableHashSize;}
    virtual s32 Kind(){return m_kind;}
    s32 HashSize(){return m_tab_hash_size;}
private:
    s32 m_kind;
    s32 m_tab_hash_size;
};
class ConstTableNum:public ConstTable{
public:
    ConstTableNum():ConstTable(kConstTable_Num){
        m_tab = new Const*[HashSize()];
        m_next_idx = new s32[HashSize()];
        for(s32 i=0;i<HashSize();i++){
            m_tab[i]=0;
            m_next_idx[i]=1;/*default idx from 1 */
        }
    }
    bool Find(s32 val,Const** out){
        Const* p;
        
        Const* c = m_tab[hash(val)%HashSize()];
        
        ConstIter iter(c);
        
        for(p=iter.Next();p;p=iter.Next())
        {
            if(dynamic_cast<TvalNum*>(p->m_val)->Data()==val){
                if(out)
                    *out = p;
                return true;
            }
        }
        return false;
    }
    bool FindByIdx(s32 idx,Const** out){
        Const* p;
        Const* c = m_tab[idx>>8];
        ConstIter iter(c);
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
    bool Insert(s32 val){
        s32 idx;
        if(Find(val,0))
            return false;
        
        idx = hash(val)%HashSize();
        
        Const* n = new Const(new TvalNum(val));
        /* operator prority */
        n->m_idx = (idx<<8)+m_next_idx[idx];
        
        m_next_idx[idx]++;
        
        n->m_next = m_tab[idx];
        
        m_tab[hash(val)%HashSize()] = n;
        
        return true;
            
    }
    bool Dump(){
        Const* t;
        for(s32 i=0;i<HashSize();i++){
            if(m_tab[i]){
                ConstIter iter(m_tab[i]);
                for(t=iter.Next();t;t=iter.Next())
                {
                    //to be continue
                    //...
                    std::cout<<t->GetIdx()<<"---"<<dynamic_cast<TvalNum*>(t->Val())->Data()<<std::endl;
                }
            }
        }
        return true;
    }
    ~ConstTableNum(){
        for(s32 i=0;i<HashSize();i++)
            FreeConst(m_tab[i]);
        delete[] m_tab; 
        delete[] m_next_idx;
    }
private:
    void FreeConst(Const* c){
        Const* p,*q;
        p = q = c;
        while(p){
            q = p->m_next;
            delete p;
            p = q;
        }
    }
    s32 hash(s32 i){
        return i;
    }
    
    Const** m_tab;
    s32* m_next_idx;
};
    
}// namespace tiger

#endif
