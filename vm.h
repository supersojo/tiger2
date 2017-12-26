/* Coding: ANSI */
#ifndef VM_H
#define VM_H

#include "tiger_type.h"
#include "parser.h"
#include "ir.h"

namespace tiger{

class VM{
public:
    VM(parser::Parser* parser){m_parser = parser;}
    bool Run(){
        s32 i = 0;
        IR* ir = m_parser->GetIR();
        IREntryNode* p;
        p = ir->GetHead();
        while(p){
            i++;
            RunOne(p);
            p = p->m_next;
            
        }
        return true;
    }
    bool Dump(){
        m_parser->GetSymTabStack()->Top()->Dump();
        return true;
    }
private:
    Tval* GetOperandTval(IREntryNode* p,s32 idx,s32 operand_from,parser::Parser* parser){
        bool ret=false;
        Temp* t;
        Symbol* s;
        LabelNode* l;
        SymbolTable* tab;
        
        Tval* v = new TvalNum;
        
        if(operand_from==IREntry::kIREntry_Operand_Temp){
            ret = parser->GetTempTable()->FindByIdx(idx,&t);
            assert(ret);
            if(t->Val()){
                delete v;
                return t->Val();
            }
            t->Bind(v);
            return v;
        }
        
        if(operand_from==IREntry::kIREntry_Operand_Label){
            ret = parser->GetLabelTable()->FindByIdx(idx,&l);
            assert(ret);
            delete v;
            return 0;/* only execute expression calculating */
        }
        
        assert(operand_from==IREntry::kIREntry_Operand_Sym);
        
        SymTabStackIter iter(p->m_symtab);
        for(tab=iter.Next();tab;tab=iter.Next()){
            ret=tab->FindByIdx(idx,&s);
            if(ret)
                break;
        }
        //std::cout<<idx<<std::endl;
        assert(ret);

        delete v;
        return s->GetVal();

    }
    bool RunOne(IREntryNode* p){
        Const* c;
        Tval* v1,*v2,*v3;
        switch(p->m_ir->m_operator){
            case IR::CONST:
                dynamic_cast<ConstTableNum*>(m_parser->GetConstTable())->FindByIdx(p->m_ir->m_operand1,&c);
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(c->Val())->Data() );
                break;
            case IR::MOV:
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                v3 = GetOperandTval(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser);
                
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(v3)->Data() );
                break;
            case IR::ADD:
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                v2 = GetOperandTval(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,m_parser);
                v3 = GetOperandTval(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser);
                
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(v2)->Data() + dynamic_cast<TvalNum*>(v3)->Data() );
                
                break;
            case IR::SUB:
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                v2 = GetOperandTval(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,m_parser);
                v3 = GetOperandTval(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser);
                
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(v3)->Data() - dynamic_cast<TvalNum*>(v2)->Data() );
                break;
            case IR::MUL:
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                v2 = GetOperandTval(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,m_parser);
                v3 = GetOperandTval(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser);
                
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(v3)->Data() * dynamic_cast<TvalNum*>(v2)->Data() );
                break;
            case IR::DIV:
                v1 = GetOperandTval(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser);
                v2 = GetOperandTval(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,m_parser);
                v3 = GetOperandTval(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser);
                
                dynamic_cast<TvalNum*>(v1)->SetData( dynamic_cast<TvalNum*>(v3)->Data() / dynamic_cast<TvalNum*>(v2)->Data() );
                break;
            case IR::CJMP:
                //std::cout<<"CJMP "<<GetOperandId(p,p->m_ir->m_operand1,p->m_ir->m_operand1_from,m_parser)<<","<<GetOperandId(p,p->m_ir->m_operand2,p->m_ir->m_operand2_from,m_parser)<<","<<GetOperandId(p,p->m_ir->m_operand3,p->m_ir->m_operand3_from,m_parser)<<std::endl;
                break;
        }

        return true;
    }
    
    parser::Parser* m_parser;
};



}// namespace tiger

#endif
