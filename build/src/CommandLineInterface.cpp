// INCLUDES
#include <iostream>
#include <sstream>
#include <vector>
#include "include/CommandLineInterface.h"

bool scheduled;  // this variable is a global from main

// Constructor
// - pass pointer to aggregator object for control
CommandLineInterface::CommandLineInterface (Aggregator* vpp) : vpp_ptr_(vpp) {
}  // end constructor

CommandLineInterface::~CommandLineInterface () {
    // do nothing at this time
}  // end destructor

// Help
// - CLI interface description
void CommandLineInterface::Help () {
    std::cout << "\n\t[Help Menu]\n\n"
        << "> q                 quit\n"
        << "> h                 display help menu\n"
        << "> a                 display all resources\n"
        << "> f                 display filtered resources\n"
        << "> s                 display summary\n"
        << "> t <arg arg...>    set filter targets\n"
        << "> o <y/n>           operator enable/disable\n"
        << "> i <watts>         import power\n"
        << "> e <watts>         export power\n" << std::endl;
} // end Help

// Command Line Interface
// - method to allow user controls during program run-time
bool CommandLineInterface::Control (const std::string& input) {
    // check for program argument
    if (input == "") {
        CommandLineInterface::Help ();
        return false;
    }
    char cmd = input[0];

    // deliminate input string to argument parameters
    std::vector <std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }

    switch (cmd) {
        case 'q': {
           return true;
        }

        case 'a': {
            vpp_ptr_->DisplayAllResources ();
            break;
        }
        case 'f': {
            vpp_ptr_->DisplayTargetResources ();
            break;
        }
        case 's': {
            vpp_ptr_->DisplaySummary ();
            break;
        }
        case 't': {
            try {
                tokens.erase (tokens.begin ());  // remove command
                vpp_ptr_->SetTargets (tokens);
                break;
            } catch (...) {
                std::cout << "[ERROR]: Invalid Argument.\n";
                break;
            }
        }
        case 'o': {
            try {
                if (tokens.at(1) == "n") {
                    scheduled = false;
                } else if (tokens.at(1) == "y") {
                    scheduled = true;
                } else {
                    throw;
                }
                break;
            } catch(...) {
                std::cout << "[ERROR]: Invalid Argument.\n";
                break;
            }
        }
        case 'i': {
            try {
                vpp_ptr_->SetImportWatts (stoul (tokens.at (1)));
                break;
            } catch (...) {
                std::cout << "[ERROR]: Invalid Argument.\n";
                break;
            }
        }
        case 'e': {
            try {
                vpp_ptr_->SetExportWatts (stoul (tokens.at (1)));
                break;
            } catch (...) {
                std::cout << "[ERROR]: Invalid Argument.\n";
                break;
            }
        }
        
         default: {
            CommandLineInterface::Help ();
            break;
        }
    }

    return false;
}  // end Command Line Interface