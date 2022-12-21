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
#include <cmath>
#include <deque>

#define endl '\n'
const int pathSize = 1000;
using namespace std;


////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool StartsWith(const char *a, const char *b) {
    int nb = (int) strlen(b);
    return strncmp(a, b, nb) == 0;
}

void Copy(char *a, const char *b, int n = 0) {
    if (n > 0) {
        strncpy(a, b, n);
        a[n] = 0;
    }
    else strcpy(a, b);
}

void AllocateAndCopy(char **a, const char *b) {
    if (b == nullptr) {
        *a = nullptr;
        return;
    }
    int n = (int) strlen(b);
    *a = new char[n + 1];
    strcpy(*a, b);
}

// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile {
    FILE *file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH]{};
    int cur_ind, cur_line_size;

    explicit InFile(const char *str) {
        file = nullptr;
        if (str) file = fopen(str, "r");
        cur_line_size = 0;
        cur_ind = 0;
        cur_line_num = 0;
    }

    ~InFile() { if (file) fclose(file); }

    void SkipSpaces() {
        while (cur_ind < cur_line_size) {
            char ch = line_buf[cur_ind];
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char *str) {
        while (true) {
            SkipSpaces();
            while (cur_ind >= cur_line_size) {
                if (!GetNewLine()) return false;
                SkipSpaces();
            }

            if (StartsWith(&line_buf[cur_ind], str)) {
                cur_ind += (int) strlen(str);
                return true;
            }
            cur_ind++;
        }
    }

    bool GetNewLine() {
        cur_ind = 0;
        line_buf[0] = 0;
        if (!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size = (int) strlen(line_buf);
        if (cur_line_size == 0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char *GetNextTokenStr() {
        SkipSpaces();
        while (cur_ind >= cur_line_size) {
            if (!GetNewLine()) return nullptr;
            SkipSpaces();
        }
        return &line_buf[cur_ind];
    }

    void Advance(int num) {
        cur_ind += num;
    }
};

struct OutFile {
    FILE *file;

    explicit OutFile(const char *str) {
        file = nullptr;
        if (str) file = fopen(str, "w");
    }

    ~OutFile() { if (file) fclose(file); }

    void Out(const char *s) const {
        fprintf(file, "%s\n", s);
        fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo {
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char *in_str, const char *out_str, const char *debug_str)
            : in_file(in_str), out_file(out_str), debug_file(debug_str) {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType {
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
const char *TokenTypeStr[] =
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

struct Token {
    TokenType type;
    char str[MAX_TOKEN_LEN + 1]{};

    Token() {
        str[0] = 0;
        type = ERROR;
    }

    Token(TokenType _type, const char *_str) {
        type = _type;
        Copy(str, _str);
    }
};

const Token reserved_words[] =
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
const int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[] =
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
const int num_symbolic_tokens = sizeof(symbolic_tokens) / sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch) { return (ch >= '0' && ch <= '9'); }

inline bool IsLetter(char ch) { return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')); }

inline bool IsLetterOrUnderscore(char ch) { return (IsLetter(ch) || ch == '_'); }

void GetNextToken(CompilerInfo *pci, Token *ptoken) {
    ptoken->type = ERROR;
    ptoken->str[0] = 0;

    int i;
    char *s = pci->in_file.GetNextTokenStr();
    if (!s) {
        ptoken->type = ENDFILE;
        ptoken->str[0] = 0;
        return;
    }

    for (i = 0; i < num_symbolic_tokens; i++) {
        if (StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if (i < num_symbolic_tokens) {
        if (symbolic_tokens[i].type == LEFT_BRACE) {
            pci->in_file.Advance((int) strlen(symbolic_tokens[i].str));
            if (!pci->in_file.SkipUpto(symbolic_tokens[i + 1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type = symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    } else if (IsDigit(s[0])) {
        int j = 1;
        while (IsDigit(s[j])) j++;

        ptoken->type = NUM;
        Copy(ptoken->str, s, j);
    } else if (IsLetterOrUnderscore(s[0])) {
        int j = 1;
        while (IsLetterOrUnderscore(s[j])) j++;

        ptoken->type = ID;
        Copy(ptoken->str, s, j);

        for (i = 0; i < num_reserved_words; i++) {
            if (Equals(ptoken->str, reserved_words[i].str)) {
                ptoken->type = reserved_words[i].type;
                break;
            }
        }
    }

    int len = (int) strlen(ptoken->str);
    if (len > 0) pci->in_file.Advance(len);
}

// Parser //////////////////////////////////////////////////////////////////////////


enum NodeKind {
    IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
    OPER_NODE, NUM_NODE, ID_NODE
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *NodeKindStr[] =
        {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
        };

enum ExprDataType {
    VOID, INTEGER, BOOLEAN [[maybe_unused]]
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *ExprDataTypeStr[] =
        {
                "Void", "Integer", "Boolean"
        };

#define MAX_CHILDREN 3

struct TreeNode {
    TreeNode *child[MAX_CHILDREN]{};//terminal
    TreeNode *sibling; // used for sibling statements only

    NodeKind node_kind;
    string id;
    union {
        TokenType oper;
        int num{};
    }; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num{};

    TreeNode() {
        int i;
        for (i = 0; i < MAX_CHILDREN; i++) child[i] = nullptr;
        sibling = nullptr;
        expr_data_type = VOID;
    }
};

struct ParseInfo {
    Token next_token;
};

void PrintTree(TreeNode *node, int sh = 0) {
    int i, NSH = 3;
    for (i = 0; i < sh; i++) printf(" ");

    if (node != nullptr) printf("[%s]", NodeKindStr[node->node_kind]);

    if (node != nullptr && node->node_kind == OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if (node != nullptr && node->node_kind == NUM_NODE) printf("[%d]", node->num);
    else if (node != nullptr &&
             (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE))
        printf("[%s]", node->id.c_str());

    if (node != nullptr && node->expr_data_type != VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for (i = 0; i < MAX_CHILDREN; i++) if (node->child[i]) PrintTree(node->child[i], sh + NSH);
    if (node->sibling) PrintTree(node->sibling, sh);


}


//global variables
deque<pair<TokenType, pair<int, string>>> Tokens; // to save each token with its value and line
pair<TokenType, pair<int, string>> CToken; // to know the current token
bool checkErrorsExist = false; // to check if errors exist in parsing phase

//Utility Functions
pair<TokenType, pair<int, string>> getNextTokenUtility();

bool matchAndAdvance(TokenType);

//Tiny Grammar
TreeNode *program();

TreeNode *stmtseq();

TreeNode *stmt();

TreeNode *ifstmt();

TreeNode *exp();

TreeNode *repeatstmt();

TreeNode *readstmt();

TreeNode *mathexpr();

TreeNode *term();

TreeNode *factor();

TreeNode *newexpr();

TreeNode *writestmt();

TreeNode *assignstmt();



TreeNode *Parser() {
    TreeNode *syntaxTree = program();
    return syntaxTree;
}



////////////////////////////////////////////////////////////////////////////////////
// Analyzer ////////////////////////////////////////////////////////////////////////

const int SYMBOL_HASH_SIZE = 10007;

struct LineLocation {
    int line_num;
    LineLocation *next;
};

struct VariableInfo {
    char *name;
    int memloc;
    LineLocation *head_line; // the head of linked list of source line locations
    LineLocation *tail_line; // the tail of linked list of source line locations
    VariableInfo *next_var; // the next variable in the linked list in the same hash bucket of the symbol table
};

struct SymbolTable {
    int num_vars;
    VariableInfo *var_info[SYMBOL_HASH_SIZE]{};

    SymbolTable() {
        num_vars = 0;
        int i;
        for (i = 0; i < SYMBOL_HASH_SIZE; i++) var_info[i] = nullptr;
    }

    static int Hash(const char *name) {
        int i, len = (int) strlen(name);
        int hash_val = 11;
        for (i = 0; i < len; i++) hash_val = (hash_val * 17 + (int) name[i]) % SYMBOL_HASH_SIZE;
        return hash_val;
    }

    VariableInfo *Find(const char *name) {
        int h = Hash(name);
        VariableInfo *cur = var_info[h];
        while (cur) {
            if (Equals(name, cur->name)) return cur;
            cur = cur->next_var;
        }
        return nullptr;
    }

    void Insert(const char *name, int line_num) {
        auto *lineloc = new LineLocation;
        lineloc->line_num = line_num;
        lineloc->next = nullptr;

        int h = Hash(name);
        VariableInfo *prev = nullptr;
        VariableInfo *cur = var_info[h];

        while (cur) {
            if (Equals(name, cur->name)) {
                // just add this line location to the list of line locations of the existing var
                cur->tail_line->next = lineloc;
                cur->tail_line = lineloc;
                return;
            }
            prev = cur;
            cur = cur->next_var;
        }

        auto *vi = new VariableInfo;
        vi->head_line = vi->tail_line = lineloc;
        vi->next_var = nullptr;
        vi->memloc = num_vars++;
        AllocateAndCopy(&vi->name, name);

        if (!prev) var_info[h] = vi;
        else prev->next_var = vi;
    }

    void Print() {
        int i;
        for (i = 0; i < SYMBOL_HASH_SIZE; i++) {
            VariableInfo *curv = var_info[i];
            while (curv) {
                printf("[Var=%s][Mem=%d]", curv->name, curv->memloc);
                LineLocation *curl = curv->head_line;
                while (curl) {
                    printf("[Line=%d]", curl->line_num);
                    curl = curl->next;
                }
                printf("\n");
                curv = curv->next_var;
            }
        }
    }

    void Destroy() {
        int i;
        for (i = 0; i < SYMBOL_HASH_SIZE; i++) {
            VariableInfo *curv = var_info[i];
            while (curv) {
                LineLocation *curl = curv->head_line;
                while (curl) {
                    LineLocation *pl = curl;
                    curl = curl->next;
                    delete pl;
                }
                VariableInfo *p = curv;
                curv = curv->next_var;
                delete p;
            }
            var_info[i] = nullptr;
        }
    }
};

// this function to generate symbol table and identify nodes type and type checking errors
void generateProgram(TreeNode *currNode, SymbolTable *symbolTable) {

    // Identify The Node Kind For Parse Tree
    if (currNode->node_kind == ID_NODE || currNode->node_kind == NUM_NODE)
        currNode->expr_data_type = INTEGER;
    else if (currNode->node_kind == OPER_NODE) {
        (currNode->oper == EQUAL || currNode->oper == LESS_THAN) ? currNode->expr_data_type = BOOLEAN :
                currNode->expr_data_type = INTEGER;
    }
    else
        currNode->expr_data_type = VOID;

    // generate Symbol Table with variables of code
    if (currNode->node_kind == ASSIGN_NODE || currNode->node_kind == READ_NODE || currNode->node_kind == ID_NODE)
        symbolTable->Insert(currNode->id.c_str(), currNode->line_num);

    // call function with each node in tree
    if (currNode->child[0] != nullptr) {
        for (auto &child: currNode->child) {
            if (child != nullptr)
                generateProgram(child, symbolTable);
        }
    }

    //Type Checking
    if(currNode->node_kind==WRITE_NODE && currNode->child[0]->expr_data_type!=INTEGER) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected integer with write stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==ASSIGN_NODE && currNode->child[0]->expr_data_type!=INTEGER) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected integer with Assign stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==IF_NODE && currNode->child[0]->expr_data_type!=BOOLEAN) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected boolean condition with if stmt"<<endl;
        exit(0);
    }
    if(currNode->node_kind==REPEAT_NODE && currNode->child[1]->expr_data_type!=BOOLEAN) {
        cerr<<"Error in line "<<currNode->line_num<<": Expected boolean condition with until stmt"<<endl;
        exit(0);
    }


    if (currNode->sibling != nullptr)
        generateProgram(currNode->sibling, symbolTable);
}



// to evaluate the simple
long long eval(TokenType tokenType, long long left, long long right) {
    if (tokenType == POWER) return (long long) pow(left, right);
    else if (tokenType == PLUS) return left+right;
    else if  (tokenType == MINUS) return left-right;
    else if  (tokenType == TIMES) return left*right;
    else if  (tokenType == DIVIDE) return left/right;
    else if  (tokenType == EQUAL) return left==right;
    else if  (tokenType == LESS_THAN) return left<right;
    return 0;
}

// to evaluate complex stmt
long long evalUtility(TreeNode* currNode, SymbolTable* symbolTable, vector<long long> &variablesTable){
    // base cases
    if(currNode->node_kind==ID_NODE) return variablesTable[symbolTable->Find(currNode->id.c_str())->memloc];
    if(currNode->node_kind==NUM_NODE) return currNode->num;

    long long left=evalUtility(currNode->child[0], symbolTable, variablesTable); // left part
    long long right=evalUtility(currNode->child[1], symbolTable, variablesTable); // right part

    return eval(currNode->oper,left,right);
}

// This function to simulate running program
void runProgram(TreeNode *currNode, SymbolTable *symbolTable, vector<long long> &variablesTable) {
    // if node type is read -> expected integer input
    if (currNode->node_kind == READ_NODE){
        int idx=symbolTable->Find(currNode->id.c_str())->memloc;
        cout<<"Enter "<<currNode->id.c_str()<<":";
        cin>>variablesTable[idx];
        // check if input not integer
        if(!std::cin.good()) {
            cerr<<"Run Time Error in line "<<currNode->line_num<<": Expected integer with read stmt"<<endl;
            exit(0);
        }
    }

    // if node type is write -> expected integer input (variable, number or operations)
    else if(currNode->node_kind == WRITE_NODE){
        // if write variable -> print the value of this variable
        if (currNode->child[0]->node_kind == ID_NODE)
            cout<<"val: "<<variablesTable[symbolTable->Find((currNode->child[0]->id.c_str()))->memloc]<<endl;

        // if write number -> print this number
        else if (currNode->child[0]->node_kind == NUM_NODE)
            cout<<"val: "<<currNode->child[0]->num<<endl;

        // if write mathematics operations for example:- 5+10
        else {
            long long val=evalUtility(currNode->child[0], symbolTable, variablesTable);
            cout<<"val: "<<val<<endl;
        }
    }

    // if node type is IF
    else if (currNode->node_kind == IF_NODE) {
        long long left = 0, right = 0;

        // check left part of condition
        if(currNode->child[0]->child[0]->node_kind==ID_NODE)
            left = variablesTable[symbolTable->Find(currNode->child[0]->child[0]->id.c_str())->memloc];
        else if(currNode->child[0]->child[0]->node_kind==NUM_NODE)
            left = currNode->child[0]->child[0]->num;

        // check right part of condition
        if(currNode->child[0]->child[1]->node_kind==ID_NODE)
            right = variablesTable[symbolTable->Find(currNode->child[0]->child[1]->id.c_str())->memloc];
        else if(currNode->child[0]->child[1]->node_kind==NUM_NODE)
            right = currNode->child[0]->child[1]->num;

        bool conditionCheck = bool(eval(currNode->child[0]->oper,left,right));

        if(conditionCheck)
            runProgram(currNode->child[1],symbolTable,variablesTable); // Run code after if part
        else if (currNode->child[2])
            runProgram(currNode->child[2],symbolTable,variablesTable); // Run code after else part
    }

    // if node type is ASSIGN
    else if (currNode->node_kind == ASSIGN_NODE) {
        long long value=0;
        // if variable -> assign the value of this variable (initial zero for each variable unassigned)
        if(currNode->child[0]->node_kind==ID_NODE)
            value = variablesTable[symbolTable->Find(currNode->child[0]->id.c_str())->memloc];

        // if number -> assign this value
        else if(currNode->child[0]->node_kind==NUM_NODE)
            value = currNode->child[0]->num;

        // if write mathematics operations for example:- 5+10
        else
            value=evalUtility(currNode->child[0], symbolTable, variablesTable);

        variablesTable[symbolTable->Find(currNode->id.c_str())->memloc] = value; // assign variable with value
    }

        // if node type is REPEAT
    else if (currNode->node_kind == REPEAT_NODE) {
        // this node act as do while loop behavior

        endingPoint: // do {stmt}
        runProgram(currNode->child[0], symbolTable, variablesTable); // do stmt until specific condition

        long long value=0;

        // if variable -> assign the value of this variable (initial zero for each variable unassigned)
        if(currNode->child[1]->node_kind==ID_NODE)
            value = variablesTable[symbolTable->Find(currNode->child[1]->id.c_str())->memloc];

        // if number -> assign this value
        else if(currNode->child[1]->node_kind==NUM_NODE)
            value = currNode->child[1]->num;

        // if exist boolean condition
        else {
            long long left = 0, right = 0;

            // check left part of condition
            if(currNode->child[1]->child[0]->node_kind==ID_NODE)
                left = variablesTable[symbolTable->Find(currNode->child[1]->child[0]->id.c_str())->memloc];
            else if(currNode->child[1]->child[0]->node_kind==NUM_NODE)
                left = currNode->child[1]->child[0]->num;

            // check right part of condition
            if(currNode->child[1]->child[1]->node_kind==ID_NODE)
                right = variablesTable[symbolTable->Find(currNode->child[1]->child[1]->id.c_str())->memloc];
            else if(currNode->child[1]->child[1]->node_kind==NUM_NODE)
                right = currNode->child[1]->child[1]->num;

            value = eval(currNode->child[1]->oper,left,right); // evaluate the value
        }
        if(!value) goto endingPoint; // until value -> if value=false then break ... if value=ture then execute one more time
    }

    if (currNode->sibling!= nullptr) runProgram(currNode->sibling, symbolTable, variablesTable);
}


int main() {
    //Scanning Phase
    FILE *f = freopen("input.txt", "r", stdin);
    auto *compiler = new CompilerInfo("input.txt", "output.txt", "debug.txt");
    auto *token = new Token();
    bool checkError = false;
    int lineTemp=0;
    while (token->type != ENDFILE) {
        GetNextToken(compiler, token);

        // check error in scanning phase
        if (token->type == ERROR) {
            checkError = true;
            lineTemp=compiler->in_file.cur_line_num;
            break;
        }
        compiler->out_file.Out(token->str);
        Tokens.push_back({token->type, {compiler->in_file.cur_line_num, token->str}});
    }

    //Parsing Phase
    FILE *fp = freopen("scanningOutput.txt", "r", stdin);
    //freopen("ParsingOutput.txt", "w", stdout);
    // stopping if error in scanning phase
    if (checkError) {
        cout << "Compilation Error in line " << lineTemp << endl;
        return 0;
    }

    TreeNode *syntaxTree = Parser(); // syntaxTree => root
    freopen("CON", "r", stdin);
    if (!checkErrorsExist) {
        auto *symbolTable = new SymbolTable();
        generateProgram(syntaxTree, symbolTable);

        // Print The Symbol Table
        cout << "The Symbol Table" << endl;
        symbolTable->Print();
        cout << "------------------------------------------------------------" << endl;

        // Print The Syntax Tree
        cout << "The Syntax Tree" << endl;
        PrintTree(syntaxTree);
        cout << "------------------------------------------------------------" << endl;

        // Run instance of program
        vector<long long> variablesTable(symbolTable->num_vars);
        cout << "Instance of Running Program" << endl;
        runProgram(syntaxTree, symbolTable, variablesTable);
        cout << "------------------------------------------------------------" << endl;
        delete syntaxTree;delete symbolTable;
    }

}


// program -> stmtseq
TreeNode *program() {
    TreeNode *root;
    CToken = getNextTokenUtility();
    root = stmtseq();
    return root;
}

// stmtseq -> stmt { ; stmt }
TreeNode *stmtseq() {
    TreeNode *firstStmt = stmt();
    TreeNode *ptr = firstStmt;
    while (CToken.first != ENDFILE && CToken.first != END && CToken.first != UNTIL && CToken.first != ELSE) {
        matchAndAdvance(SEMI_COLON);
        TreeNode *nextStmt = stmt();
        if (ptr != nullptr) {
            ptr->sibling = nextStmt;
            ptr = nextStmt;
        } else firstStmt = nextStmt; // handle else
    }
    return firstStmt;
}

// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
TreeNode *stmt() {
    TreeNode *stmtNode = nullptr;
    switch (CToken.first) {
        case IF:
            stmtNode = ifstmt();
            break;
        case REPEAT:
            stmtNode = repeatstmt();
            break;
        case ID:
            stmtNode = assignstmt();
            break;
        case READ:
            stmtNode = readstmt();
            break;
        case WRITE:
            stmtNode = writestmt();
            break;
        case NUM:
            cout << "Error in line " << CToken.second.first << ": "
                 << "Variable Name not correct you should begin with _ or any Character" << endl;
            //exit(0);
            checkErrorsExist = true;
        default:
            matchAndAdvance(CToken.first);
            break;
    }
    return stmtNode;
}

// ifstmt -> if exp then stmtseq [ else stmtseq ] end
// ifstmt -> if child[0] then child[1] [ else child[2] ] end
TreeNode *ifstmt() {
    auto *ifStmtNode = new TreeNode;
    ifStmtNode->node_kind = IF_NODE;
    ifStmtNode->line_num = CToken.second.first;
    matchAndAdvance(IF);

    // ifStmt has 3 child ... if elseStmt exist
    ifStmtNode->child[0] = exp(); // first child is the condition

    // if THEN Stmt not exist
    if (CToken.first != THEN) {
        cout << "Error in line " << CToken.second.first << ": " << "THEN stmt expected after IF Condition" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    matchAndAdvance(THEN);

    ifStmtNode->child[1] = stmtseq(); // second child is the stmtSeq after then

    // if elseStmt exist
    if (CToken.first == ELSE)
        ifStmtNode->child[2] = stmtseq(); // third child is the elseStmt

    // if END Stmt not exist
    if (CToken.first != END) {
        cout << "Error in line " << CToken.second.first << ": " << "END stmt expected after IF Condition" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    matchAndAdvance(END);

    return ifStmtNode;
}

// repeatstmt -> repeat stmtseq until expr
TreeNode *repeatstmt() {
    auto *repeatStmtNode = new TreeNode;
    repeatStmtNode->node_kind = REPEAT_NODE;
    repeatStmtNode->line_num = CToken.second.first;
    matchAndAdvance(REPEAT);

    // repeatStmt has 2 child
    repeatStmtNode->child[0] = stmtseq(); // first child is the stmtSeq

    // if UNTIL Stmt not exist
    if (CToken.first != UNTIL) {
        cout << "Error in line " << CToken.second.first << ": " << "UNTIL condition expected after repeat stmt" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    matchAndAdvance(UNTIL);

    repeatStmtNode->child[1] = exp(); // second child is the condition

    return repeatStmtNode;
}

// assignstmt -> identifier := expr
TreeNode *assignstmt() {
    auto *assignNode = new TreeNode;
    assignNode->node_kind = ASSIGN_NODE;
    assignNode->line_num = CToken.second.first;

    // identifier (variable name)
    assignNode->id = CToken.second.second;
    matchAndAdvance(ID);

    // if Assignment Operator not exist
    if (CToken.first != ASSIGN) {
        cout << "Error in line " << CToken.second.first << ": " << "Assignment(:=) operator expected" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    matchAndAdvance(ASSIGN);
    assignNode->child[0] = exp(); // expression after := operator
    return assignNode;
}

// readstmt -> read identifier
TreeNode *readstmt() {
    auto *readStmtNode = new TreeNode;
    readStmtNode->node_kind = READ_NODE;
    readStmtNode->line_num = CToken.second.first;
    matchAndAdvance(READ);

    // if the identifier not exist or variable name wrong
    if (CToken.first != ID) {
        cout << "Error in line " << CToken.second.first << ": "
             << "Identifier expected after read stmt, variable name may be wrong" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    readStmtNode->id = CToken.second.second;
    matchAndAdvance(ID);
    return readStmtNode;
}

// writestmt -> write expr
TreeNode *writestmt() {
    auto *writeStmtNode = new TreeNode;
    writeStmtNode->node_kind = WRITE_NODE;
    writeStmtNode->line_num = CToken.second.first;
    matchAndAdvance(WRITE);

    writeStmtNode->child[0] = exp(); // any expression
    return writeStmtNode;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode *exp() {
    TreeNode *expNode = mathexpr();
    if (CToken.first == LESS_THAN || CToken.first == EQUAL) {
        auto *newExpNode = new TreeNode;
        newExpNode->node_kind = OPER_NODE;
        newExpNode->oper = CToken.first;
        newExpNode->line_num = CToken.second.first;
        newExpNode->child[0] = expNode;
        matchAndAdvance(CToken.first);
        newExpNode->child[1] = mathexpr();
        return newExpNode;
    }
    return expNode;
}

// mathexpr -> term { (+|-) term }    left associative
TreeNode *mathexpr() {
    TreeNode *mathExprNode = term();

    while (CToken.first == PLUS || CToken.first == MINUS) {
        auto *newMathExprNode = new TreeNode;
        newMathExprNode->node_kind = OPER_NODE;
        newMathExprNode->oper = CToken.first;
        newMathExprNode->line_num = CToken.second.first;

        newMathExprNode->child[0] = mathExprNode;
        matchAndAdvance(CToken.first);
        newMathExprNode->child[1] = term();
        mathExprNode = newMathExprNode;
    }
    return mathExprNode;
}

// term -> factor { (*|/) factor }    left associative
TreeNode *term() {
    TreeNode *termNode = factor();

    while (CToken.first == TIMES || CToken.first == DIVIDE) {
        auto *newTermNode = new TreeNode;
        newTermNode->node_kind = OPER_NODE;
        newTermNode->oper = CToken.first;
        newTermNode->line_num = CToken.second.first;

        newTermNode->child[0] = termNode;
        matchAndAdvance(CToken.first);
        newTermNode->child[1] = factor();
        termNode = newTermNode;
    }
    return termNode;
}

// factor -> newexpr { ^ newexpr }    right associative
TreeNode *factor() {
    TreeNode *factorNode = newexpr();
    while (CToken.first == POWER) {
        auto *newFactorNode = new TreeNode;
        newFactorNode->node_kind = OPER_NODE;
        newFactorNode->oper = CToken.first;
        newFactorNode->line_num = CToken.second.first;

        newFactorNode->child[0] = factorNode;
        matchAndAdvance(CToken.first);
        newFactorNode->child[1] = factor();
        return newFactorNode;
    }
    return factorNode;
}

// newexpr -> ( mathexpr ) | number | identifier
TreeNode *newexpr() {
    auto *newExprNode = new TreeNode;
    if (CToken.first == LEFT_PAREN) {
        matchAndAdvance(LEFT_PAREN);
        newExprNode = mathexpr();
        matchAndAdvance(RIGHT_PAREN);
    } else if (CToken.first == NUM) {
        newExprNode->node_kind = NUM_NODE;
        newExprNode->num = stoi(CToken.second.second);
        newExprNode->line_num = CToken.second.first;
        matchAndAdvance(NUM);
    } else if (CToken.first == ID) {
        newExprNode->node_kind = ID_NODE;
        newExprNode->id = CToken.second.second;
        newExprNode->line_num = CToken.second.first;
        matchAndAdvance(ID);
    } else {
        cout << "Error in line " << CToken.second.first << ": " << "Unexpected Error" << endl;
//        exit(0);
        checkErrorsExist = true;
    }
    return newExprNode;
}


pair<TokenType, pair<int, string>> getNextTokenUtility() {
    if (Tokens.empty())exit(0);
    auto token = Tokens.front();
    Tokens.pop_front();
    return token;
}

bool matchAndAdvance(TokenType currentToken) {
    if (CToken.first == currentToken) {
        CToken = getNextTokenUtility();
        return true;
    }
    return false;
}