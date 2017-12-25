#include <string.h>
#include <stdlib.h>

#include "label.h"

namespace tiger{

char* Label::Label_Prefix=0;
s32 Label::Label_Next_Id=0;
char* Label::Label_Buf=0;
s32 Label::Label_Max_Len = 0;

Label::Label(){
    sprintf(Label_Buf,"%s%04d",Label_Prefix,Label_Next_Id);
    m_id = strdup(Label_Buf);
    m_pos = 0;
}
Label::~Label(){
    free(m_id);
}
/* L0000 L0001 ...*/
void Label::Init(char* prefix,s32 max_len){
    if(prefix)
        Label_Prefix = strdup(prefix);
    else
        Label_Prefix = strdup("L");
    if(max_len)
        Label_Max_Len = max_len;
    else
        Label_Max_Len = kLabel_Max_Len;
    
    assert((strlen(Label_Prefix)+4)<Label_Max_Len);
    
    Label_Buf = new char[Label_Max_Len];
    memset(Label_Buf,0,Label_Max_Len);
    
    Label_Next_Id = 0;
    
}

void Label::Exit(){
    delete[] Label_Buf;
    
    if(Label_Prefix)
        free(Label_Prefix);
    
}

}// namespace tiger