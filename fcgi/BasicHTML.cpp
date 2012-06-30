//==============================================================================
// BasicHTML.cpp
// Created March 5 2012
//==============================================================================

#include "BasicHTML.h"
#include <unistd.h>
#include "HTTPHTMLHeader.h"
#include "HTMLClasses.h"

using namespace std;
using namespace cgicc;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
ostream& printHeader (std::ostream& os) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   os << HTTPHTMLHeader() << '\n';
   os << HTMLDoctype( HTMLDoctype::eStrict ) << '\n';
   os << html().set( "lang", "en" ) << '\n';
   return os;
}

//------------------------------------------------------------------------------
ostream& printXMLHeader (std::ostream& os) {
   // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
   os << HTTPContentHeader("text/xml");
   // doctype is handled by pugixml
   return os;
}

//------------------------------------------------------------------------------
std::ostream& printCSSLink (std::ostream& os) {
   os << cgicc::link().set("rel", "stylesheet").set("type", "text/css").set("href", "../include/css/data_entry.css") << '\n';
   return os;
}

//------------------------------------------------------------------------------
std::ostream& renderDebuggingInfo (std::ostream& os, Cgicc const& cgi) {
   // Print out debugging info
   os << cgicc::div().set("id", "cgicc_info") << '\n';
   os << h1("Cgicc/FastCGI Debugging Info") << '\n';
   os << p() << '\n';
   os << "PID: "   << getpid() << br() << '\n';
//   os << "Requests Processed: " << count++  << br() << '\n';

   // Print Form Elements
   os  << "Form Elements:" << br() << '\n';
   for (const_form_iterator i = cgi.getElements().begin(); i != cgi.getElements().end(); ++i )
   os << i->getName() << " = " << i->getValue() << br() << '\n';
   os << p() << '\n';
   os << cgicc::div() << '\n';
   return os;
}


//------------------------------------------------------------------------------
char const* renderPhotoURL (unsigned id) {
   std::ostringstream oss;
   oss << "../photos/" << id << ".jpg";
   return oss.str().c_str(); 
}

//------------------------------------------------------------------------------
char const* renderPhotoLink (unsigned id) {
   std::ostringstream oss;
   oss << "<a href=\"" << renderPhotoURL(id) << "\">";
   oss << "Photo " << id;
   oss << "</a>";
   return oss.str().c_str(); 
}
