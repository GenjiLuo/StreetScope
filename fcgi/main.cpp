//==============================================================================
// main.cpp
// Created March 5 2012
//==============================================================================

#include <exception>
#include <iostream>
#include <sstream>

// fcgi Headers
#include <unistd.h>
#include "Cgicc.h"
#include "HTTPHTMLHeader.h"
#include "HTMLClasses.h"
#include "FCgiIO.h"

// Database Headers
#include "Database.h"
#include "DatabaseServer.h"

// WebPage Headers
#include "BasicHTML.h"
//#include "SessionData.h"

#include <fstream>

using namespace std;
using namespace cgicc;

int main (int /*argc*/, const char** /*argv*/, char** /*envp*/) {

   // fcgi initialization
   FCGX_Request request;
   FCGX_Init();
   FCGX_InitRequest(&request, 0, 0);
   
//   unsigned count = 0;
   
   // Database Initialization
   bool failure = false;
   Database database;
   if (database.connect()) {
      database.ensureIndices();
   } else {
      failure = true;
   }
   DatabaseServer dbserver(database);
   
   // Log File Initialization
   // Note: Logging is primarily for debugging purposes - the server is not designed to output any logs under normal operation.
   // For this reason not all DatabaseServer methods take the log as a parameter (these methods haven't ever acted up).
   ofstream log(database.logFile().c_str(), ofstream::out | ofstream::app);


   // main FCGI loop
   while(FCGX_Accept_r(&request) == 0) {
      try {
         FCgiIO IO(request);
         Cgicc CGI(&IO);

         // Find which command is being invoked
         const_form_iterator command = CGI["cmd"];
         if (command == CGI.getElements().end()) {
            // If there's no command, render an eror message.
            printHeader(IO);
            IO << "Error: no command found.\n";
         } else if (strcmp("status", command->getValue().c_str()) == 0) {
            dbserver.status(IO, CGI, failure);
         } else if (strcmp("panorama", command->getValue().c_str()) == 0) {
            dbserver.panorama(IO, CGI, log);
         } else if (strcmp("panorama_near", command->getValue().c_str()) == 0) {
            dbserver.panoramaNear(IO, CGI, log);
         } else if (strcmp("panorama_by_panoid", command->getValue().c_str()) == 0) {
            dbserver.panoramaByPanoid(IO, CGI, log);
         } else if (strcmp("feature", command->getValue().c_str()) == 0) {
            dbserver.feature(IO, CGI);
         } else if (strcmp("features", command->getValue().c_str()) == 0) {
            dbserver.features(IO, CGI);
         } else if (strcmp("tags", command->getValue().c_str()) == 0) {
            dbserver.tags(IO, CGI);
         } else if (strcmp("tags_by_panorama", command->getValue().c_str()) == 0) {
            dbserver.tagsByPanorama(IO, CGI);
         } else if (strcmp("download_pano", command->getValue().c_str()) == 0) {
            dbserver.downloadPanorama(IO, CGI, log);
         } else if (strcmp("insert_tag", command->getValue().c_str()) == 0) {
            dbserver.insertTag(IO, CGI);
         } else if (strcmp("remove_tag", command->getValue().c_str()) == 0) {
            dbserver.removeTag(IO, CGI);
         } else {
            // If we haven't found the command yet, it doesn't exist.
            printHeader(IO);
            IO << "Error: command not recognized.\n";
         }
      }
      catch(const exception& e) {
         // handle error condition
         log << "Error: " << e.what() << endl;
      }
      
      FCGX_Finish_r(&request);
   }

   log.close();
   
   return 0;
}

