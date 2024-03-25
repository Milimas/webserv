#include "../../includes/parser/Lexer.hpp"

Lexer::tokens_t Lexer::tokens ;
Lexer::iterator_t Lexer::it ;
Lexer::iterator_t Lexer::prev ;
Lexer::sym_t Lexer::sym = Lexer::ERROR ;
Lexer::sym_names_t Lexer::symNames ;
Lexer::name_sym_t Lexer::nameSyms ;

static void fillMaps( void )
{
	Lexer::symNames[Lexer::SERVER] = "SERVER" ;
	Lexer::symNames[Lexer::OCB] = "OCB" ;
	Lexer::symNames[Lexer::CCB] = "CCB" ;
	Lexer::symNames[Lexer::LOCATION] = "LOCATION" ;
	Lexer::symNames[Lexer::SEMICOLON] = "SEMICOLON" ;
	Lexer::symNames[Lexer::ERR_PAGE] = "ERR_PAGE" ;
	Lexer::symNames[Lexer::RETURN] = "RETURN" ;
	Lexer::symNames[Lexer::ALLOW_METHODS] = "ALLOW_METHODS" ;
	Lexer::symNames[Lexer::CGI] = "CGI" ;
	Lexer::symNames[Lexer::MULTIPLE_P] = "MULTIPLE_P" ;
	Lexer::symNames[Lexer::SINGLE_P] = "SINGLE_P" ;

	Lexer::nameSyms["server"] = Lexer::SERVER ;
	Lexer::nameSyms["listen"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["server_name"] = Lexer::MULTIPLE_P ;
	Lexer::nameSyms[";"] = Lexer::SEMICOLON ;
	Lexer::nameSyms["host"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["root"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["upload"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["upload_path"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["client_max_body_size"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["index"] = Lexer::MULTIPLE_P ;
	Lexer::nameSyms["error_page"] = Lexer::ERR_PAGE ;
	Lexer::nameSyms["location"] = Lexer::LOCATION ;
	Lexer::nameSyms["{"] = Lexer::OCB ;
	Lexer::nameSyms["autoindex"] = Lexer::SINGLE_P ;
	Lexer::nameSyms["allow_methods"] = Lexer::ALLOW_METHODS ;
	Lexer::nameSyms["return"] = Lexer::RETURN ;
	Lexer::nameSyms["cgi"] = Lexer::CGI ;
	Lexer::nameSyms["}"] = Lexer::CCB ;
}

/**
 * This function opens and split the text in the config file into a vector of tokens
*/
Lexer::tokens_t Lexer::lexer( const std::string& configPath )
{
	/**
	 * open file and check if file is ok to read and its not empty
	*/
	std::ifstream configFile(configPath.c_str()) ;
	if (!configFile)
		throw std::runtime_error("Couldn't open: " + configPath) ;
	if (configFile.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Confing file is empty") ;

	/**
	 * fill string stream with read buffer
	*/
	std::stringstream ss ;
	ss << configFile.rdbuf() ;
	configFile.close() ;

	/**
	 * split all the text with white space as a delimiter using stringstream
	 * and then split it again using delimiters ``{}; ''
	 * and fill the tokens vector
	*/
	std::string token ;
	while (ss >> token)
	{
		// ignore comments from config file
		if (token.find('#') != std::string::npos)
		{
			getline(ss, token) ;
			continue ;
		}
		size_t scPos = 0 ;
		size_t scPrev = 0 ;
		std::string delimiters("{}; ") ;
		// split by semicolon so the semicolon is a token by it self
		while ((scPos = token.find_first_of(delimiters, scPrev)) != std::string::npos)
		{
			if (scPos > scPrev)
				tokens.push_back(token.substr(scPrev, scPos - scPrev)) ;
			tokens.push_back(token.substr(scPos, 1)) ;
			scPrev = scPos + 1 ;
		}
		if (scPrev < token.length())
			tokens.push_back(token.substr(scPrev)) ;
	}
	fillMaps() ; // fills the maps for (symbol names)
	// std::list<std::string>::iterator it = tokens.begin() ;
	// while (it != tokens.end())
	// 	std::cerr << *prev = it << std::endl ;
	return (tokens) ;
}

std::string Lexer::getSymbolName( const sym_t& s )
{
	return (symNames[s]) ;
}

void Lexer::nextSym( void )
{
	if (it != tokens.end())
		prev = it++ ;
	sym = nameSyms[*it] ;
	// std::cerr << std::setw(10) << std::left << getSymbolName(sym) << " ";
	// std::cerr << *it << std::endl ;
}

bool Lexer::accept(sym_t s)
{
    if (sym == s) {
		do
		{
	        nextSym();
		} while (sym == SEMICOLON && s == SEMICOLON);
        return (true) ;
    }
    return (false) ;
}

bool	Lexer::expect( sym_t s )
{
	if (accept(s))
		return (true) ;
	throw std::runtime_error("expect: " + getSymbolName(s) + " unexpected symbol " + *prev) ;
	return (false) ;
}

void Lexer::signleParam( void )
{
	expect(SINGLE_P) ;
	expect(SINGLE_P) ;
	expect(SEMICOLON) ;
}

void Lexer::multipleParam( void )
{
	expect(MULTIPLE_P) ;
	expect(SINGLE_P) ;
	while (accept(SINGLE_P)) ;
	expect(SEMICOLON) ;
}

void Lexer::location( void )
{
	expect(LOCATION) ;
	std::string path(*it) ;
	expect(SINGLE_P) ;
	expect(OCB) ;
	while (sym == SINGLE_P || sym == ERR_PAGE || sym == ALLOW_METHODS || sym == CGI || sym == MULTIPLE_P || sym == RETURN )
	{
		if (sym == SINGLE_P)
			signleParam() ;
		else if (sym == MULTIPLE_P)
			multipleParam() ;
		else if (sym == ERR_PAGE)
			errPage() ;
		else if (sym == RETURN)
			_return() ;
		else if (sym == ALLOW_METHODS)
			allowMethods() ;
		else if (sym == CGI)
			cgi() ;
	}
	expect(CCB) ;
}

void Lexer::errPage( void )
{
	expect(ERR_PAGE) ;
	expect(SINGLE_P) ;
	expect(SINGLE_P) ;
	expect(SEMICOLON) ;
}

void Lexer::_return( void )
{
	expect(RETURN) ;
	expect(SINGLE_P) ;
	expect(SINGLE_P) ;
	expect(SEMICOLON) ;
}

void Lexer::allowMethods( void )
{
	expect(ALLOW_METHODS) ;
	while (accept(SINGLE_P)) ;
	expect(SEMICOLON) ;
}

void Lexer::cgi( void )
{
	expect(CGI) ;
	expect(SINGLE_P) ;
	expect(SEMICOLON) ;
}

Lexer::tokens_t Lexer::checkSyntax( const std::string& configPath )
{
	tokens = lexer(configPath) ;
	it = tokens.begin() ;
	sym = nameSyms[*it] ;
	while (it != tokens.end() && expect(SERVER))
	{
		expect(OCB) ;
		while (sym != CCB)
		{
			if (sym == LOCATION)
				location() ;
			else if (sym == ERR_PAGE)
				errPage() ;
			else if (sym == MULTIPLE_P)
				multipleParam() ;
			else
				signleParam() ;
			
		}
		expect(CCB) ;
	}
	return (tokens) ;
}