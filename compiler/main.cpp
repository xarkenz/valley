#include <iostream>
#include <fstream>

#include "errors.hpp"
#include "tokenizer.hpp"
#include "compiler_context.hpp"
#include "expression_manager.hpp"
#include "statement_manager.hpp"

int main(int argc, char* argv[]) {
  using namespace valley;

  if (argc == 1)
    return 0;
  char* filename = argv[1];
  
  compiler_context context;
  context.create_identifier("true", type_registry::bool_handle(), true, true);
  context.create_identifier("false", type_registry::bool_handle(), true, true);
  context.create_identifier("null", type_registry::void_handle(), true, true);

  std::ifstream reader(filename);
  reader.seekg(0);
  get_character input = [&reader]() {
    return reader.get();
  };

  try {
    push_back_stream stream(input);
    tokens_iterator it(stream);
    std::vector<statement::ptr> code;
    code = parse_code_statements(context, it, code);
    for (size_t i = 0; i < code.size(); i++) {
      if (code.at(i).get()) {
        std::cout << "--- " << std::to_string(i + 1) << " ---" << std::endl << code[i]->to_string() << std::endl;
      }
    }
  } catch (error& err) {
    reader.seekg(0);
    err.format(input, std::cerr);
  }
  
  return 0;
}
