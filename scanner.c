#include <string.h>
#include <stdio.h>
#include <iostream>

#include "scanner.h"

namespace tiger {
    
namespace scanner {

StringSourceCodeStream::StringSourceCodeStream(char* source)
{
    m_string = source;
    m_pos = 0;
    m_len = strlen(source);
    m_off = 0;
    m_lineno = 1;/* always start from 1 */
}
s32 StringSourceCodeStream::Next()
{
    s32 ret = *(m_string + m_pos);
    if(m_pos<=(m_len-1)){
        m_pos++;
        m_off++;
        return ret;
    }        
    else
        return kSourceCodeStream_EOS;
}
void StringSourceCodeStream::Back(s32 n)
{
    assert(n<=m_pos &&n>=(m_pos-m_len+1));
    m_pos = m_pos-n;
    m_off = m_off-n;
    if(m_off<0){
        m_lineno = m_lineno-1;
        m_off = m_off_prev;
    }
}
s32 StringSourceCodeStream::Pos()
{
    return m_off;
}
/*
 Note: 
 open file with flag "rb", which will open file in binary mode.
 Or else, when "\r\n" occur, fread only return "\n" on windows platform.
*/
FileSourceCodeStream::FileSourceCodeStream(char* file)
{
    FILE *fp = fopen(file,"rb");
    
    assert(fp!=0);
    
    m_file = fp;
    
    fseek(m_file,0,SEEK_END);
    m_len = ftell(m_file);
    
    fseek(m_file,0,SEEK_SET);
    
    m_off = 0;
    m_lineno = 1;/* always start from 1 */
    
}
FileSourceCodeStream::~FileSourceCodeStream()
{
    fclose(m_file);
}
s32 FileSourceCodeStream::Next()
{
    s32 v = 0;
    s32 ret = fread(&v,1,1,m_file);
    if(ret==0)/* read error or eof */
        return kSourceCodeStream_EOS;
    else{
        m_off++;
        return v;
    }
}
void FileSourceCodeStream::Back(s32 n)
{
    s32 pos = ftell(m_file);
    
    assert(n<=pos &&n>=(pos-m_len+1));
    
    pos = pos-n;
    m_off = m_off - n;
    
    if(m_off<0){
        m_lineno = m_lineno-1;
        m_off = m_off_prev;
    }
    
    fseek(m_file,pos,SEEK_SET);
}
s32 FileSourceCodeStream::Pos()
{
    return m_off;
}
Scanner::Scanner(SourceCodeStreamBase* stream)
{
    m_stream = stream;
    
    Init();
}
void Scanner::Init()
{

}

void Scanner::SkipSpace()
{
    s32 v;
    do{
        v = m_stream->Next();
    }while((char)v==' ' ||
           (char)v=='\t');
    if(v!=kSourceCodeStream_EOS){
        m_stream->Back(1);
    }
}
s32 Scanner::IsAlpha(s32 c)
{
    return (((char)c>='a' && (char)c<='z') ||
             ((char)c>='A' && (char)c<='Z'));
}
s32 Scanner::IsDigit(s32 c)
{
    return ((char)c>='0' && (char)c<='9');
}
void Scanner::Back(Token* t)
{
    m_stream->Back(t->len);
}
s32 Scanner::Next(Token* t)
{
    char sval[TOKEN_MAX_LEN];
    s32 v;
    s32 i = 0;
    
    assert(t!=0);
        
    //free token related memory
    t->Clear();
    
    SkipSpace();
    
    v = m_stream->Next();
    if((char)v=='\r'){
        v = m_stream->Next();
        if((char)v=='\n'){
            m_stream->NewLine();
        }else{
            if(v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
        }
    }
    else
    {
        if(v!=kSourceCodeStream_EOS)
            m_stream->Back(1);
    }
    
    v = m_stream->Next();
    
    t->lineno = m_stream->Lineno();
    t->pos = m_stream->Pos();
    
    t->len = 1;/* default length of string */
    
    /* id or keyword */
    if(IsAlpha(v)||
       (char)v=='_')
    {
        sval[i++]=(char)v;
        do{
            v = m_stream->Next();
            sval[i++]=(char)v;
        }while(IsAlpha(v) || 
               IsDigit(v) ||
               (char)v=='_');
        sval[i-1]='\0';
        
        //record the token string length, because we need back a token supporting
        t->len = i - 1 ;
        
        if(v!=kSourceCodeStream_EOS)
            m_stream->Back(1);
        
        if(token_is_keyword(sval))
            return keyword_type(sval);
        else{
            t->kind = kToken_ID;
            t->u.name=strdup(sval);/* Note: memory leak */
            return kToken_ID;
        }
    }
    /* num */
    if(IsDigit(v)){
        if((char)v=='0'){
            t->kind=kToken_NUM;
            t->u.ival = 0;
            return kToken_NUM;
        }else{/* 1~9 */
            sval[i++]=(char)v;
            do{
                v = m_stream->Next();
                sval[i++]=(char)v;
            }while(IsDigit(v));
            sval[i-1]='\0';
            
            //record the token string length, because we need back a token support
            t->len = i - 1;
            
            t->kind=kToken_NUM;
            t->u.ival = atoi(sval);
            if(v!=kSourceCodeStream_EOS)
                m_stream->Back(1);
            return kToken_NUM;
        }
    }
    /* ( */
    if((char)v=='(')
        return kToken_LPAR;
    /* ) */
    if((char)v==')')
        return kToken_RPAR;
    /* { */
    if((char)v=='{')
        return kToken_LBRA;
    /* } */
    if((char)v=='}')
        return kToken_RBRA;
    /* kToken_EOT */
    if(v==kSourceCodeStream_EOS){
        t->kind=kToken_EOT;
        return kToken_EOT;
    }
    /* + */
    if((char)v=='+')
        return kToken_ADD;
    /* - */
    if((char)v=='-')
        return kToken_SUB;
    /* * */
    if((char)v=='*')
        return kToken_MUL;
    /* / */
    if((char)v=='/')
        return kToken_DIV;
    /* = */
    if((char)v=='=')
        return kToken_ASSIGN;
    /* , */
    if((char)v==',')
        return kToken_COMMA;
    
    /* Known token */
    return kToken_Unknown;
}

} //namespace scanner
    
} // namespace tiger