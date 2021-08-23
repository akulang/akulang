#include "lexer.hpp"
#include <stdlib.h>

Token::Token(std::string text, TokenType type) {
    this->text = text;
    this->type = type;
}

Token::Token(char text, TokenType type)  {
    this->text = text;
    this->type = type;
}

TokenType Token::checkIfKeyword(std::string text) {
    if(text == "label") return TOK_LABEL;
    else if(text == "goto") return TOK_GOTO;
    else if(text == "let") return TOK_LET;
    else if(text == "if") return TOK_IF;
    else if(text == "else") return TOK_ELSE;
    else if(text == "elif") return TOK_ELIF;
    else if(text == "while") return TOK_WHILE;
    else if(text == "include") return TOK_INCLUDE;
    else if(text == "return") return TOK_RETURN;
    else if(text == "func") return TOK_FUNC;
    else if(text == "extern") return TOK_EXTERN;
    else if(text == "struct") return TOK_STRUCT;
    else if(text == "typedef") return TOK_TYPEDEF;
    else if(text == "for") return TOK_FOR;
    else if(text == "break") return TOK_BREAK;
    else if(text == "continue") return TOK_CONTINUE;
    else if(text == "switch") return TOK_SWITCH;
    else if(text == "case") return TOK_CASE;
    else if(text == "enum") return TOK_ENUM;
    else if(text == "header") return TOK_HEADER;
    else if(text == "extc") return TOK_EXTC;
    else if(text == "from") return TOK_FROM;
    return TOK_IDENT;
}

Lexer::Lexer(std::string source) {
    this->source = source + "\n";
    this->current = '\0';
    this->pos = -1;
    this->nextChar();
    this->line = 1;
}

void Lexer::nextChar() {
    this->pos += 1;
    if(this->pos >= this->source.length())
        this->current = '\0';
    else
        this->current = this->source[this->pos];
    if(this->current == '\n')
        this->line++;
}

char Lexer::peek() {
    if(this->pos + 1 >= this->source.length())
        return '\0';
    return this->source[this->pos + 1];
}

void Lexer::abort(std::string reason) {
    fprintf(stderr, "akucc: \033[31;1mlexer\033[0m: %s\n", reason.c_str());
    std::exit(1);
}

void Lexer::skipWhitespace() {
    while(this->current == ' ' || this->current == '\t' || this->current == '\r') {
        this->nextChar();
    }
}

void Lexer::skipComment() {
    if(this->current == '/') {
        this->nextChar();
        if(this->current == '/') {
            while(this->current != '\n')
                this->nextChar();
        } else if(this->current == '*') {
            this->nextChar();
            while(!(this->current == '*' && this->current == '/'))
                this->nextChar();
            this->nextChar();
            this->nextChar();
        }
    }
}

Token Lexer::getToken() {
    this->skipWhitespace();
    this->skipComment();
    Token token("", TOK_EOF);

    if(this->current == '+')
        token = Token(this->current, TOK_PLUS);
    else if(this->current == '-')
        token = Token(this->current, TOK_MINUS);
    else if(this->current == '*')
        token = Token(this->current, TOK_ASTERISK);
    else if(this->current == '/')
        token = Token(this->current, TOK_ASTERISK);
    else if(this->current == '=') {
        if(this->peek() == '=') {
            this->nextChar();
            token = Token("==", TOK_EQEQ);
        } else {
            token = Token(this->current, TOK_EQ);
        }
    } else if(this->current == '>') {
        if(this->peek() == '=') {
            this->nextChar();
            token = Token(">=", TOK_GTEQ);
        } else if(this->peek() == '>') {
            this->nextChar();
            token = Token(">>", TOK_RSHIFT);
        } else {
            token = Token(this->current, TOK_GT);
        }
    } else if(this->current == '<') {
        if(this->peek() == '=') {
            this->nextChar();
            token = Token("<=", TOK_LTEQ);
        } else if(this->peek() == '<') {
            this->nextChar();
            token = Token("<<", TOK_LSHIFT);
        } else {
            token = Token(this->current, TOK_LT);
        }
    } else if(this->current == '!') {
        if(this->peek() == '=') {
            this->nextChar();
            token = Token("!=", TOK_BANGEQ);
        } else {
            char* t = (char*)malloc(32);
            sprintf(t, "expected !=, got !`%c`", this->peek());
            this->abort(t);
        }
    } else if(this->current == '|') {
        if(this->peek() == '|') {
            this->nextChar();
            token = Token("||", TOK_LOR);
        } else {
            token = Token(this->current, TOK_BOR);
        }
    } else if(this->current == '&') {
        if(this->peek() == '&') {
            this->nextChar();
            token = Token("&", TOK_LAND);
        } else {
            token = Token(this->current, TOK_BAND);
        }
    } else if(this->current == '"') {
        this->nextChar();
        int startPos = this->pos;
        bool marked = false;

        while(!(!marked && this->current == '"')) {
            marked = false;
            if(this->current == '\r' || this->current == '\n' || this->current == '\t') {
                this->abort("illegal character in string");
            }
            if(this->current == '\\')
                marked = true;
            this->nextChar();
        }

        std::string tokText = this->source.substr(startPos, (this->pos) - startPos);
        token = Token(tokText, TOK_STRING);
    } else if(this->current == '\'') {
        this->nextChar();
        int startPos = this->pos;
        int allowed = 1;
        bool marked = false;

        while(!(!marked && this->current == '\'')) {
            marked = false;
            if(!isascii(this->current))
                this->abort("character exceeds ASCII range 0-255");
            else if(this->current == '\r' || this->current == '\n' || this->current == '\t')
                this->abort("illegal escape in character literal");
            else if(this->current == '\\') {
                marked = true;
                allowed = 2;
            }
            this->nextChar();
        }

        std::string tokText = this->source.substr(startPos, (this->pos) - startPos);
        if(this->pos - startPos > allowed)
            this->abort("character of illegal length.");
        token = Token(tokText, TOK_CHAR);
    } else if(isdigit(this->current)) {
        if(this->peek() != 'x') {
            int startPos = this->pos;
            bool isFloat = false;

            while(isdigit(this->peek()))
                this->nextChar();

            if(this->peek() == '.') {
                isFloat = true;
                this->nextChar();
                while(isdigit(this->peek()))
                    this->nextChar();
            }

            std::string tokText = this->source.substr(startPos, (this->pos + 1) - startPos);
            if(!isFloat)
                token = Token(tokText, TOK_NUMBER);
            else
                token = Token(tokText, TOK_FLOAT);
        } else {
            this->nextChar();
            this->nextChar();
            int startPos = this->pos;

            while(isxdigit(this->peek()))
                this->nextChar();

            std::string tokText = this->source.substr(startPos, (this->pos + 1) - startPos);
            token = Token(std::to_string((int)strtol(tokText.c_str(), NULL, 16)), TOK_NUMBER);
        }
    } else if(this->current == '@') {
        int startPos = this->pos + 1;
        while(isalpha(this->peek()) || this->peek() == '_') {
            this->nextChar();
        }
        std::string tokText = this->source.substr(startPos, (this->pos + 1) - startPos);
        token = Token(tokText, TOK_TYPE);
    } else if(this->current == '#') {
        token = Token(this->current, TOK_HASH);
    } else if(isalpha(this->current)) {
        int startPos = this->pos;
        while(isalnum(this->peek()) || this->peek() == '_') {
            this->nextChar();
        }
        std::string tokText = this->source.substr(startPos, (this->pos + 1) - startPos);
        TokenType keyword = Token::checkIfKeyword(tokText);
        token = Token(tokText, keyword);
    } else if(this->current == '(')
        token = Token(this->current, TOK_LPAREN);
    else if(this->current == ')')
        token = Token(this->current, TOK_RPAREN);
    else if(this->current == '{')
        token = Token(this->current, TOK_LBRACE);
    else if(this->current == '}')
        token = Token(this->current, TOK_RBRACE);
    else if(this->current == ';')
        token = Token(this->current, TOK_SEMI);
    else if(this->current == ',')
        token = Token(this->current, TOK_COMMA);
    else if(this->current == '.')
        token = Token(this->current, TOK_DOT);
    else if(this->current == '[')
        token = Token(this->current, TOK_LSQUARE);
    else if(this->current == ']')
        token = Token(this->current, TOK_RSQUARE);
    else if(this->current == ':')
        token = Token(this->current, TOK_COLON);
    else if(this->current == '\n') {
        token = Token(this->current, TOK_NEWLINE);
    } else if(this->current == '\0')
        token = Token("", TOK_EOF);
    else {
        char* temp = (char*)malloc(32);
        sprintf(temp, "unexpected character `%c`", this->current);
        this->abort(temp);
    }

    this->nextChar();
    return token;
}