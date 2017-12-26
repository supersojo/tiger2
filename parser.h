/* Coding: ANSI */
#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "symtable.h"
#include "temp.h"
#include "const.h"
#include "label.h"


namespace tiger{

class IR;

namespace parser{

/*
Tiger expression:

1) E -> TE'
2) E'-> +TE' | -TE' | e
3) T -> FT'
3) T'-> *FT' | /FT' | e
4) F -> int | (E)|id

statement:
id=expr|statement

block={statement block}

StatementIf:
if exp then
if_body

if_body:
{statementlist}

function f(arglist)
f_body
end

statmentfuncDes:
function f_name(arglist)
{}

end
*/

class Expr;

class Factor{
public:
    enum{
        kFactor_Num,
        kFactor_Id,
        kFactor_Expr,
        kFactor_Invalid
    }FactorType;
    Factor(){
        m_kind = (s32)kFactor_Invalid;
    }
    Factor(s32 kind){
        m_kind = kind;
    }
    Temp* GetTemp(){return m_tmp;}
    virtual s32 Kind(){
        return m_kind;
    }
    void SetTemp(Temp* tmp){m_tmp = tmp;}
private:
    s32 m_kind;
    Temp* m_tmp;/* each Non-terminal binding a temp */
};
class FactorNum:public Factor{
public:
    FactorNum(s32 ival):Factor(kFactor_Num){
        m_ival = ival;
    }
    s32 Data(){
        return m_ival;
    }
private:
    s32 m_ival;
};
class FactorId:public Factor{
public:
    FactorId(char* id):Factor(kFactor_Id){
        m_id = strdup(id);
    }
    char* Data(){
        return m_id;
    }
    ~FactorId(){
        free(m_id);
    }
private:
    char* m_id; 
};
class FactorExpr:public Factor{
public:
    FactorExpr(Expr* expr):Factor(kFactor_Expr){
        m_expr = expr;
    }
    Expr* Data(){
        return m_expr;
    }
    ~FactorExpr(){
        delete m_expr;
    }
private:
    Expr* m_expr; 
};
class Trest{
public:
    enum{
        kTrest_Mul,
        kTrest_Div,
        kTrest_None,
        kTrest_Invalid
    };
    Trest(){
        m_kind = kTrest_Invalid;
    }
    Trest(s32 kind){
        m_kind = kind;
    }
    virtual s32 Kind(){
        return m_kind;
    }
    Factor* GetFactor(){
        return m_factor;
    }
    Trest* GetTrest(){
        return m_trest;
    }
    void SetFactor(Factor* factor){
        m_factor = factor;
    }
    void SetTrest(Trest* trest){
        m_trest = trest;
    }
    void SetTemp(Temp* tmp){m_tmp = tmp;}
    Temp* GetTemp(){return m_tmp;}
    virtual ~Trest(){
        delete m_factor;
        delete m_trest;
    }
private:
    s32 m_kind;
    Factor* m_factor;
    Trest* m_trest;
    Temp* m_tmp;/* each Non-terminal binding a temp */
};
class TrestMul:public Trest{
public:
    TrestMul(Factor* factor,Trest* trest):Trest(kTrest_Mul){
        SetFactor(factor);
        SetTrest(trest);
    }
};
class TrestDiv:public Trest{
public:
    TrestDiv(Factor* factor,Trest* trest):Trest(kTrest_Div){
        SetFactor(factor);
        SetTrest(trest);
    }
};
class Term{
public:
    Term(Factor* factor,Trest* trest){
        m_factor = factor;
        m_trest = trest;
    }
    Factor* GetFactor(){return m_factor;}
    Trest*  GetTrest(){return m_trest;}
    void SetTemp(Temp* tmp){m_tmp = tmp;}
    Temp* GetTemp(){return m_tmp;}
    ~Term(){
        delete m_factor;
        delete m_trest;
    }
public:
    Factor* m_factor;
    Trest* m_trest;
    Temp* m_tmp;/* each Non-terminal binding a temp */
};
class Erest{
public:
    enum{
        kErest_Add,
        kErest_Sub,
        kErest_None,
        kErest_Invalid
    };
    Erest(){m_kind = kErest_Invalid;}
    Erest(s32 kind){m_kind=kind;}
    Term* GetTerm(){return m_term;}
    Erest* GetErest(){return m_erest;}
    void SetErest(Erest* erest){m_erest=erest;}
    void SetTerm(Term* term){m_term = term;}
    virtual s32 Kind(){return m_kind;}
    virtual ~Erest(){delete m_term;delete m_erest;}
    void SetTemp(Temp* tmp){m_tmp = tmp;}
    Temp* GetTemp(){return m_tmp;}
private:
    s32 m_kind;
    Term* m_term;
    Erest* m_erest;
    Temp* m_tmp;/* each Non-terminal binding a temp */
};
class ErestAdd:public Erest{
public:
    ErestAdd(Term* term,Erest* erest):Erest(kErest_Add){
        SetTerm(term);
        SetErest(erest);
    }
};
class ErestSub:public Erest{
public:
    ErestSub(Term* term,Erest* erest):Erest(kErest_Sub){
        SetTerm(term);
        SetErest(erest);
    }
};
class Expr{
public:
    Expr(Term* term,Erest* erest){
        m_term = term;
        m_erest = erest;
    }
    void SetTemp(Temp* tmp){m_tmp = tmp;}
    Temp* GetTemp(){return m_tmp;}
    ~Expr(){delete m_term;delete m_erest;}
private:
    Term* m_term;
    Erest* m_erest;
    Temp* m_tmp;/* each Non-terminal binding a temp */
};
class Statement{
public:
    enum{
        kStatement_Assign,
        kStatement_Block,
        kStatement_If,
        kStatement_FunctionDeclaration,
        kStatement_Invalid
    };
    Statement(){m_kind = kStatement_Invalid;}
    Statement(s32 kind){m_kind = kind;}
private:
    s32 m_kind;
    
};
class StatementList{
friend class Parser;
public:
    StatementList(){m_statement = 0;m_next = 0;}
    StatementList(Statement* statement){m_statement = statement;m_next = 0;}
    ~StatementList(){
        if(m_statement)
            delete m_statement;
    }
private:
    Statement* m_statement;
    StatementList* m_next;
};
class StatementAssign:public Statement{
public:
    StatementAssign():Statement(kStatement_Assign){}
    StatementAssign(char* id,Expr* expr):Statement(kStatement_Assign){
        m_id = strdup(id);
        m_expr = expr;
        
    }
    char* GetId(){return m_id;}
    Expr* GetExpr(){return m_expr;}
    ~StatementAssign(){
        free(m_id);
        delete m_expr;
    }
private:
    char* m_id;
    Expr* m_expr;
};
class StatementBlock:public Statement{
public:
    StatementBlock():Statement(kStatement_Block){m_statementList=0;}
    StatementBlock(StatementList* statementList):Statement(kStatement_Block){m_statementList=statementList;}
    ~StatementBlock(){delete m_statementList;}
private:
    StatementList* m_statementList;
};
class StatementIf:public Statement{
public:
    StatementIf():Statement(kStatement_If){
        }
    StatementIf(Expr* expr,Statement* statement){
        m_expr = expr;
        m_statement = statement;
    }
    ~StatementIf(){
        delete m_expr;
        delete m_statement;
    }
private:
    Expr* m_expr;
    Statement* m_statement;  
};
class ArgList{
public:
    ArgList();
private:
    char* m_id;
    ArgList* m_next;
};
class StatementFunctionDeclaration:public Statement{
public:
    StatementFunctionDeclaration():Statement(kStatement_FunctionDeclaration){
        m_name=0;
        m_arg_list=0;
        m_body=0;
    }
    StatementFunctionDeclaration(char* name,ArgList* list,Statement* body){
        m_name = strdup(name);
        m_arg_list = list;
        m_body = body;
    }
    ~StatementFunctionDeclaration(){
        free(m_name);
        if(m_arg_list) delete m_arg_list;
        if(m_body) delete m_body;
    }
private:
    char* m_name;
    ArgList* m_arg_list;
    Statement* m_body;
};

class Parser{
public:
    Parser(scanner::SourceCodeStreamBase* stream);
    s32 Parse();
    ConstTable* GetConstTable(){return m_const_num;}
    TempTable*  GetTempTable(){return m_temp;}
    LabelTable* GetLabelTable(){return m_label;}
    SymTabStack* GetSymTabStack(){return m_stack;}
    IR* GetIR(){return m_ir;}
    ~Parser();
private:
    void Init();
    Expr* _ParseExpr();
    Erest* _ParseErest(Term* term);
    Term* _ParseTerm();
    Trest* _ParseTrest(Factor* factor);
    Factor* _ParseFactor();
    Statement* _ParseStatement();
    StatementList* _ParseStatementList();
    
    scanner::Scanner *m_scanner;
    
    SymTabStack* m_stack;
    
    /* temp related */
    TempTable* m_temp;
    
    ConstTable* m_const_num;
    ConstTable* m_const_str;
    
    /* label related */
    LabelTable* m_label;
    
    /* ir related */
    IR* m_ir;

};
    
} // namespace parser

} // namespace tiger 

#endif
