#include "parser.hpp"
#include <sstream>
#include <iterator>
#include <unistd.h>

//#define JOIN(s, args) (std::copy(args.begin(),args.end(), std::ostream_iterator<std::string>(s,", ")))

std::string join(const std::set<std::string>& v, const std::string& delimiter = ",") {
    std::string out;
    if (auto i = v.begin(), e = v.end(); i != e) {
        out += *i++;
        for (; i != e; ++i) out.append(delimiter).append(*i);
    }
    return out;
}

Parser::Parser(std::string source, std::string outfile, bool usestdlib, bool includeMetal) : lexer(source), emitter(outfile), curToken("", TOK_EOF), peekToken("", TOK_EOF) {
    this->usestdlib = usestdlib;
    this->includeMetal = includeMetal;

    this->globals.insert({"true", "bool"});
    this->globals.insert({"false", "bool"});
    this->globals.insert({"nil", "voip"});

    this->types.insert("int");
    this->types.insert("int8");
    this->types.insert("int16");
    this->types.insert("int32");
    this->types.insert("int64");
    this->types.insert("float");
    this->types.insert("char");
    this->types.insert("uint8");
    this->types.insert("uint16");
    this->types.insert("uint32");
    this->types.insert("uint64");
    this->types.insert("string");
    this->types.insert("stringp");
    this->types.insert("void");
    this->types.insert("voidp");
    this->types.insert("bool");

    this->nextToken();
    this->nextToken();
}

bool Parser::checkToken(TokenType kind) {
    return kind == this->curToken.getType();
}

bool Parser::checkPeek(TokenType kind) {
    return kind == this->peekToken.getType();
}

static std::string resolveToken(TokenType kind) {
    switch(kind) {
        case TOK_ASTERISK: return "ASTERISK";
        case TOK_BAND: return "BAND";
        case TOK_BANGEQ: return "BANGEQ";
        case TOK_BOR: return "BOR";
        case TOK_BREAK: return "BREAK";
        case TOK_CASE: return "CASE";
        case TOK_CHAR: return "CHAR";
        case TOK_COLON: return "COLON";
        case TOK_COMMA: return "COMMA";
        case TOK_CONTINUE: return "CONTINUE";
        case TOK_DOT: return "DOT";
        case TOK_ELIF: return "ELIF";
        case TOK_ELSE: return "ELSE";
        case TOK_ENUM: return "ENUM";
        case TOK_EOF: return "EOF";
        case TOK_EQ: return "EQ";
        case TOK_EQEQ: return "EQEQ";
        case TOK_EXTERN: return "EXTERN";
        case TOK_FLOAT: return "FLOAT";
        case TOK_FOR: return "FOR";
        case TOK_FUNC: return "FUNC";
        case TOK_GOTO: return "GOTO";
        case TOK_GT: return "GT";
        case TOK_GTEQ: return "GTEQ";
        case TOK_HASH: return "HASH";
        case TOK_HEADER: return "HEADER";
        case TOK_IDENT: return "IDENT";
        case TOK_IF: return "IF";
        case TOK_INCLUDE: return "INCLUDE";
        case TOK_LABEL: return "LABEL";
        case TOK_LAND: return "LAND";
        case TOK_LBRACE: return "LBRACE";
        case TOK_LET: return "LET";
        case TOK_LOR: return "LOR";
        case TOK_LPAREN: return "LPAREN";
        case TOK_LSHIFT: return "LSHIFT";
        case TOK_LSQUARE: return "LSQUARE";
        case TOK_LT: return "LT";
        case TOK_LTEQ: return "LTEQ";
        case TOK_MINUS: return "MINUS";
        case TOK_NEWLINE: return "NEWLINE";
        case TOK_NUMBER: return "NUMBER";
        case TOK_PLUS: return "PLUS";
        case TOK_RBRACE: return "RBRACE";
        case TOK_RETURN: return "RETURN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_RSHIFT: return "RSHIFT";
        case TOK_RSQUARE: return "RSQUARE";
        case TOK_SEMI: return "SEMI";
        case TOK_SLASH: return "SLASH";
        case TOK_STRING: return "STRING";
        case TOK_STRUCT: return "STRUCT";
        case TOK_SWITCH: return "SWITCH";
        case TOK_TYPE: return "TYPE";
        case TOK_TYPEDEF: return "TYPEDEF";
        case TOK_WHILE: return "WHILE";
    }
    return "EOF";
}

std::string gen_random(const int len) {
    
    std::string tmp_s;
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    srand( (unsigned) time(NULL) * getpid());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) 
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    
    
    return tmp_s;
}  

void Parser::abort(std::string reason) {
    fprintf(stderr, "akucc: \033[31;1mparser\033[0m:%d: %s\n", this->lexer.line - 1, reason.c_str());
    std::exit(1);
}

void Parser::note(std::string reason) {
    fprintf(stderr, "akucc: \033[36;1mnote\033[0m:%d %s\n", this->lexer.line - 1, reason.c_str());
}

void Parser::warn(std::string reason) {
    fprintf(stderr, "akucc: \033[35;1mwarning\033[0m:%d %s\n", this->lexer.line - 1, reason.c_str());
}

void Parser::match(TokenType kind) {
    if(!this->checkToken(kind)) {
        char* t = (char*)malloc(32);
        sprintf(t, "expected `%s` got `%s`", resolveToken(kind).c_str(), this->curToken.getText().c_str());
        this->abort(t);
    }
    this->nextToken();
}

void Parser::nextToken() {
    this->curToken = this->peekToken;
    this->peekToken = this->lexer.getToken();
}

void Parser::nl() {
    this->match(TOK_NEWLINE);
    while(this->checkToken(TOK_NEWLINE)) {
        this->nextToken();
    }
}

void Parser::statement(std::string scope) {
    if(this->checkToken(TOK_FUNC)) {
        this->nextToken();
        std::string fctype = this->curToken.getText();
        this->match(TOK_TYPE);
        if(!validType(fctype)) {
            char* t = (char*)malloc(32);
            sprintf(t, "invalid type `%s`", fctype.c_str());
            this->abort(t);   
        }

        if(this->checkToken(TOK_IDENT)) {
            std::string name = this->curToken.getText();
            if(!this->globals.count(name)) {
                this->globals.insert({name, fctype});
            } else {
                char* t = (char*)malloc(32);
                sprintf(t, "function `%s` already declared", this->curToken.getText().c_str());
                this->abort(t);
            }
            this->nextToken();
            this->match(TOK_LPAREN);
            std::set<std::string> args;
            if(this->checkToken(TOK_TYPE)) {
                std::string ftype = this->curToken.getText();
                if(!validType(ftype)) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "invalid type `%s`", ftype.c_str());
                    this->abort(t);
                }
                this->globals[name] = ftype;
                this->nextToken();
                if(this->checkToken(TOK_IDENT)) {
                    if(ftype != "string") {
                        char* t = (char*)malloc(32);
                        sprintf(t, "%s %s", ftype.c_str(), this->curToken.getText().c_str());
                        args.insert(t);
                    } else {
                        char* t = (char*)malloc(32);
                        sprintf(t, "char* %s", this->curToken.getText().c_str());
                        args.insert(t);
                    }
                    this->locals[name][this->curToken.getText()] = ftype;
                    this->nextToken();
                    if(!this->checkToken(TOK_RPAREN)) {
                        if(this->checkToken(TOK_COMMA)) {
                            this->nextToken();
                        }
                        while(this->checkToken(TOK_TYPE)) {
                            if(this->checkPeek(TOK_IDENT)) {
                                ftype = this->curToken.getText();
                                if(!this->validType(ftype)) {
                                    char* t = (char*)malloc(32);
                                    sprintf(t, "invalid type `%s`", ftype.c_str());
                                    this->abort(t);
                                }
                                this->nextToken();
                                if(ftype != "string") {
                                    char* t = (char*)malloc(32);
                                    sprintf(t, "%s %s", ftype.c_str(), this->curToken.getText().c_str());
                                    args.insert(t);
                                } else {
                                    char* t = (char*)malloc(32);
                                    sprintf(t, "char* %s", this->curToken.getText().c_str());
                                    args.insert(t);
                                }
                                this->locals[scope][this->curToken.getText()] = ftype;
                                this->nextToken();
                                if(this->checkToken(TOK_COMMA)) {
                                    this->nextToken();
                                }
                            }
                        }
                    }
                }
            }
            this->match(TOK_RPAREN);
            this->match(TOK_LBRACE);

            if(fctype == "string")
                fctype = "char*";
            this->emitter.function(fctype.c_str());
            this->emitter.function(" ");
            this->emitter.function(name.c_str());
            this->emitter.function("(");
            std::string s = join(args, ", ");
            this->emitter.function(s.c_str());
            this->emitter.function(")");
            this->emitter.functionLine("{");
                
            while(this->checkToken(TOK_NEWLINE))
                this->nextToken();

            while(!this->checkToken(TOK_RBRACE))
                this->statement(name);

            this->match(TOK_RBRACE);
            this->emitter.functionLine("}");
        }
    } else if(this->checkToken(TOK_WHILE)) {
        this->nextToken();
        this->emitter.function("while(");
        this->match(TOK_LPAREN);
        this->comparison(scope);

        this->match(TOK_RPAREN);
        this->match(TOK_LBRACE);

        this->emitter.functionLine("){");

        while(this->checkToken(TOK_NEWLINE))
            this->nextToken();

        while(!this->checkToken(TOK_RBRACE))
            this->statement(scope);

        this->match(TOK_RBRACE);
        this->emitter.functionLine("}");
    } else if(this->checkToken(TOK_FOR)) {
        // TODO: for loop
    } else if(this->checkToken(TOK_IF)) {
        this->nextToken();
        this->match(TOK_LPAREN);
        this->emitter.function("if(");
        this->comparison(scope);

        this->match(TOK_RPAREN);
        this->match(TOK_LBRACE);
        this->emitter.functionLine("){");

        while(this->checkToken(TOK_NEWLINE))
            this->nextToken();

        while(!this->checkToken(TOK_RBRACE))
            this->statement(scope);

        this->match(TOK_RBRACE);
        this->emitter.function("}");
        if(this->checkToken(TOK_ELIF)) {
            while(this->checkToken(TOK_ELIF)) {
                this->nextToken();
                this->match(TOK_LPAREN);
                this->emitter.function(" else if(");
                this->comparison(scope);
                this->match(TOK_RPAREN);
                this->emitter.functionLine("){");
                this->match(TOK_LBRACE);

                while(this->checkToken(TOK_NEWLINE))
                    this->nextToken();

                while(!this->checkToken(TOK_RBRACE))
                    this->statement(scope);

                if(!this->checkToken(TOK_RBRACE)) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "expected RBRACE got `%s`", resolveToken(this->curToken.getType()).c_str());
                    this->abort(t);
                }
                this->emitter.function("}");
                this->nextToken();
            }

            if(this->checkToken(TOK_ELSE)) {
                this->nextToken();
                this->emitter.functionLine(" else {");
                this->match(TOK_LBRACE);

                while(this->checkToken(TOK_NEWLINE))
                    this->nextToken();

                while(!this->checkToken(TOK_RBRACE))
                    this->statement(scope);

                this->match(TOK_RBRACE);
                this->emitter.function("}\n");
            }
        } else if(this->checkToken(TOK_ELSE)) {
            this->nextToken();
            this->emitter.functionLine(" else {");
            this->match(TOK_LBRACE);

            while(this->checkToken(TOK_NEWLINE))
                this->nextToken();

            while(!this->checkToken(TOK_RBRACE))
                this->statement(scope);

            this->match(TOK_RBRACE);
            this->emitter.functionLine("}");
        } else {
            this->emitter.function("\n");
        }
    } else if(this->checkToken(TOK_SWITCH)) {
        this->nextToken();
        this->match(TOK_LPAREN);
        this->emitter.function("switch(");
        this->expression(scope);
        this->match(TOK_RPAREN);
        this->match(TOK_LBRACE);
        this->emitter.functionLine("){");

        while(this->checkToken(TOK_NEWLINE))
            this->nextToken();

        while(this->checkToken(TOK_CASE)) {
            this->nextToken();
            std::string rid = "case" + gen_random(5);
            this->emitter.function("case ");
            this->primary(scope, false);
            this->emitter.functionLine(": {");
            this->match(TOK_COLON);
            this->match(TOK_LBRACE);
            while(this->checkToken(TOK_NEWLINE))
                this->nextToken();

            while(!this->checkToken(TOK_RBRACE))
                this->statement(rid);
            this->emitter.functionLine("}");
            this->match(TOK_RBRACE);
            if(this->checkToken(TOK_NEWLINE))
                this->nextToken();
        }
        this->match(TOK_RBRACE);
        this->emitter.functionLine("}");
    } else if(this->checkToken(TOK_INCLUDE)) {
        this->nextToken();
        this->cheaders.insert({this->curToken.getText()});
        this->match(TOK_STRING);
    } else if(this->checkToken(TOK_EXTC)) {
        this->match(TOK_EXTC);
        std::string name = this->curToken.getText();
        this->match(TOK_IDENT);
        if(this->checkToken(TOK_LPAREN)) {
            if(this->locals[scope].count(name) == 0) {
                if(this->globals.count(name) == 0) {
                    char* t = (char*)malloc(128);
                    sprintf(t, "function `%s` does not exist, extc is declared therefore assuming external C function.", name.c_str());
                    this->warn(t);
                    free(t);
                }
            }
            this->nextToken();
            this->emitter.function(name.c_str());
            this->emitter.function("(");
            while(!this->checkToken(TOK_RPAREN)) {
                this->expression(scope);
                if(this->checkToken(TOK_COMMA)) {
                    this->emitter.function(", ");
                    this->nextToken();
                } else if(this->checkToken(TOK_SEMI)) {
                    break;
                }
            }
            this->match(TOK_RPAREN);
            this->emitter.functionLine(");");
        } else if(this->checkToken(TOK_EQ)) {
            this->match(TOK_EQ);
            if(this->locals[scope].count(name) == 0) {
                char* t = (char*)malloc(32);
                sprintf(t, "symbol `%s` is not declared", name.c_str());
                this->abort(t);
            }
            this->emitter.function(name.c_str());
            this->emitter.function(" = ");
            this->expression(scope);
            this->emitter.functionLine(";");
        } else if(this->checkToken(TOK_PLUS)) {
            this->match(TOK_PLUS);
            this->match(TOK_PLUS);
            this->emitter.function(name.c_str());
            this->emitter.function("++;");
        } else if(this->checkToken(TOK_MINUS)) {
            this->match(TOK_MINUS);
            this->match(TOK_MINUS);
            this->emitter.function(name.c_str());
            this->emitter.function("--;");
        }
    } else if(this->checkToken(TOK_IDENT)) {
        std::string name = this->curToken.getText();
        this->nextToken();
        if(this->checkToken(TOK_LPAREN)) {
            if(this->locals[scope].count(name) == 0) {
                if(this->globals.count(name) == 0) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "function `%s` does not exist", name.c_str());
                    this->abort(t);
                }
            }
            this->nextToken();
            this->emitter.function(name.c_str());
            this->emitter.function("(");
            while(!this->checkToken(TOK_RPAREN)) {
                this->expression(scope);
                if(this->checkToken(TOK_COMMA)) {
                    this->emitter.function(", ");
                    this->nextToken();
                }
            }
            this->match(TOK_RPAREN);
            this->emitter.functionLine(");");
        } else if(this->checkToken(TOK_EQ)) {
            this->match(TOK_EQ);
            if(this->locals[scope].count(name) == 0) {
                char* t = (char*)malloc(32);
                sprintf(t, "symbol `%s` is not declared", name.c_str());
                this->abort(t);
            }
            this->emitter.function(name.c_str());
            this->emitter.function(" = ");
            this->expression(scope);
            this->emitter.functionLine(";");
        } else if(this->checkToken(TOK_PLUS)) {
            this->match(TOK_PLUS);
            this->match(TOK_PLUS);
            this->emitter.function(name.c_str());
            this->emitter.function("++;");
        } else if(this->checkToken(TOK_MINUS)) {
            this->match(TOK_MINUS);
            this->match(TOK_MINUS);
            this->emitter.function(name.c_str());
            this->emitter.function("--;");
        }
    } else if(this->checkToken(TOK_RETURN)) {
        this->emitter.function("return ");
        this->match(TOK_RETURN);
        this->expression(scope);
        this->emitter.functionLine(";");
    } else if(this->checkToken(TOK_BREAK)) {
        this->nextToken();
        this->emitter.functionLine("break;");
    } else if(this->checkToken(TOK_CONTINUE)) {
        this->nextToken();
        this->emitter.functionLine("continue;");
    } else if(this->checkToken(TOK_FROM)) {
        this->nextToken();
        std::string import = this->curToken.getText();
        this->match(TOK_STRING);
        this->usings.insert(import);
        this->match(TOK_LBRACE);
        this->match(TOK_NEWLINE);
        while(!this->checkToken(TOK_RBRACE)) {
            std::string type = this->curToken.getText();
            this->match(TOK_TYPE);
            std::string name = this->curToken.getText();
            this->match(TOK_IDENT);
            this->match(TOK_SEMI);
            this->match(TOK_NEWLINE);
            this->globals.insert({name, type});
        }
        this->match(TOK_RBRACE);
    } else if(this->checkToken(TOK_TYPE)) {
        std::string ftype = this->curToken.getText();
        if(!validType(ftype)) {
            char* t = (char*)malloc(32);
            sprintf(t, "invalid type `%s`", ftype.c_str());
            this->abort(t);
        }
        this->nextToken();

        std::string name = this->curToken.getText();
        this->match(TOK_IDENT);
        if(this->locals[scope].count(name) == 1) {
            char* t = (char*)malloc(32);
            sprintf(t, "symbol `%s` already declared in scope", name.c_str());
            this->abort(t);
        } else if(this->globals.count(name) == 1) {
            char* t = (char*)malloc(32);
            sprintf(t, "symbol `%s` already declared", name.c_str());
            this->abort(t);
        }

        this->match(TOK_EQ);
        if(!this->checkToken(TOK_LBRACE)) {
            if(ftype != "string") {
                this->emitter.function(ftype.c_str());
                this->emitter.function(" ");
                this->emitter.function(name.c_str());
                this->emitter.function(" = ");
            } else {
                this->emitter.function("char* ");
                this->emitter.function(name.c_str());
                this->emitter.function(" = ");
            }
            this->expression(scope);
            this->emitter.functionLine(";");
            this->locals[scope].insert(std::make_pair(name, ftype));
            //printf("%s\n", this->locals["main"]["hi"].c_str());
        } else {
            this->nextToken();
            if(!this->types.count(ftype)) {
                this->abort("attempting to initialize a type that doesn't exist");
            }
            this->emitter.function((ftype + " " + name + " = { ").c_str());
            while(!this->checkToken(TOK_RBRACE)) {
                this->expression(scope);
                if(this->checkToken(TOK_COMMA)) {
                    this->emitter.function(", ");
                    this->nextToken();
                }
            }

            this->emitter.function(" };\n");
            this->locals[scope].insert(std::make_pair(name, ftype));
            this->match(TOK_RBRACE);
        }
    } else {
        this->abort("invalid statement");
    }

    this->match(TOK_SEMI);
    if(this->checkToken(TOK_NEWLINE))
        this->nl();
}

void Parser::term(std::string scope) {
    this->unary(scope);

    while(this->checkToken(TOK_ASTERISK) || this->checkToken(TOK_SLASH)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->unary(scope);
    }
}

void Parser::unary(std::string scope) {
    if(this->checkToken(TOK_PLUS) || this->checkToken(TOK_MINUS)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
    }
    this->primary(scope, true);
}

void Parser::comparison(std::string scope) {
    this->expression(scope);

    if(this->isComparisonOperator()) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->expression(scope);
    }

    while(this->isComparisonOperator()) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->expression(scope);
    }
}

void Parser::shift(std::string scope) {
    this->term(scope);

    while(this->checkToken(TOK_RSHIFT) || this->checkToken(TOK_LSHIFT)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->term(scope);
    }
}

void Parser::binary(std::string scope) {
    this->shift(scope);

    while(this->checkToken(TOK_BAND) || this->checkToken(TOK_BOR)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->shift(scope);
    }
}

void Parser::primary(std::string scope, bool allowStr) {
    if(this->checkToken(TOK_NUMBER)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
    } else if(this->checkToken(TOK_FLOAT)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
    } else if(this->checkToken(TOK_EXTC)) {
        this->nextToken();
        std::string ename = this->curToken.getText();
        if(this->checkPeek(TOK_LPAREN)) { // func call
            if(this->locals[scope].count(ename) == 0) {
                if(this->globals.count(ename) == 0) {
                    char* t = (char*)malloc(128);
                    sprintf(t, "function `%s` does not exist, extc is declared therefore assuming external C function.", ename.c_str());
                    this->warn(t);
                    free(t);
                }
            }
            this->nextToken();
            this->emitter.function(ename.c_str());
            this->emitter.function("(");
            this->match(TOK_LPAREN);
            while(!this->checkToken(TOK_RPAREN)) {
                this->expression(scope);
                if(this->checkToken(TOK_COMMA)) {
                    this->emitter.function(", ");
                    this->nextToken();
                }
            }

            this->match(TOK_RPAREN);
            this->emitter.function(")");
        }   
    } else if(this->checkToken(TOK_IDENT)) {
        std::string ename = this->curToken.getText();
        if(this->checkPeek(TOK_LPAREN)) { // func call
            if(this->locals[scope].count(ename) == 0) {
                if(this->globals.count(ename) == 0) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "function `%s` does not exist", ename.c_str());
                    this->abort(t);
                }
            }
            this->nextToken();
            this->emitter.function(ename.c_str());
            this->emitter.function("(");
            while(!this->checkToken(TOK_RPAREN)) {
                this->expression(scope);
                if(this->checkToken(TOK_COMMA)) {
                    this->emitter.function(", ");
                    this->nextToken();
                }
            }

            this->match(TOK_RPAREN);
            this->emitter.function(")");
        } else if(this->checkPeek(TOK_DOT)) { // struct property
            if(this->locals[scope].count(ename) == 0) {
                if(this->globals.count(ename) == 0) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "referencing a symbol that isn't defined yet or doesn't exist `%s`", ename.c_str());
                    this->abort(t);
                }
            }
            this->nextToken();
            this->emitter.function(ename.c_str());
            while(this->checkToken(TOK_DOT)) {
                this->nextToken();
                std::string prop = this->curToken.getText();
                this->match(TOK_IDENT);
                this->emitter.function(".");
                this->emitter.function(prop.c_str());
            }
            printf("struct > %s cur: %s\n", this->curToken.getText().c_str(), resolveToken(this->curToken.getType()).c_str());
        } else if(this->checkPeek(TOK_LSQUARE)) { // pointer/array index referencing
            this->nextToken();
            this->match(TOK_LSQUARE);
            if(this->locals[scope].count(ename) == 0) {
                if(this->globals.count(ename) == 0) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "symbol `%s` does not exist", ename.c_str());
                    this->abort(t);
                }
            }
            this->emitter.function(ename.c_str());
            this->emitter.function("[");
            this->expression(scope);
            this->match(TOK_RSQUARE);
            this->emitter.function("]");
        } else if(this->checkPeek(TOK_PLUS)) {
            this->nextToken();
            this->match(TOK_PLUS);
            this->match(TOK_PLUS);
            this->emitter.function(ename.c_str());
            this->emitter.function("++");
        } else if(this->checkPeek(TOK_MINUS)) {
            this->nextToken();
            this->match(TOK_MINUS);
            this->match(TOK_MINUS);
            this->emitter.function(ename.c_str());
            this->emitter.function("--");
        } else {
            if(this->locals[scope].count(ename) == 0) {
                if(this->globals.count(ename) == 0) {
                    char* t = (char*)malloc(32);
                    sprintf(t, "symbol `%s` does not exist", ename.c_str());
                    this->abort(t);
                }
            }
            this->emitter.function(ename.c_str());
            this->nextToken();
        }
    } else if(this->checkToken(TOK_STRING)) {
        if(allowStr) {
            this->emitter.function("\"");
            this->emitter.function(this->curToken.getText().c_str());
            this->emitter.function("\"");
            this->nextToken();
        } else {
            this->abort("illegal string encountered in switch statement");  
        }
    } else if(this->checkToken(TOK_CHAR)) {
        this->emitter.function("\'");
        this->emitter.function(this->curToken.getText().c_str());
        this->emitter.function("\'");
        this->nextToken();
    } else {
        fprintf(stderr, "unexpected token at primary parsing `%s`\n", this->curToken.getText().c_str());
        this->abort("^");
    }
}

void Parser::expression(std::string scope) {
    this->binary(scope);

    while(this->checkToken(TOK_PLUS) || this->checkToken(TOK_MINUS)) {
        this->emitter.function(this->curToken.getText().c_str());
        this->nextToken();
        this->binary(scope);
    }
}

void Parser::program() {
    this->emitter.headerLine("// generated by akucc");
    if(this->usestdlib) {
        //this->emitter.headerLine(akulib);
    } else {
        this->emitter.headerLine("// --nostdlib option was passed");
    }
    if(this->includeMetal) {
        //this->emitter.headerLine(metallib);
    }
    this->emitter.headerLine(
        "// built-in type names\n"
        "#include <stdint.h>\n"
        "typedef int8_t int8;\n"
        "typedef int16_t int16;\n"
        "typedef int32_t int32;\n"
        "typedef int64_t int64;\n"
        "typedef uint8_t uint8;\n"
        "typedef uint16_t uint16;\n"
        "typedef uint32_t uint32;\n"
        "typedef uint64_t uint64;\n"
        "typedef char **stringp;\n"
        "typedef void *voidp;\n"
        "#define true 1\n"
        "#define false 0\n"
        "#define nil (voidp)0\n");

    this->emitter.headerLine("// stdlib headers\n\n");

    this->emitter.headerLine("// user headers\n");

    this->emitter.emit("\n\n");

    while(this->checkToken(TOK_NEWLINE))
        this->nextToken();

    while(!this->checkToken(TOK_EOF))
        this->statement("main");
    
    for(std::string use : this->usings) {
        char* t = (char*)malloc(64);
        sprintf(t, "#include \"%s.h\"", use.c_str());
        this->emitter.headerLine(t);
        free(t);
    }

    for(std::string cheader : this->cheaders) {
        char* t = (char*)malloc(64);
        sprintf(t, "#include \"%s\"", cheader.c_str());
        this->emitter.headerLine(t);
        free(t);
    }
    this->emitter.headerLine("\n\n");
}