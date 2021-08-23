#include "emitter.hpp"
#include <fstream>

Emitter::Emitter(std::string fullPath) {
    this->fullPath = fullPath;
    this->headers = "";
    this->macros = "";
    this->functions = "";
    this->code = "";
}

void Emitter::emit(const char* code) {
    this->code = this->code + code;
}

void Emitter::emitLine(const char* code) {
    this->code = this->code + code;
    this->code = this->code + "\n";
}

void Emitter::function(const char* code) {
    this->functions = this->functions + code;
}

void Emitter::functionLine(const char* code) {
    this->functions = this->functions + code;
    this->functions = this->functions + "\n";
}

void Emitter::headerLine(const char* code) {
    this->headers = this->headers + code;
    this->headers = this->headers + "\n";
}

void Emitter::macro(const char* code) {
    this->macros = this->macros + code;
}

void Emitter::writeFile() {
    std::ofstream out(this->fullPath);
    out << this->headers + this->macros + this->functions + this->code;
    out.close();
}