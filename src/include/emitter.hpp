#include <string>

class Emitter {
    private:
        std::string fullPath;
        std::string headers;
        std::string macros;
        std::string functions;
        std::string code;

    public:
        Emitter(std::string fullPath);
        void emit(const char* code);
        void emitLine(const char* code);
        void headerLine(const char* code);
        void macro(const char* code);
        void functionLine(const char* code);
        void function(const char* code);
        void writeFile();
};