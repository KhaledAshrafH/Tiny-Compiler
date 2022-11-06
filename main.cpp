#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bits/stdc++.h>
#define endl '\n'
using namespace std;


/*{ Sample program
  in TINY language
  compute factorial
}

read x; {input an integer}
if 0<x then {compute only if x>=1}
  fact:=1;
  repeat
    fact := fact * x;
    x:=x-1
  until x=0;
  write fact {output factorial}
end
*/

// sequence of statements separated by ;
// no procedures - no declarations
// all variables are integers
// variables are declared simply by assigning values to them :=
// variable names can include only alphabetic charachters(a:z or A:Z) and underscores
// if-statement: if (boolean) then [else] end
// repeat-statement: repeat until (boolean)
// boolean only in if and repeat conditions < = and two mathematical expressions
// math expressions integers only, + - * / ^
// I/O read write
// Comments {}

////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=strlen(b);
    return strncmp(a, b, nb)==0;
}

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
    if(b==nullptr) {*a=nullptr; return;}
    int n=strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
    FILE* file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH]{};
    int cur_ind, cur_line_size;

    explicit InFile(const char* str) {file=nullptr; if(str) file=fopen(str, "r"); cur_line_size=0; cur_ind=0; cur_line_num=0;}
    ~InFile(){if(file) fclose(file);}



    void SkipSpaces()
    {
        while(cur_ind<cur_line_size)
        {
            char ch=line_buf[cur_ind];
            if(ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char* str)
    {
        while(true)
        {
            SkipSpaces();
            while(cur_ind>=cur_line_size) {if(!GetNewLine()) return false; SkipSpaces();}

            if(StartsWith(&line_buf[cur_ind], str))
            {
                cur_ind+=strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=strlen(line_buf);
        if(cur_line_size==0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char* GetNextTokenStr()
    {
        SkipSpaces();
        while(cur_ind>=cur_line_size) {if(!GetNewLine()) return nullptr; SkipSpaces();}
        return &line_buf[cur_ind];
    }

    void Advance(int num)
    {
        cur_ind+=num;
    }

    [[nodiscard]] FILE* getFile() const{
        return file;
    }
};

struct OutFile
{
    FILE* file;
    explicit OutFile(const char* str) {file=nullptr; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s) const
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
                : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType{
                IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
                ASSIGN, EQUAL, LESS_THAN,
                PLUS, MINUS, TIMES, DIVIDE, POWER,
                SEMI_COLON,
                LEFT_PAREN, RIGHT_PAREN,
                LEFT_BRACE, RIGHT_BRACE,
                ID, NUM,
                ENDFILE, ERROR
              };

// Used for debugging only /////////////////////////////////////////////////////////
const char* TokenTypeStr[]=
            {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",//21 ,22
                "EndFile", "Error" //23, 24
            };

struct Token
{
    TokenType type;
    char str[MAX_TOKEN_LEN+1]{};

    Token(){str[0]=0; type=ERROR;}
    Token(TokenType _type, const char* _str) {type=_type; Copy(str, _str);}
};

const Token reserved_words[]=
{
    Token(IF, "if"),
    Token(THEN, "then"),
    Token(ELSE, "else"),
    Token(END, "end"),
    Token(REPEAT, "repeat"),
    Token(UNTIL, "until"),
    Token(READ, "read"),
    Token(WRITE, "write")
};
const int num_reserved_words=sizeof(reserved_words)/sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[]=
{
    Token(ASSIGN, ":="),
    Token(EQUAL, "="),
    Token(LESS_THAN, "<"),
    Token(PLUS, "+"),
    Token(MINUS, "-"),
    Token(TIMES, "*"),
    Token(DIVIDE, "/"),
    Token(POWER, "^"),
    Token(SEMI_COLON, ";"),
    Token(LEFT_PAREN, "("),
    Token(RIGHT_PAREN, ")"),
    Token(LEFT_BRACE, "{"),
    Token(RIGHT_BRACE, "}")
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch){return (ch>='0' && ch<='9');}
inline bool IsLetter(char ch){return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));}
inline bool IsLetterOrUnderscore(char ch){return (IsLetter(ch) || ch=='_');}

void writeToken(Token token,vector<pair<Token, int>>& result,int lineNum,const InFile& file,const OutFile& fileOut,const string& specificTokenStr){
    result.emplace_back(token,file.cur_line_num);
    string strTemp= "[" + to_string(lineNum) + "] " + specificTokenStr + " (" + TokenTypeStr[token.type] + ")";
    int n = strTemp.length();
    char finalStr[n + 1];
    strcpy(finalStr, strTemp.c_str());
    fileOut.Out(finalStr);
}

void writeToken(int tokenIdx,vector<pair<Token, int>>& result,int lineNum,const InFile& file,const OutFile& fileOut,const string& specificTokenStr){
//    result.emplace_back(token,file.cur_line_num);
    string strTemp= "[" + to_string(lineNum) + "] " + specificTokenStr + " (" + TokenTypeStr[tokenIdx] + ")";
    int n = strTemp.length();
    char finalStr[n + 1];
    strcpy(finalStr, strTemp.c_str());
    fileOut.Out(finalStr);
}

int main() {
    InFile file(
            R"(D:\My Home\Courses\#My Faculty\Fourth Level\First Semester\Compilers\Assignments\Assignment 1\CompilersTask_1_Scanner\input.txt)");
    OutFile fileOut(R"(D:\My Home\Courses\#My Faculty\Fourth Level\First Semester\Compilers\Assignments\Assignment 1\CompilersTask_1_Scanner\output.txt)");
    file.GetNewLine();
    stack<char> bracketsCheck;
    vector<pair<Token, int>> result;//extra
    cout<<"\nScanning Running Now.";
    while (true) {
        //this_thread::sleep_for(chrono::milliseconds(100));
        //cout<<'.';
        string str = file.GetNextTokenStr(),left="",right="",op="";;
        file.SkipSpaces();
        bool checkSemi=false;
        for (int i = 0; i < str.size(); i++) {
            string tempStr;
            if (str[i] == '{') {
                //bracketsCheck.push('{');
                writeToken(symbolic_tokens[11],result,file.cur_line_num,file,fileOut,"{");
                file.SkipUpto("}");
                //bracketsCheck.pop();
                i++;
                writeToken(symbolic_tokens[12],result,file.cur_line_num,file,fileOut,"}");
                break;
            }
//            else if (!bracketsCheck.empty()) {
//
////                if (str[i] == '}') {
////                    bracketsCheck.pop();
//////                    i++;
//////                    writeToken(symbolic_tokens[12],result,file.cur_line_num,file,fileOut,"}");
////                }
//                continue;
//            }
            else{
                tempStr = "";
                while (str[i] != ' ' || str[i] !='\n') {
                    tempStr += str[i++];
                    if(tempStr[tempStr.size()-1]==';' || tempStr[tempStr.size()-1]==' ' || tempStr[tempStr.size()-1]=='\n' ) break;
                }
                i--;
                if(tempStr[tempStr.size()-1]==';') {
                    checkSemi=true;tempStr.pop_back();
                }
                if(tempStr[tempStr.size()-1]==' ' || tempStr[tempStr.size()-1]=='\n'){
                    tempStr.pop_back();
                }

                if(tempStr=="read") {
                    writeToken(reserved_words[6],result,file.cur_line_num,file,fileOut,"read");
                    //-------------------------------------variable read---------------------------
//                    tempStr="";
//                    i++;
//                    while (str[i]!=';') {
//                        tempStr += str[i++];
//                    }
//                    i--;
//                    writeToken(21,result,file.cur_line_num,file,fileOut,tempStr);
//                    Token newToken(ID,finalStr2);
//                    result.emplace_back(newToken,file.cur_ind);
                }
                else if(tempStr=="write"){
                    writeToken(reserved_words[7],result,file.cur_line_num,file,fileOut,"write");
                    //-------------------------------------variable write---------------------------
//                    tempStr="";
//                    i++;
//                    while (str[i]!=';') {
//                        tempStr += str[i++];
//                    }
//                    i--;
//                    writeToken(21,result,file.cur_line_num,file,fileOut,tempStr);
//                    Token newToken(ID,finalStr2);
//                    result.emplace_back(newToken,file.cur_ind);
                }
                else if(tempStr=="if"){
                    writeToken(reserved_words[0],result,file.cur_line_num,file,fileOut,"if");
                }
                else if(tempStr=="else"){
                    writeToken(reserved_words[2],result,file.cur_line_num,file,fileOut,"else");
                }
                else if(tempStr=="then"){
                    writeToken(reserved_words[1],result,file.cur_line_num,file,fileOut,"then");
                }
                else if(tempStr=="end"){
                    writeToken(reserved_words[3],result,file.cur_line_num,file,fileOut,"end");
                }
                else if(tempStr=="until"){
                    writeToken(reserved_words[5],result,file.cur_line_num,file,fileOut,"until");
                }
                else if(tempStr=="repeat"){
                    writeToken(reserved_words[4],result,file.cur_line_num,file,fileOut,"repeat");
                }
                else {
                    endingPoint:
                    string type;
                    if(tempStr[0]>='0' && tempStr[0]<='9') type="num";
                    else if((tempStr[0]>='a'&&tempStr[0]<='z') || (tempStr[0]>='A'&&tempStr[0]<='Z')) type="id";
                    else type="operation";

                    if(type=="operation"){

                        if(tempStr=="+"){
                            writeToken(symbolic_tokens[3],result,file.cur_line_num,file,fileOut,"+");
                        }
                        else if(tempStr=="*"){
                            writeToken(symbolic_tokens[5],result,file.cur_line_num,file,fileOut,"*");
                        }
                        else if(tempStr=="-"){
                            writeToken(symbolic_tokens[4],result,file.cur_line_num,file,fileOut,"-");
                        }
                        else if(tempStr=="/"){
                            writeToken(symbolic_tokens[6],result,file.cur_line_num,file,fileOut,"/");
                        }
                        else if(tempStr==":="){
                            writeToken(symbolic_tokens[0],result,file.cur_line_num,file,fileOut,":=");
                        }
                        else if(tempStr=="="){
                            writeToken(symbolic_tokens[1],result,file.cur_line_num,file,fileOut,"=");
                        }
                        else if(tempStr=="<"){
                            writeToken(symbolic_tokens[2],result,file.cur_line_num,file,fileOut,"<");
                        }
                        else if(tempStr=="^"){
                            writeToken(symbolic_tokens[7],result,file.cur_line_num,file,fileOut,"^");
                        }
                        else if(tempStr==";"){
                            writeToken(symbolic_tokens[8],result,file.cur_line_num,file,fileOut,";");
                        }
                        else if(tempStr=="("){
                            writeToken(symbolic_tokens[9],result,file.cur_line_num,file,fileOut,"(");
                        }
                        else if(tempStr==")"){
                            writeToken(symbolic_tokens[10],result,file.cur_line_num,file,fileOut,")");
                        }
                        else if(tempStr!="\n" && tempStr!=" " && tempStr!=";" && tempStr!=""){
                            writeToken(24,result,file.cur_line_num,file,fileOut,tempStr);
                        }

//                        if(tempStr[]&&!tempStr.empty()) str=tempStr.substr(1);
                    }
                    //if left hand side is number
                    else if(type=="num"){
                        left="";right="";op="";
                        int j=0;
                        for(;j<tempStr.size();j++){
                            if((tempStr[j]>='0' && tempStr[j]<='9')) left+=tempStr[j];
                            else {break;}
                        }
                        //-----------------------------------------------left-----------------------------------------------
                        writeToken(22,result,file.cur_line_num,file,fileOut,left);

                        if(tempStr.size()!=left.size()){
                            //-----------------------------------------------op-----------------------------------------------
                            for(;j<tempStr.size();j++){
                                if((tempStr[j]>=':' && tempStr[j]<='>') || (tempStr[j]>='(' && tempStr[j]<='/') || (tempStr[j]=='^')) {
                                    op+=tempStr[j];

                                    if(op=="+"){
                                        writeToken(symbolic_tokens[3],result,file.cur_line_num,file,fileOut,"+");op="";
                                    }
                                    else if(op=="*"){
                                        writeToken(symbolic_tokens[5],result,file.cur_line_num,file,fileOut,"*");op="";
                                    }
                                    else if(op=="-"){
                                        writeToken(symbolic_tokens[4],result,file.cur_line_num,file,fileOut,"-");op="";
                                    }
                                    else if(op=="/"){
                                        writeToken(symbolic_tokens[6],result,file.cur_line_num,file,fileOut,"/");op="";
                                    }
                                    else if(op.size()==2 && op[0]==':' && op[1]=='='){
                                        writeToken(symbolic_tokens[0],result,file.cur_line_num,file,fileOut,":=");op="";
                                    }
                                    else if(op=="="){
                                        writeToken(symbolic_tokens[1],result,file.cur_line_num,file,fileOut,"=");op="";
                                    }
                                    else if(op=="<"){
                                        writeToken(symbolic_tokens[2],result,file.cur_line_num,file,fileOut,"<");op="";
                                    }
                                    else if(op=="^"){
                                        writeToken(symbolic_tokens[7],result,file.cur_line_num,file,fileOut,"^");op="";
                                    }
                                    else if(op==";"){
                                        writeToken(symbolic_tokens[8],result,file.cur_line_num,file,fileOut,";");op="";
                                    }
                                    else if(op=="("){
                                        writeToken(symbolic_tokens[9],result,file.cur_line_num,file,fileOut,"(");op="";
                                    }
                                    else if(op==")"){
                                        writeToken(symbolic_tokens[10],result,file.cur_line_num,file,fileOut,")");op="";
                                    }
                                }
                                else if(IsDigit(tempStr[j]) || IsLetterOrUnderscore(tempStr[j])) break;
                                else {
                                    string strSym="";strSym+=tempStr[j];
                                    writeToken(24,result,file.cur_line_num,file,fileOut, strSym);
                                }
                            }
                            //-----------------------------------------------right-----------------------------------------------
                            for(;j<tempStr.size();j++){
                                right+=tempStr[j];
                            }
                            tempStr=right;
                            goto endingPoint;
                        }
                    }
                    //if left hand side is variable
                    else {
                        left="";right="";op="";
                        int j=0;
                        for(;j<tempStr.size();j++){
                            if((tempStr[j]>='A' && tempStr[j]<='Z') || (tempStr[j]>='a' && tempStr[j]<='z')) left+=tempStr[j];
                            else {break;}
                        }
                        writeToken(21,result,file.cur_line_num,file,fileOut,left);

                        cout<<left<<endl<<tempStr[j]<<endl;
                        if(tempStr.size()!=left.size()){
                            //-----------------------------------------------op-----------------------------------------------
                            for(;j<tempStr.size();j++){
                                if((tempStr[j]>=':' && tempStr[j]<='>') || (tempStr[j]>='(' && tempStr[j]<='/') || (tempStr[j]=='^')) {
                                    op+=tempStr[j];

                                    if(op=="+"){
                                        writeToken(symbolic_tokens[3],result,file.cur_line_num,file,fileOut,"+");op="";
                                    }
                                    else if(op=="*"){
                                        writeToken(symbolic_tokens[5],result,file.cur_line_num,file,fileOut,"*");op="";
                                    }
                                    else if(op=="-"){
                                        writeToken(symbolic_tokens[4],result,file.cur_line_num,file,fileOut,"-");op="";
                                    }
                                    else if(op=="/"){
                                        writeToken(symbolic_tokens[6],result,file.cur_line_num,file,fileOut,"/");op="";
                                    }
                                    else if(op.size()==2 && op[0]==':' && op[1]=='='){
                                        writeToken(symbolic_tokens[0],result,file.cur_line_num,file,fileOut,":=");op="";
                                    }
                                    else if(op=="="){
                                        writeToken(symbolic_tokens[1],result,file.cur_line_num,file,fileOut,"=");op="";
                                    }
                                    else if(op=="<"){
                                        writeToken(symbolic_tokens[2],result,file.cur_line_num,file,fileOut,"<");op="";
                                    }
                                    else if(op=="^"){
                                        writeToken(symbolic_tokens[7],result,file.cur_line_num,file,fileOut,"^");op="";
                                    }
                                    else if(op==";"){
                                        writeToken(symbolic_tokens[8],result,file.cur_line_num,file,fileOut,";");op="";
                                    }
                                    else if(op=="("){
                                        writeToken(symbolic_tokens[9],result,file.cur_line_num,file,fileOut,"(");op="";
                                    }
                                    else if(op==")"){
                                        writeToken(symbolic_tokens[10],result,file.cur_line_num,file,fileOut,")");op="";
                                    }
                                }
                                else if(IsDigit(tempStr[j]) || IsLetterOrUnderscore(tempStr[j])) break;
                                else {
                                    string strSym="";strSym+=tempStr[j];
                                    writeToken(24,result,file.cur_line_num,file,fileOut, strSym);
                                }
                            }
                            //-----------------------------------------------right-----------------------------------------------
                            for(;j<tempStr.size();j++){
                                right+=tempStr[j];
                            }
                            tempStr=right;
                            goto endingPoint;
                        }
                    }
                }
                if(checkSemi){
                    writeToken(symbolic_tokens[8],result,file.cur_line_num,file,fileOut,";");
                    checkSemi=false;
                }
            }
        }
        file.SkipSpaces();
        file.GetNewLine();
        if(feof(file.getFile())) {writeToken(23,result,file.cur_line_num,file,fileOut,TokenTypeStr[23]);break;}
    }
    cout<<"\nScanning Process Completed Successfully."<<endl;
    return 0;
}
