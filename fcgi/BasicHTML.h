//==============================================================================
// BasicHTML.h
// Created March 5 2012
//==============================================================================

#ifndef BASIC_HTML
#define BASIC_HTML

#include <iostream>
#include <sstream>

#include "Cgicc.h"


//==============================================================================
// Function Declarations
//==============================================================================

//static char const* webRoot = "localhost/fcgi/main.fcgi";

//------------------------------------------------------------------------------
// Prints the header.
std::ostream& printHeader (std::ostream& os);

std::ostream& printXMLHeader (std::ostream& os);
std::ostream& printJSONHeader (std::ostream& os);

std::ostream& printCSSLink (std::ostream& os);

std::ostream& renderDebuggingInfo (std::ostream& os, cgicc::Cgicc const& cgi);

char const* renderPhotoURL (unsigned id);

char const* renderPhotoLink (unsigned id);

//==============================================================================
// Inline Method Definitions
//==============================================================================


#endif // BASIC_HTML
