/*
 Samah Mostafa Hussein Mahmoud => 20190248
 Khaled Ashraf Hanafy Mahmoud  => 20190186
 Ahmed Sayed Hassan Youssef    => 20190034
 Group:- 3,4
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <deque>
#define endl '\n'
const  int pathSize =1000;
using namespace std;


////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=(int)strlen(b);
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
    int n=(int)strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

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
                cur_ind+=(int)strlen(str);
                return true;
            }
            cur_ind++;
        }
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=(int)strlen(line_buf);
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
                "ID", "Num",
                "EndFile", "Error"
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

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->str[0]=0;

    int i;
    char* s=pci->in_file.GetNextTokenStr();
    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->str[0]=0;
        return;
    }

    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if(i<num_symbolic_tokens)
    {
        if(symbolic_tokens[i].type==LEFT_BRACE)
        {
            pci->in_file.Advance((int)strlen(symbolic_tokens[i].str));
            if(!pci->in_file.SkipUpto(symbolic_tokens[i+1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type=symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    }
    else if(IsDigit(s[0]))
    {
        int j=1;
        while(IsDigit(s[j])) j++;

        ptoken->type=NUM;
        Copy(ptoken->str, s, j);
    }
    else if(IsLetterOrUnderscore(s[0]))
    {
        int j=1;
        while(IsLetterOrUnderscore(s[j])) j++;

        ptoken->type=ID;
        Copy(ptoken->str, s, j);

        for(i=0;i<num_reserved_words;i++)
        {
            if(Equals(ptoken->str, reserved_words[i].str))
            {
                ptoken->type=reserved_words[i].type;
                break;
            }
        }
    }

    int len=(int)strlen(ptoken->str);
    if(len>0) pci->in_file.Advance(len);
}

// Parser //////////////////////////////////////////////////////////////////////////


enum NodeKind{
    IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
    OPER_NODE, NUM_NODE, ID_NODE
};

// Used for debugging only /////////////////////////////////////////////////////////
const char* NodeKindStr[]=
        {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
        };

enum ExprDataType {VOID, INTEGER, BOOLEAN};

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[]=
        {
                "Void", "Integer", "Boolean"
        };

#define MAX_CHILDREN 3

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN]{};//terminal
    TreeNode* sibling; // used for sibling statements only

    NodeKind node_kind;
    string id;
    union{TokenType oper; int num{}; }; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num{};

    TreeNode() {int i; for(i=0;i<MAX_CHILDREN;i++) child[i]=nullptr; sibling=nullptr; expr_data_type=VOID;}
};

struct ParseInfo
{
    Token next_token;
};

void PrintTree(TreeNode* node, int sh=0)
{
    int i, NSH=3;
    for(i=0;i<sh;i++) printf(" ");

    if(node!= nullptr) printf("[%s]", NodeKindStr[node->node_kind]);

    if(node!= nullptr&&node->node_kind==OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if(node!= nullptr&&node->node_kind==NUM_NODE) printf("[%d]", node->num);
    else if(node!= nullptr&&(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE)) printf("[%s]", node->id.c_str());

    if(node!= nullptr&&node->expr_data_type!=VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) PrintTree(node->child[i], sh+NSH);
    if(node->sibling) PrintTree(node->sibling, sh);
}


//global variables
deque<pair<TokenType,pair<int,string>>>Tokens; // to save each token with its value and line
pair<TokenType,pair<int,string>> CToken; // to know the current token
bool checkErrorsExist=false; // to check if errors exist in parsing phase

//Utility Functions
pair<TokenType,pair<int,string>> getNextTokenUtility();
bool matchAndAdvance(TokenType);

//Tiny Grammar
TreeNode* program();
TreeNode* stmtseq();
TreeNode* stmt();
TreeNode* ifstmt();
TreeNode* exp();
TreeNode* repeatstmt();
TreeNode* readstmt();
TreeNode* mathexpr();
TreeNode* term();
TreeNode* factor();
TreeNode* newexpr();
TreeNode* writestmt();
TreeNode* assignstmt();

TreeNode* Parser(){
    TreeNode* syntaxTree=program();
    return syntaxTree;
}

int main()
{
    //Scanning Phase
    char path[pathSize];cin>>path;
    freopen(path, "r", stdin);
    auto *compiler = new CompilerInfo("input.txt","output.txt","debug.txt");
    auto *token = new Token();
    bool checkError=false;
    while (token->type!=ENDFILE)
    {
        GetNextToken(compiler,token);

        // check error in scanning phase
        if(token->type==ERROR) {
            checkError=true;
            break;
        }
        compiler->out_file.Out(token->str);
        Tokens.push_back({token->type,{compiler->in_file.cur_line_num,token->str}});
    }

    //Parsing Phase
    freopen("scanningOutput.txt", "r", stdin);
    freopen("ParsingOutput.txt", "w", stdout);
    // stopping if error in scanning phase
    if(checkError){
        cout<<"Error in line "<<CToken.second.first<<endl;
        return 0;
    }

    TreeNode* syntaxTree=Parser(); // syntaxTree => root
    if(!checkErrorsExist) PrintTree(syntaxTree);
}

// program -> stmtseq
TreeNode* program(){
    TreeNode* root;
    CToken = getNextTokenUtility();
    root=stmtseq();
    return root;
}

// stmtseq -> stmt { ; stmt }
TreeNode* stmtseq(){
    TreeNode* firstStmt=stmt();
    TreeNode* ptr=firstStmt;
    while (CToken.first!=ENDFILE && CToken.first!=END && CToken.first!=UNTIL && CToken.first!=ELSE){
        matchAndAdvance(SEMI_COLON);
        TreeNode* nextStmt=stmt();
        if(ptr!=nullptr){
            ptr->sibling=nextStmt;
            ptr=nextStmt;
        }
        else firstStmt=nextStmt; // handle else
    }
    return firstStmt;
}

// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
TreeNode* stmt(){
    TreeNode* stmtNode=nullptr;
    switch (CToken.first){
        case IF:
            stmtNode=ifstmt();
            break;
        case REPEAT:
            stmtNode=repeatstmt();
            break;
        case ID:
            stmtNode=assignstmt();
            break;
        case READ:
            stmtNode=readstmt();
            break;
        case WRITE:
            stmtNode=writestmt();
            break;
        case NUM:
            cout<<"Error in line "<<CToken.second.first<<": "<<"Variable Name not correct you should begin with _ or any Character"<<endl;
            //exit(0);
            checkErrorsExist=true;
        default:
            matchAndAdvance(CToken.first);
            break;
    }
    return stmtNode;
}

// ifstmt -> if exp then stmtseq [ else stmtseq ] end
// ifstmt -> if child[0] then child[1] [ else child[2] ] end
TreeNode* ifstmt(){
    auto* ifStmtNode=new TreeNode;
    ifStmtNode->node_kind=IF_NODE;
    ifStmtNode->line_num=CToken.second.first;
    matchAndAdvance(IF);

    // ifStmt has 3 child ... if elseStmt exist
    ifStmtNode->child[0]=exp(); // first child is the condition

    // if THEN Stmt not exist
    if(CToken.first!=THEN) {
        cout<<"Error in line "<<CToken.second.first<<": "<<"THEN stmt expected after IF Condition"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    matchAndAdvance(THEN);

    ifStmtNode->child[1]=stmtseq(); // second child is the stmtSeq after then

    // if elseStmt exist
    if (CToken.first==ELSE)
        ifStmtNode->child[2]=stmtseq(); // third child is the elseStmt

    // if END Stmt not exist
    if(CToken.first!=END) {
        cout<<"Error in line "<<CToken.second.first<<": "<<"END stmt expected after IF Condition"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    matchAndAdvance(END);

    return ifStmtNode;
}

// repeatstmt -> repeat stmtseq until expr
TreeNode* repeatstmt(){
    auto* repeatStmtNode=new TreeNode;
    repeatStmtNode->node_kind=REPEAT_NODE;
    repeatStmtNode->line_num=CToken.second.first;
    matchAndAdvance(REPEAT);

    // repeatStmt has 2 child
    repeatStmtNode->child[0]=stmtseq(); // first child is the stmtSeq

    // if UNTIL Stmt not exist
    if(CToken.first!=UNTIL) {
        cout<<"Error in line "<<CToken.second.first<<": "<<"UNTIL condition expected after repeat stmt"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    matchAndAdvance(UNTIL);

    repeatStmtNode->child[1]=exp(); // second child is the condition

    return repeatStmtNode;
}

// assignstmt -> identifier := expr
TreeNode* assignstmt(){
    auto* assignNode=new TreeNode;
    assignNode->node_kind=ASSIGN_NODE;
    assignNode->line_num=CToken.second.first;

    // identifier (variable name)
    assignNode->id=CToken.second.second;
    matchAndAdvance(ID);

    // if Assignment Operator not exist
    if(CToken.first!=ASSIGN) {
        cout<<"Error in line "<<CToken.second.first<<": "<<"Assignment(:=) operator expected"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    matchAndAdvance(ASSIGN);
    assignNode->child[0]=exp(); // expression after := operator
    return assignNode;
}

// readstmt -> read identifier
TreeNode* readstmt(){
    auto* readStmtNode=new TreeNode;
    readStmtNode->node_kind=READ_NODE;
    readStmtNode->line_num=CToken.second.first;
    matchAndAdvance(READ);

    // if the identifier not exist or variable name wrong
    if(CToken.first!=ID) {
        cout<<"Error in line "<<CToken.second.first<<": "<<"Identifier expected after read stmt, variable name may be wrong"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    readStmtNode->id = CToken.second.second;
    matchAndAdvance(ID);
    return readStmtNode;
}

// writestmt -> write expr
TreeNode* writestmt(){
    auto* writeStmtNode=new TreeNode;
    writeStmtNode->node_kind=WRITE_NODE;
    writeStmtNode->line_num=CToken.second.first;
    matchAndAdvance(WRITE);

    writeStmtNode->child[0]=exp(); // any expression
    return writeStmtNode;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode* exp(){
    TreeNode* expNode=mathexpr();
    if (CToken.first==LESS_THAN || CToken.first==EQUAL){
        auto* newExpNode=new TreeNode;
        newExpNode->node_kind=OPER_NODE;
        newExpNode->oper=CToken.first;
        newExpNode->line_num=CToken.second.first;
        newExpNode->child[0]=expNode;
        matchAndAdvance(CToken.first);
        newExpNode->child[1]=mathexpr();
        return newExpNode;
    }
    return expNode;
}



// mathexpr -> term { (+|-) term }    left associative
TreeNode* mathexpr(){
    TreeNode* mathExprNode=term();

    while (CToken.first==PLUS ||CToken.first==MINUS){
        auto* newMathExprNode=new TreeNode;
        newMathExprNode->node_kind=OPER_NODE;
        newMathExprNode->oper=CToken.first;
        newMathExprNode->line_num=CToken.second.first;

        newMathExprNode->child[0]=mathExprNode;
        matchAndAdvance(CToken.first);
        newMathExprNode->child[1]=mathexpr();
        mathExprNode=newMathExprNode;
    }
    return mathExprNode;
}

// term -> factor { (*|/) factor }    left associative
TreeNode* term(){
    TreeNode* termNode=factor();

    while (CToken.first==TIMES || CToken.first==DIVIDE){
        auto* newTermNode=new TreeNode;
        newTermNode->node_kind=OPER_NODE;
        newTermNode->oper=CToken.first;
        newTermNode->line_num=CToken.second.first;

        newTermNode->child[0]=termNode;
        matchAndAdvance(CToken.first);
        newTermNode->child[1]=term();
        termNode=newTermNode;
    }
    return termNode;
}

// factor -> newexpr { ^ newexpr }    right associative
TreeNode* factor(){
    TreeNode* factorNode=newexpr();
    while (CToken.first==POWER){
        auto* newFactorNode=new TreeNode;
        newFactorNode->node_kind=OPER_NODE;
        newFactorNode->oper=CToken.first;
        newFactorNode->line_num=CToken.second.first;

        newFactorNode->child[0]=factorNode;
        matchAndAdvance(CToken.first);
        newFactorNode->child[1]=factor();
        return newFactorNode;
    }
    return factorNode;
}

// newexpr -> ( mathexpr ) | number | identifier
TreeNode* newexpr(){
    auto* newExprNode=new TreeNode;
    if (CToken.first==LEFT_PAREN){
        matchAndAdvance(LEFT_PAREN);
        newExprNode=mathexpr();
        matchAndAdvance(RIGHT_PAREN);
    }
    else if(CToken.first==NUM){
        newExprNode->node_kind=NUM_NODE;
        newExprNode->num= stoi(CToken.second.second);
        newExprNode->line_num=CToken.second.first;
        matchAndAdvance(NUM);
    }
    else if(CToken.first==ID){
        newExprNode->node_kind=ID_NODE;
        newExprNode->id=CToken.second.second;
        newExprNode->line_num=CToken.second.first;
        matchAndAdvance(ID);
    }
    else{
        cout<<"Error in line "<<CToken.second.first<<": "<<"Unexpected Error"<<endl;
//        exit(0);
        checkErrorsExist=true;
    }
    return newExprNode;
}


pair<TokenType,pair<int,string>> getNextTokenUtility(){
    if (Tokens.empty())exit(0);
    auto token = Tokens.front(); Tokens.pop_front();
    return token;
}

bool matchAndAdvance(TokenType currentToken){
    if (CToken.first==currentToken){
        CToken = getNextTokenUtility();
        return true;
    }
    return false;
}