#include "temp.h"

namespace tiger{

char* Temp::Temp_Prefix = 0;
s32   Temp::Temp_Id = 0;
s32   Temp::Temp_Max_Len = 0;
s32   Temp::IsInitialized = 0;
char* Temp::Temp_Buf = 0;


void Temp::Init(char* prefix,int max_len){
        if(prefix)
            Temp_Prefix = strdup(prefix);
        else
            Temp_Prefix = strdup("TMP");
        
        Temp_Id = 0;
        
        
        if(max_len!=0)
            Temp_Max_Len = 32;
        
        assert(strlen(Temp_Prefix)+4<Temp_Max_Len);
        
        Temp_Buf = new char[Temp_Max_Len];
        
        IsInitialized = 1;
}
void Temp::Exit(){
        free(Temp_Prefix);
        delete Temp_Buf;
}

}// namespace tiger