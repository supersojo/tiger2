#include <iostream>

#include "parser.h"
#include "symtable.h"
#include "ir.h"
#include "label.h"

namespace tiger{

namespace parser{

Parser::Parser(scanner::SourceCodeStreamBase* stream)
{
    scanner::Scanner* scanner = new scanner::Scanner(stream);
    
    m_scanner = scanner;
    
    m_stack = new SymTabStack();
    
    
    /* temp allocator init*/
    Temp::Init("TMP",32);
    
    m_temp = new TempTable;
    
    /* const table */
    m_const_num = new ConstTableNum;
    
    /* label table */
    Label::Init("L",32);
    
    m_label = new LabelTable;
    
    m_ir = new IR(this);
    
}
s32 Parser::Parse()
{
    StatementList* statementList;
    Token t;
    s32 v;
    
    /* global symbol table */
    SymbolTable* tab;
    tab = new SymbolTable;
    
    /* push global sym table */
    m_stack->Push(tab);
    
    statementList = _ParseStatementList();
    
    v = m_scanner->Next(&t);
    assert(v==kToken_EOT);
    
    /**
    std::cout<<m_temp->Size()<<std::endl;
    m_temp->Dump();
    
    dynamic_cast<ConstTableNum*>(m_const_num)->Dump();
    
    m_stack->Top()->Dump();
    **/
    m_ir->Dump(this);
    
    return 0;
}
Statement* Parser::_ParseStatement()
{
    s32 v,v1;
    Token t,t1;
    Expr* expr;
    Symbol* sym;
    StatementList* statementList;
    SymbolTable* tab;
    Statement* statement;
    
    v = m_scanner->Next(&t);
    
    /* Statement->StatementAssign */
    if(v==kToken_ID){
        //std::cout<<"id:"<<t.u.name<<std::endl;
        SymTabStackIter iter(m_stack);
        for(tab=iter.Next();tab;tab=iter.Next())
        {
            if(tab->Find(t.u.name,&sym)){
                break;
            }
        }
        /* new symbol */
        if(tab){
            ;//std::cout<<t.u.name<<" already exist"<<std::endl;
        }else{
            m_stack->Top()->Insert(t.u.name);
            m_stack->Top()->Find(t.u.name,&sym);
        }
        
        v1 = m_scanner->Next(&t1);
        
        assert(v1==kToken_ASSIGN);
        
        expr = _ParseExpr();
        
        m_ir->NewIREntry(IR::MOV,expr->GetTemp()->GetIdx(),0,sym->GetIdx(),IREntry::IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Sym,m_stack->TopNode());
        
        //std::cout<<"MOV "<<t.u.name<<","<<expr->GetTemp()->Id()<<std::endl;
        //std::cout<<"new assign statement"<<std::endl;
        return new StatementAssign(t.u.name,expr);
    }
    
    /* Statement->StatmentBlock */
    if(v==kToken_LBRA){
        /* new lexical level */
        SymbolTable* tab = new SymbolTable;
        m_stack->Push(tab);
        
        //std::cout<<"new symbol table!"<<std::endl;
        
        statementList = _ParseStatementList();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RBRA);
        
        /* dump the new symbol table */
        //m_stack->Top()->Dump();
        
        /* back to previous level */
        m_stack->Pop();
        
        //std::cout<<"delete symbol table!"<<std::endl;
        
        //std::cout<<"new block statement"<<std::endl;
        return new StatementBlock(statementList);
    }
    
    /* Statement->StatmenIf */
    if(v==kToken_IF){
        expr = _ParseExpr();
        /* new Label
           GetIr()
           Ir->Bind(new Label)
        */
        Label* label_true = new Label;
        Label* label_false = new Label;
        LabelNode* l_true,*l_false;
        //std::cout<<"new label "<<label_true->GetId()<<std::endl;
        //std::cout<<"new label "<<label_false->GetId()<<std::endl;
        
        m_label->Insert(label_true);
        m_label->Insert(label_false);
        
        m_label->Find(label_true->GetId(),&l_true);
        m_label->Find(label_false->GetId(),&l_false);
        
        m_ir->NewIREntry(IR::CJMP,expr->GetTemp()->GetIdx(),l_true->GetIdx(),l_false->GetIdx(),IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Label,IREntry::kIREntry_Operand_Label,m_stack->TopNode());
        
        m_ir->GetCur()->BindLabel(label_true);
        
        statement = _ParseStatement();

        m_ir->GetCur()->BindLabel(label_false);
        
        //std::cout<<"IF "<<expr->GetTemp()->Id()<<std::endl;
        return new StatementIf(expr,statement);
        
    }
    
    // Note: should push back for parsing
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
    
    /* Statment->empty */
    return 0;
}
StatementList* Parser::_ParseStatementList()
{
    s32 v;
    Token t;
    
    Statement* statement=0;
    StatementList* statementList=0;
    StatementList* n=0;
    
    statement = _ParseStatement();
    
    /* StatementList->empty */
    if(statement==0)
        return 0;
    
    /* StatementList->Statement  StatementList*/
    statementList = _ParseStatementList();
    
    n = new StatementList(statement);
    n->m_next = statementList;
    return n;
}
Expr* Parser::_ParseExpr()
{
    Term* term;
    Erest* erest;
    Expr* n;
    Symbol* sym;
    Temp* temp;
    
    /* E->TE' */
    term = _ParseTerm();
    erest = _ParseErest(term);
    
    /* E->empty */
    if(!term&&!erest)
       return 0;
   
    n = new Expr(term,erest);
    temp = new Temp;
    n->SetTemp(temp);
    assert(m_temp->Find(temp->Id(),0)==0);
        
    m_temp->Insert(temp);
    assert(m_temp->Find(temp->Id(),0)==1);
    
    //
    //m_stack->Top()->Insert(n->GetTemp()->Id());
    //m_stack->Top()->Find(n->GetTemp()->Id(),&sym);
    
    if(!erest){
        m_ir->NewIREntry(IR::MOV,term->GetTemp()->GetIdx(),0,n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<std::endl;
    }else{
        m_ir->NewIREntry(IR::MOV,erest->GetTemp()->GetIdx(),0,n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<erest->GetTemp()->Id()<<std::endl;
    }
    return n;
}
Term* Parser::_ParseTerm()
{
    Factor* factor;
    Trest* trest;
    Term * n;
    Temp* temp;
    /* T->FT' */
    factor = _ParseFactor();
    
    trest = _ParseTrest(factor);
    
    /* T->empty */
    if(!factor&&!trest)
        return 0;
    
    n = new Term(factor,trest);
    temp = new Temp;
    n->SetTemp(temp);
    
    assert(m_temp->Find(temp->Id(),0)==0);
        
    m_temp->Insert(temp);
    assert(m_temp->Find(temp->Id(),0)==1);
            
    if(!trest){
        m_ir->NewIREntry(IR::MOV,factor->GetTemp()->GetIdx(),0,n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<std::endl;
    }else{
        m_ir->NewIREntry(IR::MOV,trest->GetTemp()->GetIdx(),0,n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<trest->GetTemp()->Id()<<std::endl;
    }
    return n;
}
Erest* Parser::_ParseErest(Term* ter)
{
    s32 v;
    Token t;
    Term* term;
    Erest* erest;
    Erest* n;
    Temp* temp;
    
    v = m_scanner->Next(&t); 
    /* E'->+TE' E'->-TE' */
    if(v==kToken_ADD||v==kToken_SUB)
    {
        term = _ParseTerm();
        erest = _ParseErest(term);
        if(v==kToken_ADD){
            n = new ErestAdd(term,erest);
            temp = new Temp;
            n->SetTemp(temp);
            
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
            
            m_ir->NewIREntry(IR::ADD,ter->GetTemp()->GetIdx(),term->GetTemp()->GetIdx(),n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
            //std::cout<<"ADD "<<ter->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
        if(v==kToken_SUB){
            n = new ErestSub(term,erest);
            temp = new Temp;
            n->SetTemp(temp);
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
            
            m_ir->NewIREntry(IR::SUB,ter->GetTemp()->GetIdx(),term->GetTemp()->GetIdx(),n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
            //std::cout<<"SUB "<<ter->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
    }
    
    // should push back
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
    /* E'->empty */
    return 0;
}
Factor* Parser::_ParseFactor()
{
    s32 v;
    Token t;
    Expr* expr;
    Factor* factor;
    Temp* temp;
    SymbolTable* tab;
    Symbol* sym;
    Const* c;
    
    v = m_scanner->Next(&t);
    /* F->num */
    if(v==kToken_NUM){
        factor = new FactorNum(t.u.ival);
        temp = new Temp;
        factor->SetTemp(temp);
        
        assert(m_temp->Find(temp->Id(),0)==0);
        
        m_temp->Insert(temp);
        
        assert(m_temp->Find(temp->Id(),0)==1);
        
        /* cosnt table */
        dynamic_cast<ConstTableNum*>(m_const_num)->Insert(t.u.ival);
        
        dynamic_cast<ConstTableNum*>(m_const_num)->Find(t.u.ival,&c);
        
        m_ir->NewIREntry(IR::CONST,c->GetIdx(),0,factor->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Const,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<t.u.ival<<std::endl;
        return factor;
    }
    /* F->id */
    if(v==kToken_ID){
        
        factor = new FactorId(t.u.name);
        temp = new Temp;
        factor->SetTemp(temp);
        assert(m_temp->Find(temp->Id(),0)==0);
        
        m_temp->Insert(temp);
        
        assert(m_temp->Find(temp->Id(),0)==1);
        
        /* check id is defined already? */
        SymTabStackIter iter(m_stack);
        for(tab=iter.Next();tab;tab=iter.Next())
        {
            if(tab->Find(t.u.name,&sym)){
                break;
            }
        }
        if(tab){
            m_ir->NewIREntry(IR::MOV,sym->GetIdx(),0,factor->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Sym,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
            //std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<t.u.name<<std::endl;
        }
        else{
            //std::cout<<"symbol "<<t.u.name<<" not found!"<<std::endl;
            assert(0==1);
            return 0;
        }
        
        return factor;
    }
    /* F->(E) */
    if(v==kToken_LPAR)
    {
        expr = _ParseExpr();
        
        v = m_scanner->Next(&t);
        
        assert(v==kToken_RPAR);
        
        assert(expr!=0);
        
        factor = new FactorExpr(expr);
        
        temp = new Temp;
        
        factor->SetTemp(temp);
        
        assert(m_temp->Find(temp->Id(),0)==0);
        
        m_temp->Insert(temp);
        assert(m_temp->Find(temp->Id(),0)==1);
        
        m_ir->NewIREntry(IR::MOV,expr->GetTemp()->GetIdx(),0,factor->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,0,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
        //std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<expr->GetTemp()->Id()<<std::endl;
        return factor;
    }
    
    assert(0==1);
    return 0;
}
Trest* Parser::_ParseTrest(Factor* f)
{
    s32 v;
    Token t;
    Factor* factor;
    Trest* trest;
    Trest* n;
    Temp* temp;
    
    v = m_scanner->Next(&t);
    /* T'->*FT' T'->/FT' */
    if(v==kToken_MUL||v==kToken_DIV)
    {
        factor = _ParseFactor();
        trest = _ParseTrest(factor);
        if(v==kToken_MUL){
            n = new TrestMul(factor,trest);
            temp = new Temp;
            n->SetTemp(temp);
            
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
            
            m_ir->NewIREntry(IR::MUL,f->GetTemp()->GetIdx(),factor->GetTemp()->GetIdx(),n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
            //std::cout<<"MUL "<<f->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
        if(v==kToken_DIV){
            n = new TrestDiv(factor,trest);
            temp = new Temp;
            
            n->SetTemp(temp);
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
        
            m_ir->NewIREntry(IR::DIV,f->GetTemp()->GetIdx(),factor->GetTemp()->GetIdx(),n->GetTemp()->GetIdx(),IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,IREntry::kIREntry_Operand_Temp,m_stack->TopNode());
            //std::cout<<"DIV "<<f->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
    }
    
    // should push back
    if(v!=kToken_EOT)
        m_scanner->Back(&t);
    
    /* T'->empty */
    return 0;
    
}
Parser::~Parser()
{
   // temp allocator exit 
   Temp::Exit();
    
   Label::Exit();
   
   delete m_label;
   
   delete m_scanner;
   
   delete m_stack;
   
   delete m_temp;
   
   delete m_const_num;
   
   delete m_ir;
}
    
}// namespace parser

}// namespace tiger