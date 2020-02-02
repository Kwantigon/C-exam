#include "Header.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <array>

class Macro
{
public:
	// constructor
	Macro(size_t p, std::vector<Token> t) : parameter_count(p), tokens(t)
	{}

	size_t parameter_count;
	std::vector<Token> tokens;
};

bool is_proper_word(const Token& t)
{
	for (size_t i = 0; i < t.size(); i++)
	{
		if (!isalpha(t.at(i)))
		{
			return false;
		}
	}
	return true;
}

void process_line(std::string& line, Macro_map& macro_definitions)
{
	std::stringstream line_stream(line);
	Token first_token;
	Token second_token;
	line_stream >> first_token;
	if (is_proper_word(first_token))
	{
		line_stream >> second_token;
		if (second_token == "=")
		{
			// a macro definition. Add or modify it
			size_t parameter_count = 0;
			std::vector<Token> macro_params;
			Token t;
			while (line_stream.good())
			{
				line_stream >> t;
				if (is_proper_word(t))
				{
					macro_params.push_back(t);
				}
				else
				{
					// Wasn't a word. Might be a number
					try
					{
						int number = std::stoi(t);
						if (number < 1)
						{
							std::cerr << "Parameter number was less than one." << std::endl;
							return;
						}

						if (number > parameter_count)
						{
							parameter_count = number;
						}

						macro_params.push_back(t);
					}
					catch (std::invalid_argument e)
					{
						// was neither a valid word nor a number
						std::cerr << "Invalid macro definition" << std::endl;
						return;
					}
				}
			}
			// check just to be sure
			if (macro_params.size() == 0)
			{
				std::cerr << "Invalid macro definition." << std::endl;
				return;
			}

			Macro new_macro(parameter_count, macro_params);

			// find the macro name to add or modify
			auto map_itr = macro_definitions.find(first_token);
			if (map_itr == macro_definitions.end())
			{
				// didn't find it. Add a new one
				macro_definitions.insert(std::make_pair(first_token, new_macro));
			}
			else
			{
				// found it. Replace the old one
				map_itr->second = new_macro;
			}

		}
		else // not a definition. It's a line to be replaced
		{
			// line_stream bad ==> only 1 word on the line

			// put all tokens into a deque<Token> and call replace_line()
			std::deque<Token> tokens;
			tokens.push_back(first_token);
			if (is_proper_word(second_token))
			{
				tokens.push_back(second_token);
			}
			Token t;
			while (line_stream.good())
			{
				line_stream >> t;
				if (!is_proper_word(t))
				{
					// invalid input. Don't do anything
					std::cerr << "Invalid input." << std::endl;
					return;
				}
				else
				{
					// is a word. Push it to tokens
					tokens.push_back(t);
				}
			}

			// got all the tokens. Now call replace
			Tokenized_line line_to_print = replace_line(tokens, macro_definitions);
			print_tokenized_line(line_to_print);
		}
	}
	else //first_token is invalid
	{
		std::cerr << "Invalid input." << std::endl;
		return;
	}
}

Tokenized_line replace_line(std::deque<Token>& t_line, const Macro_map& m)
{
	Tokenized_line ret_line;
	Token t;
	size_t replacement_count = 0;

	while (!t_line.empty())
	{
		t = t_line.front();
		t_line.pop_front();

		auto map_itr = m.find(t);
		if (map_itr == m.end())
		{
			// this token is not defined as a macro. Move onto the next
			ret_line.push_back(t);
			continue;
		}
		else
		{
			// token is defined as a macro. Replace it
			Macro macro = map_itr->second;
			
			// stop and return if there are not enough parameters left
			if (t_line.size() < macro.parameter_count)
			{
				std::cerr << "Not enough parameters." << std::endl;
				return ret_line;
			}
			
			// get the parameters
			std::vector<Token> parameters;
			parameters.push_back("dummy token"); // token at index 0 is never used
			for (size_t i = 1; i <= macro.parameter_count; i++)
			{
				// get tokens from the top of the queue (t_line)
				parameters.push_back(t_line.at(i - 1)); // indices are shifted by 1
			}
			// pop the parameters that will be replaced
			for (size_t i = 0; i < macro.parameter_count; i++)
			{
				t_line.pop_front();
			}

			// replace by pushing the tokens to the top of the queue (t_line)
			size_t end_of_macro_line = macro.tokens.size() - 1;
			for (size_t i = end_of_macro_line; i > 0; i--) // ToDo: fix INFINITE LOOP
			{
				Token macro_t = macro.tokens[i];
				try
				{
					// check if it's a number
					int number = std::stoi(macro_t);

					// it can't be a negative number unless there was an overflow
					t_line.push_front(parameters[number]);
				}
				catch (std::invalid_argument e)
				{
					// was not a number. Just push to the front of queue
					t_line.push_front(macro_t);
				}
			}
			// do the same for the last token (at index 0)
			// <Copied code>
			Token macro_t = macro.tokens[0];
			try
			{
				// check if it's a number
				int number = std::stoi(macro_t);

				// it can't be a negative number unless there was an overflow
				t_line.push_front(parameters[number]);
			}
			catch (std::invalid_argument e)
			{
				// was not a number. Just push to the front of queue
				t_line.push_front(macro_t);
			}
			replacement_count++;
		}

		if (replacement_count == 1000)
		{
			break;
		}
	}
	return ret_line;
}

void print_tokenized_line(const Tokenized_line& t_line)
{
	// print the first token without leading space
	std::cout << t_line[0];

	// print the rest with a leading space
	for (size_t i = 1; i < t_line.size(); i++)
	{
		std::cout << " " << t_line[i];
	}
	
	// print new line
	std::cout << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Not enough parameters." << std::endl;
		return 42;
	}

	std::ifstream file(argv[1]);
	Macro_map macro_def;
	std::string line;
	while (file.good())
	{
		std::getline(file, line);
		process_line(line, macro_def);
	}

	return 0;
}
