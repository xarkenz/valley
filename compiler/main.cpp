#include <fstream>
#include <iostream>

#include "errors.hpp"
#include "tokenizer.hpp"
#include "compiler_context.hpp"
#include "statement_manager.hpp"

int main(int argc, char* argv[]) {
  using namespace valley;

  if (argc == 1) {
    std::cout << "Enter filename as first command line argument." << std::endl;
    return 0;
  }
  char* filename = argv[1];
  
  CompilerContext context;
  context.createIdentifier("true", TypeRegistry::boolHandle(), true, true);
  context.createIdentifier("false", TypeRegistry::boolHandle(), true, true);
  context.createIdentifier("null", TypeRegistry::voidHandle(), true, true);

  std::ifstream reader(filename);
  reader.seekg(0);
  CharGetter input = [&reader]() {
    return reader.get();
  };

  try {
    PushBackStream stream(input);
    TokenIterator it(stream);
    std::vector<Statement::Ptr> code = parseCode(context, it);
    for (size_t i = 0; i < code.size(); i++) {
      if (code.at(i).get()) {
        std::cout << "--- " << std::to_string(i + 1) << " ---" << std::endl << code[i]->toString() << std::endl;
      }
    }
  } catch (Error& err) {
    reader.seekg(0);
    err.format(input, std::cerr);
  }
  
  return 0;
}