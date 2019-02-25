// INCLUDES
#include <iostream>
#include <sstream>
#include <vector>
#include "include/CommandLineInterface.h"

// Constructor
// - pass pointer to aggregator object for control
CommandLineInterface::CommandLineInterface (Aggregator* vpp, Operator* opr) 
    : vpp_ptr_(vpp), oper_ptr_(opr) {
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
        << "> t <arg arg...>    targets filter\n"
        << "> i <watts>         import power\n"
        << "> e <watts>         export power\n"
        << "> p <1/10 of cent>  power price\n"
        << "> w <F>             weather (deg. F)\n"
        << "> o <service>       set operator service\n"
        << "                    OFF: disable operator\n"
        << "                    PJMA: Regulation A\n"
        << "                    PJMD: Regulation D\n"
        << "                    EIM: Energy Imbalance Market\n"
        << "                    TOU: Time of Use\n"
        << "                    PDM: Peak Demand Mitigation\n"
        << "                    FER: Frequency Event Response\n" << std::endl;
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
            oper_ptr_->Summary ();
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
                oper_ptr_->SetService(tokens.at (1));
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
        case 'p': {
            try {
                vpp_ptr_->SetPrice (stoul (tokens.at (1)));
                break;
            } catch (...) {
                std::cout << "[ERROR]: Invalid Argument.\n";
                break;
            }
        }
        case 'w': {
            try {
                vpp_ptr_->SetTemperature (stoi (tokens.at (1)));
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