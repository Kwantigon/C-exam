#ifndef HEADER
#define HEADER

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>

class Macro;

using Token = std::string;
using Tokenized_line = std::vector<Token>;
using Macro_map = std::unordered_map<Token, Macro>;

bool is_proper_word(const Token& t);
void process_line(std::string& line, Macro_map& macro_definitions);
Tokenized_line replace_line(std::deque<Token>& t_line, const Macro_map& m);
void print_tokenized_line(const Tokenized_line& t_line);

#endif // !HEADER
