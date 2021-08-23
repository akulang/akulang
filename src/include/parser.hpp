#include "lexer.hpp"
#include "emitter.hpp"
#include <map>
#include <list>
#include <set>

typedef void (*porfunc)(std::string);
typedef void (*linefunc)(std::string);

class Parser {
    public:
        Parser(std::string source, std::string outfile, bool usestdlib, bool includeMetal);
        bool checkToken(TokenType kind);
        bool checkPeek(TokenType kind);
        void match(TokenType kind);
        bool validType(std::string typk) {
            if (types.find(typk) != types.end())
                return true;
            return false;
        }
        void nextToken();
        void abort(std::string reason);
        void note(std::string reason);
        void warn(std::string reason);
        void nl();
        void statement(std::string scope);
        bool isComparisonOperator() {
            return checkToken(TOK_GT) || checkToken(TOK_GTEQ) || checkToken(TOK_LT) || checkToken(TOK_LTEQ) || checkToken(TOK_EQEQ) || checkToken(TOK_BANGEQ) || checkToken(TOK_LOR) || checkToken(TOK_LAND);
        }
        void term(std::string scope);
        void unary(std::string scope);
        void comparison(std::string scope);
        void shift(std::string scope);
        void binary(std::string scope);
        void primary(std::string scope, bool allowStr);
        void expression(std::string scope);
        void program();
        Emitter emitter;
    private:
        Lexer lexer;
        bool usestdlib;
        bool includeMetal;
        std::map<std::string, std::string> globals;
        std::map<std::string, std::map<std::string, std::string>> locals;
        std::set<std::string> types;
        Token curToken;
        Token peekToken;
        std::set<std::string> cheaders;
        std::set<std::string> usings;
};