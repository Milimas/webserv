#include "includes/Location.hpp"
#include "includes/Server.hpp"
#include "includes/Error.hpp"
#include "includes/parser/Parser.hpp"
#include "includes/parser/Lexer.hpp"
#include "includes/Multiplex.hpp"

#include <cstdlib>
#include <algorithm>

// static void print(Server s)
// {
// 	std::cerr << s << std::endl ;
// }



int main( int argc, char **argv )
{
	Parser::servers_t servers ;
	if (argc != 2)
	{
		std::cerr << USAGE << std::endl ;
		return (2) ;
	}
	(void) argv ;
	Lexer::tokens_t tokens ;
	try
	{
		tokens = Lexer::checkSyntax(argv[1]) ;
		servers = Parser::parse(tokens) ;
		// std::for_each(servers.begin(), servers.end(), print) ;
		Multiplex::setup(servers) ;
		// hani hadi nzid servers f func dyal start  (by mballa)
		Multiplex::start() ;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl ;
	}

	return (0) ;
}
