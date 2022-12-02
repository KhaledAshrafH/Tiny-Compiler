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

bool isReservedWord(string str,vector<pair<Token, int>>& result,int lineNum,const InFile& file,const OutFile& fileOut){
    if(str=="read"){
        writeToken(reserved_words[6],result,file.cur_line_num,file,fileOut,"read");
        return true;
    }

    if(str=="write") {
        writeToken(reserved_words[7], result, file.cur_line_num, file, fileOut, "write");
        return true;
    }

    if(str=="if"){
        writeToken(reserved_words[0],result,file.cur_line_num,file,fileOut,"if");
        return true;
    }

    if(str=="else"){
        writeToken(reserved_words[2],result,file.cur_line_num,file,fileOut,"else");
        return true;
    }

    if(str=="then"){
        writeToken(reserved_words[1],result,file.cur_line_num,file,fileOut,"then");
        return true;
    }

    if(str=="end"){
        writeToken(reserved_words[3],result,file.cur_line_num,file,fileOut,"end");
        return true;
    }

    if(str=="until"){
        writeToken(reserved_words[5],result,file.cur_line_num,file,fileOut,"until");
        return true;
    }

    else if(str=="repeat"){
        writeToken(reserved_words[4],result,file.cur_line_num,file,fileOut,"repeat");
        return true;
    }
    return false;
}
void Scanner(InFile& file,OutFile& fileOut){
    file.GetNewLine();
    //stack<char> bracketsCheck;
    vector<pair<Token, int>> result;//extra
    cout<<"\nScanning Running Now.";
    bool checkLoop=true;int lastLine=0;
    while (checkLoop) {
        this_thread::sleep_for(chrono::milliseconds(100));cout<<'.'; //For printing
        endingPoint0: // If we reach the end of the file
        string str = file.GetNextTokenStr(),left="",right="",op="";;
        file.SkipSpaces();
        bool checkSemi=false;
        for (int i = 0; i < str.size(); i++) {
            string tempStr;
            // if the current character is '{'
            if (str[i] == '{') {
                writeToken(symbolic_tokens[11],result,file.cur_line_num,file,fileOut,"{");
                file.SkipUpto("}");
                i++;
                writeToken(symbolic_tokens[12],result,file.cur_line_num,file,fileOut,"}");
                break;
            }
            else{
                tempStr = "";
                while (str[i] != ' ' || str[i] !='\n') {
                    tempStr += str[i++];
                    if(tempStr[tempStr.size()-1]==';' || tempStr[tempStr.size()-1]==' ' || tempStr[tempStr.size()-1]=='\n' ) break;
                }
                i--;
                if(tempStr[tempStr.size()-1]==';') {
                    checkSemi=true;
                    tempStr.pop_back();
                }

                if(tempStr[tempStr.size()-1]==' ' || tempStr[tempStr.size()-1]=='\n')
                    tempStr.pop_back();

                if(!isReservedWord(tempStr,result,file.cur_line_num,file,fileOut)){
                    endingPoint:
                    string type;
                    //check type of characters of tempStr as string...
                    if(tempStr[0]>='0' && tempStr[0]<='9') type="num"; // is number
                    else if((tempStr[0]>='a'&&tempStr[0]<='z') || (tempStr[0]>='A'&&tempStr[0]<='Z')) type="id"; // is variable
                    else type="operation"; // is operation or error

                    // if characters is operation or error
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
                        else if((tempStr!="\n" && tempStr!=" " && tempStr!=";" && tempStr!="")|| (!IsDigit(tempStr[0]) && !IsLetterOrUnderscore(tempStr[0]) && (tempStr!="\n" && tempStr!=" " && tempStr!=";" && tempStr!=""))){
                            writeToken(24,result,file.cur_line_num,file,fileOut,tempStr);
                        }
                    }
                    //if characters is number
                    else if(type=="num"){
                        left="";right="";op="";
                        int j=0;
                        // loop to get all contiguous numbers and store it in left variable
                        for(;j<tempStr.size();j++){
                            if((tempStr[j]>='0' && tempStr[j]<='9')) left+=tempStr[j];
                            else break;
                        }
                        //-----------------------------------------------write left that stored sequence of numbers------------------
                        writeToken(22,result,file.cur_line_num,file,fileOut,left);
                        // if tempStr not number only !
                        if(tempStr.size()!=left.size()){
                            //-----------------------------------------------check operation exist-----------------------------------
                            for(;j<tempStr.size();j++){
                                while (tempStr[j]==' ') continue; // to ignore the spaces
                                // if operation exist after number ... check type of it
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
                                    else if(op.size()==1 && op==":" &&  tempStr[j+1]!='=' && tempStr[j+1]!=' '){
                                        writeToken(24,result,file.cur_line_num,file,fileOut, op);op="";
                                    }
                                }
                                else if(IsDigit(tempStr[j]) || IsLetterOrUnderscore(tempStr[j])) break; // if the character number or identifier
                                else {
                                    string strSym="";strSym+=tempStr[j];
                                    writeToken(24,result,file.cur_line_num,file,fileOut, strSym);
                                }
                            }
                            //store the rest of string to check it after collect the left part
                            for(;j<tempStr.size();j++)
                                right+=tempStr[j];
                            tempStr=right;
                            goto endingPoint;
                        }
                    }
                    //if characters is identifier
                    else {
                        left="";right="";op="";
                        int j=0;
                        // loop to get all contiguous characters and store it in left variable
                        for(;j<tempStr.size();j++){
                            if((tempStr[j]>='A' && tempStr[j]<='Z') || (tempStr[j]>='a' && tempStr[j]<='z')) left+=tempStr[j];
                            else break;
                        }
                        // check type of identifier is variable or reservedWord
                        if(!isReservedWord(left,result,file.cur_line_num,file,fileOut))
                            writeToken(21,result,file.cur_line_num,file,fileOut,left);

                        // if tempStr not variable (or reservedWord) only !
                        if(tempStr.size()!=left.size()){
                            // if operation exist after variable or reservedWord ... check type of it
                            for(;j<tempStr.size();j++){
                                while (tempStr[j]==' ') continue; // ignore the spaces
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
                                    else if(op.size()==1 && op==":" &&  tempStr[j+1]!='=' && tempStr[j+1]!=' '){
                                        writeToken(24,result,file.cur_line_num,file,fileOut, op);op="";
                                    }
                                }
                                else if(IsDigit(tempStr[j]) || IsLetterOrUnderscore(tempStr[j])) break;
                                else {
                                    string strSym="";strSym+=tempStr[j];
                                    writeToken(24,result,file.cur_line_num,file,fileOut, strSym);
                                }
                            }
                            //store the rest of string to check it after collect the left part
                            for(;j<tempStr.size();j++)
                                right+=tempStr[j];
                            tempStr=right;
                            goto endingPoint;
                        }
                    }
                }
                // if the line ended by semicolon...
                if(checkSemi){
                    writeToken(symbolic_tokens[8],result,file.cur_line_num,file,fileOut,";");
                    checkSemi=false; // to check this only one
                }
            }
        }
        file.SkipSpaces();
        file.GetNewLine();
        //this to check the end line by include it to checking and get tokens from it...
        if(feof(file.getFile()) && checkLoop) {
            lastLine=file.cur_line_num; // to store the end line to write it after checking
            checkLoop=false; // to break from while loop after checking
            goto endingPoint0; // only one going
        }
    }
    writeToken(23,result,lastLine,file,fileOut,TokenTypeStr[23]); // write token of last line
    cout<<"\nScanning Process Completed Successfully."<<endl;
}


int main() {
    InFile file(
            R"(D:\My Home\Courses\#My Faculty\Fourth Level\First Semester\Compilers\Assignments\Assignment 1\CompilersTask_1_Scanner\input.txt)");
    OutFile fileOut(
            R"(D:\My Home\Courses\#My Faculty\Fourth Level\First Semester\Compilers\Assignments\Assignment 1\CompilersTask_1_Scanner\output.txt)");
    Scanner(file,fileOut);
    return 0;
}
