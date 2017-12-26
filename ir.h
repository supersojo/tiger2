/* Coding: ANSI */
#ifndef IR_H
#define IR_H

#include "tiger_type.h"
#include "parser.h"


namespace tiger{


class IREntry{
    friend class IREntryNode;
    friend class IR;
public:
    enum{
        kIREntry_Operand_Const,
        kIREntry_Operand_Temp,
        kIREntry_Operand_Sym,
        kIREntry_Operand_Label
    };
    IREntry(){
        m_operator = 0;
        m_operand1 = 0;
        m_operand2 = 0;
        m_operand3 = 0;
        s32 m_operand1_from = 0;
        s32 m_operand2_from = 0;
        s32 m_operand3_from = 0;
    }
    IREntry(s32 op,s32 o1,s32 o2,s32 o3,s32 o1_f,s32 o2_f,s32 o3_f){
        m_operator = op;
        m_operand1 = o1;
        m_operand2 = o2;
        m_operand3 = o3;
        

        m_operand1_from = o1_f;
        m_operand2_from = o2_f;
        m_operand3_from = o3_f;
    }
    
private:
    s32 m_operator;
    s32 m_operand1;
    s32 m_operand2;
    s32 m_operand3;
    
    s32 m_operand1_from;
    s32 m_operand2_from;
    s32 m_operand3_from;
};

class IREntryNode{
    friend class IR;
public:
    IREntryNode(IREntry* ir){
        m_ir = ir;
        m_label = 0;
        
        m_prev = 0;
        m_next = 0;
    }
    void BindLabel(Label* l){
        m_label=l;
    }
    ~IREntryNode(){
        delete m_ir;
    }
private:
    IREntry* m_ir;
    Label*   m_label;/* a label binding ?*/
    
    IREntryNode* m_next;
    IREntryNode* m_prev;
    
    SymTabNode* m_symtab;
};

class IR{
public:
    enum{
    CONST,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    CJMP,
    JMP,
    };
    IR(){m_head=0;m_cur=0;m_len=0;}
    IR(tiger::parser::Parser* parser){m_head=0;m_cur=0;m_len=0;}
    bool NewIREntry(s32 op,s32 o1,s32 o2,s32 o3,s32 o1_f,s32 o2_f,s32 o3_f,SymTabNode* symtab){
        IREntry* e = new IREntry(op,o1,o2,o3,o1_f,o2_f,o3_f);
        IREntryNode* n = new IREntryNode(e);
        n->m_symtab = symtab;/* for symbol table */
        if(m_cur){
            m_cur->m_next = n;
            n->m_prev = m_cur;
            
            m_cur = n;
        }
        else{
            m_head = n;
            m_cur = n;
        }
        m_len++;
        return true;
    }
    IREntryNode* GetCur(){
        return m_cur;
    }
    bool Dump(parser::Parser* parser){
        IREntryNode* p;
        p = m_head;
        while(p){
            DumpNode(p,parser);
            p = p->m_next;
        }
        return true;
    }
    ~IR(){
        IREntryNode* p;
        do{
            p = m_head;
            
            m_head = m_head->m_next;
            
            if(p)
                delete p;
            
        }while(m_head);
    }
private:
    char* GetOperandId(IREntryNode* p,s32 idx,s32 operand_from,parser::Parser* parser){
        bool ret=false;
        Temp* t;
        Symbol* s;
        LabelNode* l;
        SymbolTable* tab;
        
        if(operand_from==IREntry::kIREntry_Operand_Temp){
            ret = parser->GetTempTable()->FindByIdx(idx,&t);
            assert(ret);
            return t->Id();
        }
        
        if(operand_from==IREntry::kIREntry_Operand_Label){
            ret = parser->GetLabelTable()->FindByIdx(idx,&l);
            assert(ret);
            return l->GetLabel()->GetId();
        }
        
        assert(operand_from==IREntry::kIREntry_Operand_Sym);
        
        SymTabStackIter iter(p->m_symtab);
        for(tab=iter.Next();tab;tab=iter.Next()){
            ret=tab->FindByIdx(idx,&s);
            if(ret)
                break;
        }
        
        assert(ret);
        return s->GetId();
    }
    bool DumpNode(IREntryNode* p,parser::Parser* parser){
        Const* c;
        
        switch(p->m_ir->m_operator){
            case CONST:
                dynamic_cast<ConstTableNum*>(parser->GetConstTable())->FindByIdx(p->m_ir->m_operand1,&c);
                std::cout<<"MOV "<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<","<<dynamic_cast<TvalNum*>(c->Val())->Data()<<std::endl;
                break;
            case MOV:
                std::cout<<"MOV "<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<std::endl;
                break;
            case ADD:
                std::cout<<"ADD "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,parser)<<"->"<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<std::endl;
                break;
            case SUB:
                std::cout<<"SUB "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,parser)<<"->"<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<std::endl;
                break;
            case MUL:
                std::cout<<"MUL "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,parser)<<"->"<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<std::endl;
                break;
            case DIV:
                std::cout<<"DIV "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,parser)<<"->"<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<std::endl;
                break;
            case CJMP:
                std::cout<<"CJMP "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,parser)<<","<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,parser)<<std::endl;
                break;
        }
        if(p->m_label){
            std::cout<<p->m_label->GetId()<<":"<<std::endl;
        }
        return true;
    }

    
    IREntryNode*  m_head;
    IREntryNode*  m_cur;
    
    s32 m_len;
    
};

    
}// namespace tiger

#endif

