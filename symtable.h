/* Coding:ANSI */
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "tiger_type.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace tiger{

/*
----------------
sym_name | Tval
----------------
a        | 1
b        | 2
*/
class Tval{
public:
    enum{
        kTval_Num,
        kTval_Str,
        kTval_Invalid
    };
    Tval(){m_kind=kTval_Invalid;}
    Tval(s32 kind){m_kind=kind;}
    virtual s32 Kind(){return m_kind;}
private:
    s32 m_kind;
};
class TvalNum:public Tval{
public:
    TvalNum():Tval(kTval_Num){m_ival=0;}
    TvalNum(s32 ival):Tval(kTval_Num){m_ival=ival;}
    s32 Data(){return m_ival;}
private:
    s32 m_ival;
};
class TvalStr:public Tval{
public:
    TvalStr():Tval(kTval_Str){m_sval=0;}
    TvalStr(char* sval):Tval(kTval_Str){m_sval=strdup(sval);}
    char* Data(){return m_sval;}
    ~TvalStr(){
        if(m_sval)
            free(m_sval);
    }
private:
    char* m_sval;
};

class SymbolIterator;

class Symbol{
friend class SymbolIterator;
friend class SymbolTable;
public:
    
    Symbol(char* id){
        m_id = strdup(id);
        m_val = new Tval;
        m_next = 0;
    }
    char* GetId(){return m_id;}
    s32   GetIdx(){return m_idx;}
    Tval* GetVal(){return m_val;}
    bool Bind(Tval* val){
        if(m_val) // free previous memory
            delete m_val;
        m_val=val;
        return true;
    }
    ~Symbol(){
        if(m_id)    
            free(m_id);
        if(m_val)
            delete m_val;
    }
private:
    Symbol(){
        m_id = 0;
        m_val = 0;
        m_idx = 0;
        m_next = 0;
    }
    
    char* m_id;
    Tval* m_val;
    s32   m_idx;/* identify id number */
    
    Symbol* m_next;
};
class SymbolIterator{
public:
    SymbolIterator(Symbol* sym){
        m_orig = sym;
        m_cur = sym;
    }
    Symbol* Next(){
        Symbol* n = m_cur;
        if(n)
            m_cur = n->m_next;
        return n;
    }
    void Reset(){
        m_cur = m_orig;
    }
private:
    Symbol* m_orig;
    Symbol* m_cur;
};
class SymbolTable{
public:
    enum{
      kSymbolTableSize = 32
    };
    SymbolTable(){
        m_table = new Symbol*[kSymbolTableSize];
        for(s32 i=0;i<kSymbolTableSize;i++)
            m_table[i]=0;
        m_table_size = kSymbolTableSize;

        m_next_idx = new s32[kSymbolTableSize];
        for(s32 i=0;i<kSymbolTableSize;i++)
            m_next_idx[i] = 1;/* default start idx */
    }
    bool Find(char* id,Symbol** out){
        s32 idx;
        Symbol* sym;
        idx = hash(id);
        sym = m_table[idx];
        SymbolIterator iter(sym);

        for(sym=iter.Next();sym;sym=iter.Next())
        {
            
            if(strcmp(sym->GetId(),id)==0){
                if(out)
                    *out = sym;
                return true;
            }
        }
        return false;
    }
    bool FindByIdx(s32 idx,Symbol** out){
        Symbol* sym = m_table[idx>>8];
        SymbolIterator iter(sym);
        for(sym=iter.Next();sym;sym=iter.Next())
        {
            if(sym->m_idx==idx){
                if(out)
                    *out = sym;
                return true;
            }
        }
        return false;
    }
    bool Insert(char* id){
        s32 idx;
        Symbol* sym;
        Symbol* n;
        
        
        if(Find(id,0))
            return false;
        
        idx = hash(id);
        sym = m_table[idx];
        n = new Symbol(id);
        /*
        table index
        */
        n->m_idx = (idx<<8) + m_next_idx[idx];
        
        m_next_idx[idx]++;
        
        n->m_next = m_table[idx];
        m_table[idx] = n;
        return true;
    }
    ~SymbolTable(){
        for(s32 i=0;i<kSymbolTableSize;i++)
            FreeSym(m_table[i]);
        
        delete[] m_table;
        
        delete[] m_next_idx;
    }
private:
    void FreeSym(Symbol* sym){
        Symbol* p,*q;
        p = q = sym;
        while(p){
            q = p->m_next;
            delete p;
            p = q;
        }
    }
    u32 hash(char* id){
        u32 idx = 0;
        s32 len = strlen(id);
        for(s32 i=0;i<len;i++)
            idx+=(*(id+i))*13;
        return idx%kSymbolTableSize;
    }
    Symbol** m_table;
    s32* m_next_idx;
    s32 m_table_size;
};
class SymTabNode{
    friend class SymTabStack; 
    friend class SymTabStackIter;
public:
    SymTabNode(){m_tab = 0; m_next = 0;}
    SymTabNode(SymbolTable* tab){m_tab = tab;m_next = 0;}
    ~SymTabNode(){delete m_tab;}
private:
    SymbolTable* m_tab;
    SymTabNode* m_next;
};
/*
  top
  |
  [n]->...->[1]
  |
  cur
*/
class SymTabStack{
    friend class SymTabStackIter;
public:
    SymTabStack(){m_top = 0;m_size=0;}
    SymbolTable* Top(){
        if(m_top)
            return m_top->m_tab;
        else
            return 0;/* no element in stack */
    }
    void Push(SymbolTable* tab){
        SymTabNode* n = new SymTabNode(tab);
        n->m_next = m_top;
        m_top = n;
        m_size++;
    }
    void Pop(){
        SymTabNode* p = m_top;
        if(m_top){
            m_top = m_top->m_next;
            m_size--;
            delete p;
        }
    }
    s32 Size(){return m_size;}
private:
    SymTabNode* m_top;
    s32 m_size;
};
class SymTabStackIter{
public:
    SymTabStackIter(SymTabStack* stack){
        m_next = stack->m_top;
        m_orig = stack->m_top;
    }
    SymbolTable* Next(){
        SymTabNode* n = m_next;
        if(n){
            m_next = n->m_next;
            return n->m_tab;
        }
        return 0;
    }
    void Reset(){
        m_next = m_orig;
    }
private:
    SymTabNode* m_next;
    SymTabNode* m_orig;
};

} // namespace tiger
#endif
