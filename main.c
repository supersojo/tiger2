#include <iostream>

#include "scanner.h"
#include "parser.h"
#include "symtable.h"
#include "temp.h"
#include "const.h"

void test_StringSourceCodeStream()
{
    char *string="just a test";
    s32 len=strlen(string);
    tiger::scanner::StringSourceCodeStream stream("just a test");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_FileSourceCodeStream()
{
    char *string="just b test";
    s32 len=strlen(string);
    tiger::scanner::FileSourceCodeStream stream("a.txt");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_Next_With_StringSourceCodeStream()
{
    s32 v;
    tiger::Token t;
    tiger::scanner::StringSourceCodeStream stream("   \r\na b 111 a_ \r\na1 0 11 ( ) + for / function end ");
    tiger::scanner::Scanner scanner(&stream);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a")==0);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"b")==0);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==111);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a_")==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a1")==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==11);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_LPAR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_RPAR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ADD);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_FOR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_DIV);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_FUNCTION);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_END);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_EOT);
}
void test_Next_With_FileSourceCodeStream()
{
    s32 v;
    tiger::Token t;
    tiger::scanner::FileSourceCodeStream stream("a.txt");
    tiger::scanner::Scanner scanner(&stream);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a")==0);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"b")==0);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==111);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a_")==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"a1")==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==0);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_NUM);
    assert(t.u.ival==11);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_LPAR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_RPAR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ADD);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_FOR);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_DIV);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_FUNCTION);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_END);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_EOT);
}
void test_Parser()
{
    tiger::scanner::StringSourceCodeStream stream("a=1{a=2 b=3}b=1 ");
    tiger::parser::Parser parser(&stream);
    parser.Parse();
    
}
void test_SymbolTable()
{
    tiger::Symbol* sym;
    tiger::SymbolTable tab;
    tiger::Tval* v1=new tiger::TvalNum(10);//freed by caller
    tiger::Tval* v2=new tiger::TvalStr("aaaa");//freed by caller
    tiger::Tval* v3=new tiger::TvalStr("bbbb");//freed by caller
    
    tab.Insert("a");
    tab.Insert("b");
    
    assert(tab.Find("c",0)==false);
    
    tab.Find("a",&sym);
    sym->Bind(v1);
    
    tab.Find("b",&sym);
    sym->Bind(v2);
    
    tab.Find("a",&sym);
    std::cout<<sym->GetIdx()<<std::endl;
    if(sym->GetVal()->Kind()==tiger::Tval::kTval_Num)
        std::cout<<dynamic_cast<tiger::TvalNum*>(sym->GetVal())->Data()<<std::endl;
    tab.Find("b",&sym);
    std::cout<<sym->GetIdx()<<std::endl;
    sym->Bind(v3);
    if(sym->GetVal()->Kind()==tiger::Tval::kTval_Str)
        std::cout<<dynamic_cast<tiger::TvalStr*>(sym->GetVal())->Data()<<std::endl;
    std::cout<<sym->GetIdx()<<std::endl;
}
void test_SymTabStack()
{
    tiger::SymTabStack stack;
    assert(stack.Top()==0);
    
    tiger::SymbolTable tab;
    stack.Push(&tab);
    assert(stack.Size()==1);
    stack.Top()->Insert("a");
    assert(tab.Find("a",0));
    
    stack.Pop();
    assert(stack.Size()==0);
    
    stack.Push(&tab);
    stack.Push(&tab);
    assert(stack.Size()==2);
}
void test_Temp()
{
    tiger::Temp::Init("TMP",32);
    
    tiger::Temp temp,temp1,temp2;
    std::cout<<temp.Id()<<std::endl;
    std::cout<<temp1.Id()<<std::endl;
    std::cout<<temp2.Id()<<std::endl;
    tiger::Temp::Exit();
}
void test_Const()
{
    tiger::Const* c;
    tiger::ConstTableNum ctable;
    assert(ctable.Find(1234,&c)==0);
    ctable.Insert(1234);
    assert(ctable.Find(1234,&c)==1);
    assert(ctable.FindByIdx(c->GetIdx(),0)==1);
}
void test_TempTable()
{
    tiger::Temp* t;
    tiger::TempTable tab;
    
    tiger::Temp::Init("TMP",32);
    
    assert(tab.Find("a",0)==0);
    t = new tiger::Temp;
    tab.Insert(t);
    assert(tab.Find(t->Id(),0)==1);
    
    tiger::Temp::Exit();
    
}
int main()
{
    //test_StringSourceCodeStream();
    //test_FileSourceCodeStream();
    //test_Next_With_StringSourceCodeStream();
    //test_Next_With_FileSourceCodeStream();
    test_Parser();
    //test_SymbolTable();
    //test_SymTabStack();
    //test_Temp();
    //test_Const();
    //test_TempTable();
    return 0;
}