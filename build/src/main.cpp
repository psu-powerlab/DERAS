/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF), AllJoyn Open Source
 *    Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Copyright (c) V2 Systems, LLC.  All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
******************************************************************************/

// INCLUDE
#include <iostream>     // cout, cin
#include <thread>       // thread, join
#include <chrono>       // now, duration
#include <map>
#include <string>
#include <vector>

#include "include/SetPoint.h"
#include "include/xml2schedule.h"
//#include "include/schedulizer.h"
#include "include/tsu.h"
#include "include/aj_utility.h"
#include "include/Aggregator.h"
#include "include/DistributedEnergyResource.h"
#include "include/CommandLineInterface.h"
#include "include/ClientListener.h"
#include "include/SmartGridDevice.h"
#include "include/ScheduleOperator.h"

// NAMESPACES
using namespace std;
using namespace ajn;

// GLOBALS
bool done = false;      // signal program to stop
extern bool scheduled;  // toggle operator using program args/CLI

// Program Help
// - command line interface arguments during run, [] items have default values
static void ProgramHelp (const string& name) {
    cout << "\n[Usage] > " << name << " -c <file path> [-o <y/n>] -h\n"
        "\t[] means it has a default value\n"
        "\t -h \t help\n"
        "\t -c \t configuration filename" 
        "\t -o \t enable operator"  << endl;
}  // end Program Help

// Argument Parser
// - method to parse program initial parameters
static std::map <std::string, std::string> ArgumentParser (int argc, 
                                                           char** argv) {
    string name = argv[0];
    scheduled = false;

    // parse tokens
    map <string, string> parameters;
    string token, argument;

    for (int i = 1; i < argc; i = i+2){
        token = argv[i];

        // check to see if the is an argument for the program control token
        if (argc <= i+1) {
            cout << "[ERROR] : Invalid program argument: " << token << endl;
            ProgramHelp(name);
            exit(EXIT_FAILURE); 
        } else {
            argument = argv[i+1];
        }
        

        if ((token == "-h")) {
            ProgramHelp(name);
            exit(EXIT_FAILURE);
        } else if ((token == "-c")) {
            parameters["config"] = argument;
        } else if ((token == "-o")) {
            if ((argument == "y")) {
                scheduled = true;
            } else if ((argument == "n")) {
                scheduled = false;
            } else {
                cout << "[ERROR] : Invalid program argument: " << token << endl;
                ProgramHelp(name);
                exit(EXIT_FAILURE); 
            }
        } else {
            cout << "[ERROR] : Invalid parameter: " << token << endl;
            ProgramHelp(name);
            exit(EXIT_FAILURE);
        }
    }
    return parameters;
}  // end Argument Parser

// THREADS
// -------

// Resource Loop
// - this loop runs the aggregator control loop at the desired frequency
// - it subtracks processing time of the Loop () function to make the frequency
// - more consistant
void AggregatorLoop (unsigned int sleep, Aggregator* vpp_ptr) {
    unsigned int time_remaining, time_past;
    unsigned int time_wait = sleep;
    auto time_start = chrono::high_resolution_clock::now();
    auto time_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time_elapsed;

    while (!done) {
        time_start = chrono::high_resolution_clock::now();
            // time since last control call;
            time_elapsed = time_start - time_end;
            time_past = time_elapsed.count();
            vpp_ptr->Loop(time_past);
        time_end = chrono::high_resolution_clock::now();
        time_elapsed = time_end - time_start;

        // determine sleep duration after deducting process time
        if (time_wait - time_elapsed.count() > 0) {
            time_remaining = time_wait - time_elapsed.count();
            this_thread::sleep_for (chrono::milliseconds (time_remaining));
        } 
    }
}  // end Aggregator Loop

// Operator Loop
// - this loop runs the resource control loop at the desired frequency
// - it subtracks processing time of the Loop () function to make the frequency
// - more consistant
void OperatorLoop (unsigned int sleep, ScheduleOperator* oper_ptr) {
    unsigned int time_remaining;
    unsigned int time_wait = sleep;
    auto time_start = chrono::high_resolution_clock::now();
    auto time_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time_elapsed;

    while (!done && scheduled) {
        time_start = chrono::high_resolution_clock::now();
        oper_ptr->Loop();
        time_end = chrono::high_resolution_clock::now();
        time_elapsed = time_end - time_start;

        // determine sleep duration after deducting process time
        if (time_wait - time_elapsed.count() > 0) {
            time_remaining = time_wait - time_elapsed.count();
            this_thread::sleep_for (chrono::milliseconds (time_remaining));
        } 
    }
}  // end Resource Loop

// Smart Grid Device Loop
// - this loop runs the resource control loop at the desired frequency
// - it subtracks processing time of the Loop () function to make the frequency
// - more consistant
void SmartGridDeviceLoop (unsigned int sleep, SmartGridDevice* sgd_ptr) {
    unsigned int time_remaining;
    unsigned int time_wait = sleep;
    auto time_start = chrono::high_resolution_clock::now();
    auto time_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time_elapsed;

    while (!done) {
        time_start = chrono::high_resolution_clock::now();
        sgd_ptr->Loop();
        time_end = chrono::high_resolution_clock::now();
        time_elapsed = time_end - time_start;

        // determine sleep duration after deducting process time
        if (time_wait - time_elapsed.count() > 0) {
            time_remaining = time_wait - time_elapsed.count();
            this_thread::sleep_for (chrono::milliseconds (time_remaining));
        } 
    }
}  // end Resource Loop

// Main
// ----
int main (int argc, char** argv) {
cout 
        << "\n******************************************************"
        << "\n*** Distributed Energy Resource Aggregation System ***"
        << "\n******************************************************\n";

    cout << "Initialization...\n";
    // if the config file is not passed to the program then exit
   if (strcmp(argv[1], "-c") != 0) {
        string name = argv[0];
        ProgramHelp(name);
        return EXIT_FAILURE;
    }
    map <string, string> arguments = ArgumentParser(argc, argv);

    // read config file for program configurations and object attributes
    tsu::config_map configs = tsu::MapConfigFile (arguments["config"]);

    cout << "\tCreating virtual power plant\n";
    // ~ reference Aggregator
    Aggregator* vpp_ptr = new Aggregator (configs);

    // TODO (TS): move the xml2schedule function into the operator class
    cout << "\tCreating Operator\n";
    std::vector<SetPoint> schedule = xml2schedule(configs["Operator"]["path"]);
    //ScheduleOperator *opr_ptr = new ScheduleOperator("../DERAS/data/timeActExt.csv", vpp_ptr);
    ScheduleOperator *oper_ptr = new ScheduleOperator(schedule, vpp_ptr);

    cout << "\tCreating Command Line Interface\n";
    // ~ reference CommandLineInterface.h
    CommandLineInterface CLI(vpp_ptr);

    cout << "\tCreating AllJoyn Message Bus\n";
    try {
        cout << "\t\tInitializing AllJoyn...\n";
        // Must be called before any AllJoyn functionality
        AllJoynInit();
    } catch (exception &e) {
        cout << "[ERROR]: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    #ifdef ROUTER
        try {
            cout << "\t\tInitializing AllJoyn Router...\n";
            // Must be called before any AllJoyn routing functionality
            AllJoynRouterInit();
        } catch (exception &e) {
            cout << "[ERROR]: " << e.what() << endl;
            return EXIT_FAILURE;
        }
    #endif // ROUTER

    cout << "\tCreating AllJoyn Bus Attachment\n";
    // BusAttachment is the top-level object responsible for connecting to and
    // optionally managing a message bus.
    // ~ AllJoyn Docs
    string app = configs["AllJoyn"]["app"];
    bool allow_remote = true;
    BusAttachment* bus_ptr = new BusAttachment(app.c_str(), allow_remote);

    cout << "\tCreating AllJoyn About Data\n";
    // The AboutObj class is responsible for transmitting information about the
    // interfaces that are available for other applications to use.
    // ~ AllJoyn Docs
    AboutData about_data("en");
    AboutObj* about_ptr = new AboutObj(*bus_ptr);

    cout << "\tCreating AllJoyn Session Port\n";
    // inform users of session related events
    // ~ AllJoyn Docs
    aj_utility::SessionPortListener SPL;
    SessionPort port = stoul(configs["AllJoyn"]["port"]);

    cout << "\tSetting up AllJoyn Bus Attachment...\n";
    // ~ reference aj_utility.cpp
    QStatus status = aj_utility::SetupBusAttachment (configs,
                                                     port,
                                                     SPL,
                                                     bus_ptr,
                                                     &about_data);

    cout << "\tCreating AllJoyn Observer\n";
    // takes care of discovery, session management and ProxyBusObject creation
    // for bus objects.
    // ~ AllJoyn Docs
    const char* client_name = configs["AllJoyn"]["client_interface"].c_str();
    Observer *obs_ptr = new Observer(*bus_ptr, &client_name, 1);

    cout << "\tCreating AllJoyn Server Listener\n";
    // ~ reference ClientListener.cpp
    ClientListener *listner_ptr = new ClientListener(bus_ptr,
                                                     obs_ptr,
                                                     vpp_ptr,
                                                     client_name);
    obs_ptr->RegisterListener(*listner_ptr);

    cout << "\tCreating AllJoyn Smart Grid Device\n";
    // ~ reference SmartGridDevice.cpp
    const char* device_name = configs["AllJoyn"]["server_interface"].c_str();
    const char* path = configs["AllJoyn"]["path"].c_str();
    SmartGridDevice *sgd_ptr = new SmartGridDevice(bus_ptr, 
                                                   vpp_ptr, 
                                                   device_name, 
                                                   path);

    cout << "\t\tRegistering AllJoyn Smart Grid Device\n";
    if (ER_OK != bus_ptr->RegisterBusObject(*sgd_ptr)){
        cout << "\t\t[ERROR]: Failed Registration!\n";
        return EXIT_FAILURE;
    }
    about_ptr->Announce(port, about_data);

    // most objects will have a dedicated thread, but not all
    cout << "\tSpawning threads...\n";
    thread DER (AggregatorLoop, stoul(configs["Threads"]["sleep"]), vpp_ptr);
    thread OPER (
        OperatorLoop, stoul(configs["Threads"]["sleep"]), oper_ptr
    );
    thread SGD (
        SmartGridDeviceLoop, stoul(configs["Threads"]["sleep"]), sgd_ptr
    );

    // the CLI will control the program and can signal the program to stop
    cout << "Initialization complete...\n";
    CLI.Help ();
    string input;
    while (!done) {
        getline(cin, input);
        done = CLI.Control (input);
    }

    // when done = true, the program begins the shutdown process
    // TODO (TS): AllJoyn still leaves alot of errors when closing. The docs
    // - dont really explain the shutdown procedure for lots of alljoyn objects
    cout << "Closing program...\n";

    // First join all active threads to main thread
    cout << "\tJoining threads\n";
    DER.join ();
    OPER.join ();
    SGD.join ();

    cout << "\tUnregistering AllJoyn objects\n";
    //obs_ptr->UnregisterListener (*listner_ptr);
    //bus_ptr->UnregisterBusObject(*sgd_ptr);
    //status = bus_ptr->Stop ();
    //status = bus_ptr->Join ();

    // Then delete all pointers that were created using "new" since they do not
    // automaticall deconstruct at the end of the program.
    cout << "\nDeleting pointers...\n";
    delete sgd_ptr;
    delete listner_ptr;
    delete obs_ptr;
    delete about_ptr;
    delete bus_ptr;
    delete oper_ptr;
    delete vpp_ptr;

    #ifdef ROUTER
        cout << "\tShutting down AllJoyn Router\n";
        status = AllJoynRouterShutdown ();
    #endif // ROUTER

    cout << "\tShutting down AllJoyn\n";
    status = AllJoynShutdown ();

    // return exit status
    return EXIT_SUCCESS;
} // end main
