#include <string>

typedef enum {
    TOK_EOF = -1,
    TOK_NEWLINE = 0,
    TOK_NUMBER = 1,
    TOK_IDENT = 2,
    TOK_STRING = 3,
    TOK_SEMI = 4,
    TOK_TYPE = 5,
    TOK_CHAR = 6,
    TOK_FLOAT = 7,
    TOK_COLON = 8,

    TOK_LABEL = 101,
    TOK_GOTO = 102,
    TOK_LET = 103,
    TOK_IF = 104,
    TOK_ELSE = 105,
    TOK_ELIF = 106,
    TOK_WHILE = 107,
    TOK_INCLUDE = 108,
    TOK_RETURN = 109,
    TOK_FUNC = 110,
    TOK_EXTERN = 111,
    TOK_STRUCT = 112,
    TOK_TYPEDEF = 113,
    TOK_FOR = 114,
    TOK_BREAK = 115,
    TOK_CONTINUE = 116,
    TOK_SWITCH = 117,
    TOK_CASE = 118,
    TOK_ENUM = 119,
    TOK_HEADER = 120,
    TOK_EXTC = 121,
    TOK_FROM = 122,

    TOK_EQ = 201,
    TOK_PLUS = 202,
    TOK_MINUS = 203,
    TOK_ASTERISK = 204,
    TOK_SLASH = 205,
    TOK_EQEQ = 206,
    TOK_BANGEQ = 207,
    TOK_LT = 208,
    TOK_LTEQ = 209,
    TOK_GT = 210,
    TOK_GTEQ = 211,
    TOK_LPAREN = 212,
    TOK_RPAREN = 213,
    TOK_LBRACE = 214,
    TOK_RBRACE = 215,
    TOK_COMMA = 216,
    TOK_HASH = 217,
    TOK_DOT = 218,
    TOK_LSQUARE = 219,
    TOK_RSQUARE = 220,
    TOK_LSHIFT = 221,
    TOK_RSHIFT = 222,
    TOK_BOR = 223,
    TOK_BAND = 224,
    TOK_LOR = 225,
    TOK_LAND = 226,
} TokenType;

class Token {
    private:
        std::string text;
        TokenType type;

    public:
        Token(std::string text, TokenType type);
        Token(char text, TokenType type);
        std::string getText() {
            return text;
        }

        TokenType getType() {
            return type;
        }

        static TokenType checkIfKeyword(std::string text);

};

class Lexer {
    public:
        Lexer(std::string source);
        void nextChar();
        char peek();
        void abort(std::string reason);
        void skipWhitespace();
        void skipComment();
        Token getToken();
        int line;
    private:
        std::string source;
        char current;
        int pos;
};