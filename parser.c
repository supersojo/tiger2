#include <iostream>

#include "parser.h"
#include "symtable.h"

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
    
    m_const_num = new ConstTableNum;
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
    
    std::cout<<m_temp->Size()<<std::endl;
    m_temp->Dump();
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
    
    v = m_scanner->Next(&t);
    
    /* Statement->StatementAssign */
    if(v==kToken_ID){
        std::cout<<"id:"<<t.u.name<<std::endl;
        SymTabStackIter iter(m_stack);
        for(tab=iter.Next();tab;tab=iter.Next())
        {
            if(tab->Find(t.u.name,&sym)){
                break;
            }
        }
        /* new symbol */
        if(tab){
            std::cout<<t.u.name<<" already exist"<<std::endl;
        }else{
            std::cout<<"new symbol "<<t.u.name<<std::endl;
            m_stack->Top()->Insert(t.u.name);
            m_stack->Top()->Find(t.u.name,&sym);
        }
        
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_ASSIGN);
        expr = _ParseExpr();
        std::cout<<"MOV "<<t.u.name<<","<<expr->GetTemp()->Id()<<std::endl;
        std::cout<<"new assign statement"<<std::endl;
        return new StatementAssign(t.u.name,expr);
    }
    
    /* Statement->StatmentBlock */
    if(v==kToken_LBRA){
        /* new lexical level */
        SymbolTable* tab = new SymbolTable;
        m_stack->Push(tab);
        
        statementList = _ParseStatementList();
        v1 = m_scanner->Next(&t1);
        assert(v1==kToken_RBRA);
        
        /* back to previous level */
        m_stack->Pop();
        
        std::cout<<"new block statement"<<std::endl;
        return new StatementBlock(statementList);
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
    m_stack->Top()->Insert(n->GetTemp()->Id());
    m_stack->Top()->Find(n->GetTemp()->Id(),&sym);
    
    if(!erest)
        std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<std::endl;
    else
        std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<erest->GetTemp()->Id()<<std::endl;
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
            
    if(!trest)
        std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<std::endl;
    else
        std::cout<<"MOV "<<n->GetTemp()->Id()<<","<<trest->GetTemp()->Id()<<std::endl;
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
            
            std::cout<<"ADD "<<ter->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
        if(v==kToken_SUB){
            n = new ErestSub(term,erest);
            temp = new Temp;
            n->SetTemp(temp);
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
            std::cout<<"SUB "<<ter->GetTemp()->Id()<<","<<term->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
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
        assert(m_const_num->Find(t.u.ival,0)==0);
        m_const_num->Insert(t.u.ival);
        assert(m_const_num->Find(t.u.ival,0)==1);
        
        std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<t.u.ival<<std::endl;
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
        std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<t.u.name<<std::endl;
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
        
        std::cout<<"MOV "<<factor->GetTemp()->Id()<<","<<expr->GetTemp()->Id()<<std::endl;
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
            
            std::cout<<"MUL "<<f->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
            return n;
        }
        if(v==kToken_DIV){
            n = new TrestDiv(factor,trest);
            temp = new Temp;
            
            n->SetTemp(temp);
            assert(m_temp->Find(temp->Id(),0)==0);
        
            m_temp->Insert(temp);
            assert(m_temp->Find(temp->Id(),0)==1);
        
            std::cout<<"DIV "<<f->GetTemp()->Id()<<","<<factor->GetTemp()->Id()<<"->"<<n->GetTemp()->Id()<<std::endl;
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
    
   delete m_scanner;
   
   delete m_stack;
   
   delete m_temp;
   
   delete m_const_num;
}
    
}// namespace parser

}// namespace tiger