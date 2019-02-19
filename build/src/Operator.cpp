#include <iostream>
#include <ctime>
#include "include/Operator.h"
#include "include/tsu.h"

// constructor
Operator::Operator (std::map <std::string, std::string>& init, 
					Aggregator* vpp_pointer) 
					: vpp_ptr_(vpp_pointer),
					  configs_(init),
					  service_(""),
					  tou_tier_(0),
					  pjm_a_index_(0),
					  pjm_d_index_(0),
					  eim_index_(0),
					  tou_index_(0),
					  pdm_index_(0),
					  fer_index_(0) {
	// do nothing
};

// destructor
Operator::~Operator () {
	// do nothing
};

// Loop
// - used to determine which services is active and call appropriate method
void Operator::Loop () {
	if (service_ == "OFF") {
		// do nothing
	} else if (service_ == "PJMA") {
		Operator::ServicePJMA ();
	} else if (service_ == "PJMD") {
		Operator::ServicePJMD ();
	} else if (service_ == "EIM") {
		Operator::ServiceEIM ();
	} else if (service_ == "TOU") {
		Operator::ServiceTOU ();
	} else if (service_ == "PDM") {
		Operator::ServicePDM ();
	} else if (service_ == "FER") {
		Operator::ServiceFER ();
	}
};  // end Loop

// Set Service
// - mutator for the service variable
void Operator::SetService (std::string service) {
	if (service == "PJMA"
		|| service == "PJMD"
		|| service == "EIM" 
		|| service == "TOU" 
		|| service == "PDM"
		|| service == "FER"
		|| service == "OFF") {
		service_ = service;		
	} else {
		std::cout << "Set Service Error: Invalid service type." << std::endl;
	}
};  // end Set Service

// Get PJM A
// - read the PJM schedule and format for use
void Operator::GetPJMA () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["pjma_filepath"], ',', 2);
	struct tm tm;
	unsigned int time;
	float percent_power;
	schedule_pjm_a_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    // http://man7.org/linux/man-pages/man3/strptime.3.html
	    strptime(row.at (0).c_str(), configs_["pjma_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		percent_power = stof (row.at (1));
		schedule_pjm_a_.emplace_back(time, percent_power);
	}
};  // end Get PJM A

// Get PJM D
// - read the PJM schedule and format for use
void Operator::GetPJMD () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["pjmd_filepath"], ',', 2);
	struct tm tm;
	unsigned int time;
	float percent_power;
	schedule_pjm_d_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    strptime(row.at (0).c_str(), configs_["pjmd_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		percent_power = stof (row.at (1));
		schedule_pjm_d_.emplace_back(time, percent_power);
	}
};  // end Get PJM D

// Get EIM
// - read the EIM schedule and format for use
void Operator::GetEIM () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["eim_filepath"], ',', 2);
	struct tm tm;
	unsigned int time;
	float percent_power;
	schedule_eim_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    strptime(row.at (0).c_str(), configs_["eim_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		percent_power = stof (row.at (1));
		schedule_eim_.emplace_back(time, percent_power);
	}
};  // end Get EIM

// Get TOU
// - read the TOU schedule and format for use
void Operator::GetTOU () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["tou_filepath"], ',', 3);
	struct tm tm;
	unsigned int time;
	float day_ahead;
	float real_time;
	schedule_tou_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    strptime(row.at (0).c_str(), configs_["tou_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		day_ahead = stof (row.at (1));
		real_time = stof (row.at (2));
		schedule_tou_.emplace_back(time, real_time, day_ahead);
	}
};  // end Get TOU

// Get PDM
// - read the PDM schedule and format for use
void Operator::GetPDM () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["pdm_filepath"], ',', 2);
	struct tm tm;
	unsigned int time;
	int fahrenheit;
	schedule_pdm_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    strptime(row.at (0).c_str(), configs_["pdm_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		fahrenheit = stoi (row.at (1));
		schedule_pdm_.emplace_back(time, fahrenheit);
	}
};  // end Get PDM

// Get FER
// - read the FER schedule and format for use
void Operator::GetFER () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["fer_filepath"], ',', 2);
	struct tm tm;
	unsigned int time;
	float hertz;
	schedule_fer_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    strptime(row.at (0).c_str(), configs_["fer_time_format"].c_str(), &tm);
	    time = std::mktime (&tm);
		hertz = stof (row.at (1));
		schedule_fer_.emplace_back(time, hertz);
	}
};  // end Get FER

// Service PJM Reg A
// - Reg A is a normalized power control signal that is meant for traditional
// - regulating resources. Please reference PJM's Manual 12 for more information
// - The services will used the vpp resource info to determine dispatch and call
// - the appropriate control method.
void Operator::ServicePJMA () {
	// if the schedule has not been read, then load it into memory
	if (schedule_pjm_a_.empty()) {
		Operator::GetPJMA ();
	}

	// get current utc and modulo the date info out since it isn't required for
	// our tests.
	unsigned int seconds_per_day = 60*60*24;
	unsigned int utc = std::time(nullptr) % seconds_per_day;

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pjm_a_index_; i < schedule_pjm_a_.size(); i++) {
        RowPJM& row = schedule_pjm_a_.at (i);


        if ((row.utc % seconds_per_day == utc) && pjm_a_index_ != i) {
            // if the time is found then determine dispatch
            if (row.normalized_power > 0) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	float dispatch_watts = available_watts * row.normalized_power;
                vpp_ptr_->SetImportWatts (dispatch_watts);
            } else if (row.normalized_power < 0) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalExportPower ();
            	float dispatch_watts = available_watts*(-row.normalized_power);
                vpp_ptr_->SetExportWatts (dispatch_watts);
            } else {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
            }

            // log index so multiple controls are not sent and to reduce
            // search time.
            pjm_a_index_ = i;
        }
    }
};  // end Service PJM Reg A

// Service PJM Reg D
// - Reg D is a normalized power control signal that is meant for dynamic
// - regulating resources. Please reference PJM's Manual 12 for more information
// - The services will used the vpp resource info to determine dispatch and call
// - the appropriate control method.
void Operator::ServicePJMD () {
	// if the schedule has not been read, then load it into memory
	if (schedule_pjm_d_.empty()) {
		Operator::GetPJMD ();
	}

	// get current utc and modulo the date info out since it isn't required for
	// our tests.
	unsigned int seconds_per_day = 60*60*24;
	unsigned int utc = std::time(nullptr) % seconds_per_day;

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pjm_d_index_; i < schedule_pjm_d_.size(); i++) {
        RowPJM& row = schedule_pjm_d_.at (i);

        if ((row.utc % seconds_per_day == utc) && pjm_d_index_ != i) {

            // if the time is found then determine dispatch
            if (row.normalized_power > 0) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	float dispatch_watts = available_watts * row.normalized_power;
                vpp_ptr_->SetImportWatts (dispatch_watts);
            } else if (row.normalized_power < 0) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalExportPower ();
            	float dispatch_watts = available_watts*(-row.normalized_power);
                vpp_ptr_->SetExportWatts (dispatch_watts);
            } else {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
            }

            // store index so multiple control signals are not sent
            pjm_d_index_ = i;
        }
    }
};  // end Service PJM Reg D

// Service EIM
// - EIM is the energy imbalance market and is designed for Balancing Area
// - authorities. Please reference https://www.westerneim.com for more.
// - The services will used the vpp resource info to determine dispatch and call
// - the appropriate control method.
void Operator::ServiceEIM () {
	// if the schedule has not been read, then load it into memory
	if (schedule_eim_.empty()) {
		Operator::GetEIM ();
	}

	// get current utc and modulo the date info out since it isn't required for
	// our tests.
	unsigned int seconds_per_day = 60*60*24;
	unsigned int utc = std::time(nullptr) % seconds_per_day;

    // loop through each row of schedule looking for current utc
    for (unsigned int i = eim_index_; i < schedule_eim_.size(); i++) {
        RowEIM& row = schedule_eim_.at (i);

        if ((row.utc % seconds_per_day == utc) && eim_index_ != i) {

            // if the time is found then determine dispatch
            if (row.normalized_power > 0) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	float dispatch_watts = available_watts * row.normalized_power;
            	std::cout << "EIM: import = " << dispatch_watts << std::endl;
                vpp_ptr_->SetImportWatts (dispatch_watts);
            } else if (row.normalized_power < 0) {
	        	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
            	float available_watts = vpp_ptr_->GetTotalExportPower ();
            	float dispatch_watts = available_watts*(-row.normalized_power);
                vpp_ptr_->SetExportWatts (dispatch_watts);
            } else {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
            }

            // store index so multiple control signals are not sent
            eim_index_ = i;
        }
    }
};  // end Service EIM

// Service TOU
// - TOU is the Time of Use and will try to reduce the cost of importing 
// - while maximizing the return for exporting to the grid.
// - Reference https://www.portlandgeneral.com for TOU info
void Operator::ServiceTOU () {
	// get current utc and modulo the date info out since it isn't required for
	// our tests.
	unsigned int seconds_per_day = 60*60*24;
	time_t time = std::time(nullptr);
	unsigned int utc = time % seconds_per_day;

	// Every minute determine TOU tier and set import/export accordingly
    if (utc % 60 == 0 && tou_index_ != utc) {
        // check season for tou tiers
        struct tm time_info = *std::localtime (&time);
        int month = time_info.tm_mon + 1;
        int hour = time_info.tm_hour;
        if (month >= MAY && month <= OCT) {
        	// SUMMER TOU
        	if (hour >= 22 && hour < 6) {
        		tou_tier_ = OFF_PEAK;
        	} else if (hour >= 6 && hour < 15) {
        		tou_tier_ = MID_PEAK;
        	} else if (hour >= 15 && hour < 20) {
        		tou_tier_ = ON_PEAK;
        	} else {
        		tou_tier_ = MID_PEAK;
        	}
        } else {
        	// WINTER TOU
        	if (hour >= 22 && hour < 6) {
        		tou_tier_ = OFF_PEAK;
        	} else if (hour >= 6 && hour < 10) {
        		tou_tier_ = ON_PEAK;
        	} else if (hour >= 10 && hour < 17) {
        		tou_tier_ = MID_PEAK;
        	} else if (hour >= 17 && hour < 20) {
        		tou_tier_ = ON_PEAK;
        	} else {
        		tou_tier_ = MID_PEAK;
        	}
        }

        if (tou_tier_ == ON_PEAK) {
        	// sell as much as possible
        	std::vector <std::string> targets = {""};
        	vpp_ptr_->SetTargets(targets);
	    	float available_watts = vpp_ptr_->GetTotalExportPower ();
	        vpp_ptr_->SetExportWatts (available_watts);
        } else if (tou_tier_ == MID_PEAK) {
        	// only import on priority loads that cannot be time shifted
        	std::vector <std::string> targets = {"buffer"};
        	vpp_ptr_->SetTargets(targets);
        	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	vpp_ptr_->SetImportWatts (available_watts);
        } else {
        	// import as much as possible
        	std::vector <std::string> targets = {""};
        	vpp_ptr_->SetTargets(targets);
        	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	vpp_ptr_->SetImportWatts (available_watts);
        }
        tou_index_ = utc;
    }
};  // end Service TOU

// Service PDM
// - PDM is the Peak Demand Mitigation and will try to reduce peak power under
// - specified conditions
void Operator::ServicePDM () {
	// if the schedule has not been read, then load it into memory
	if (schedule_pdm_.empty()) {
		Operator::GetPDM ();
	}

	// get current utc and modulo the date info out since it isn't required for
	// our tests.
	unsigned int seconds_per_day = 60*60*24;
	time_t time = std::time(nullptr);
	unsigned int utc = time % seconds_per_day;

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pdm_index_; i < schedule_pdm_.size(); i++) {
        RowPDM& row = schedule_pdm_.at (i);

        if ((row.utc % seconds_per_day == utc) && pdm_index_ != i) {
	        // get hour info
	        struct tm time_info = *std::localtime (&time);
	        int hour = time_info.tm_hour;
            // if the time is found then determine dispatch
            if (row.temperature > 85 && hour >= 18 && hour <= 21) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
		    	float available_watts = vpp_ptr_->GetTotalExportPower ();
		        vpp_ptr_->SetExportWatts (available_watts);
            } else if (row.temperature < 39 && hour >= 17 && hour <= 20) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
		    	float available_watts = vpp_ptr_->GetTotalExportPower ();
		        vpp_ptr_->SetExportWatts (available_watts);
            } 

            // store index so multiple control signals are not sent
            pdm_index_ = i;
        }
    }
};  // end Service PDM

// Service FER
void Operator::ServiceFER () {

};  // end Service FER
