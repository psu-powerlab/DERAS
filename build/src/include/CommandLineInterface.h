// Author: Tylor Slay
// Description:
// 		This class is used to control the Aggregator class. It also sets the 
//		main variables (done), and (scheduled) to turn the program off and toggle
// 		the operator class.

#ifndef COMMANDLINEINTERFACE_H_INCLUDED
#define COMMANDLINEINTERFACE_H_INCLUDED

// INCLUDES
#include <string>
#include "Aggregator.h"

class CommandLineInterface {
    public:
        // constructor / destructor
        CommandLineInterface (Aggregator* vpp);
        virtual ~CommandLineInterface ();
        void Help ();
        bool Control (const std::string& input);

    private:
        Aggregator* vpp_ptr_;

};  // end Command Line Interface

#endif // COMMANDLINEINTERFACE_H_INCLUDED