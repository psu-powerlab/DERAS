#include <iostream>
#include <ctime>
#include <numeric>
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

// Get Time
// - return HH:MM:SS formatted time
std::string Operator::GetTime (time_t utc) {
	    struct tm ts = *localtime(&utc);
	    char buf[100];
	    strftime(buf, sizeof(buf), "%T", &ts);
	    return std::string(buf);
}

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

// Summary
// - display the current service and the last control sent
void Operator::Summary () {
	std::string last_time;
	float last_control;
	if (service_ == "OFF") {
		last_time = 1;
		last_control = 0;
	} else if (service_ == "PJMA") {
		RowPJM& row = schedule_pjm_a_.at(pjm_a_index_);
		last_time = row.time;
		last_control = row.normalized_power;
	} else if (service_ == "PJMD") {
		RowPJM& row = schedule_pjm_d_.at(pjm_d_index_);
		last_time = row.time;
		last_control = row.normalized_power;
	} else if (service_ == "EIM") {
		RowEIM& row = schedule_eim_.at(eim_index_);
		last_time = row.time;
		last_control = row.normalized_power;
	} else if (service_ == "TOU") {
	    time_t now = time(nullptr);
		last_time = Operator::GetTime (now);
		last_control = tou_tier_;
	} else if (service_ == "PDM") {
		RowPDM& row = schedule_pdm_.at(pdm_index_);
		last_time = row.time;
		last_control = pdm_control_;
	} else if (service_ == "FER") {
		RowFER& row = schedule_fer_.at(fer_index_);
		last_time = row.time;
		last_control = fer_control_;
	}
	std::cout << "\nOperator:"
		<< "\n\t service:\t" << service_
		<< "\n\t last time:\t" << last_time
		<< "\n\t last control:\t" << last_control << std::endl;

};  // end Summary

// Get PJM A
// - read the PJM schedule and format for use
void Operator::GetPJMA () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["pjma_filepath"], ',', 2);
	std::string time;
	float percent_power;
	schedule_pjm_a_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    // http://man7.org/linux/man-pages/man3/strptime.3.html
	    time = row.at(0);
		percent_power = stof (row.at (1));
		schedule_pjm_a_.emplace_back(time, percent_power);
	}
};  // end Get PJM A

// Get PJM D
// - read the PJM schedule and format for use
void Operator::GetPJMD () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["pjmd_filepath"], ',', 2);
	std::string time;
	float percent_power;
	schedule_pjm_d_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    time = row.at(0);
		percent_power = stof (row.at (1));
		schedule_pjm_d_.emplace_back(time, percent_power);
	}
};  // end Get PJM D

// Get EIM
// - read the EIM schedule and format for use
void Operator::GetEIM () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["eim_filepath"], ',', 2);
	std::string time;
	float percent_power;
	schedule_eim_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    time = row.at(0);
		percent_power = stof (row.at (1));
		std::cout << time << "\t" << percent_power << '\n';
		schedule_eim_.emplace_back(time, percent_power);
	}
};  // end Get EIM

// Get TOU
// - read the TOU schedule and format for use
void Operator::GetTOU () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["tou_filepath"], ',', 3);
	std::string time;
	float day_ahead;
	float real_time;
	schedule_tou_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    time = row.at(0);
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
	std::string time;
	int fahrenheit;
	schedule_pdm_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    time = row.at(0);
		fahrenheit = stoi (row.at (1));
		schedule_pdm_.emplace_back(time, fahrenheit);
	}
};  // end Get PDM

// Get FER
// - read the FER schedule and format for use
void Operator::GetFER () {
	tsu::string_matrix schedule 
		= tsu::FileToMatrix (configs_["fer_filepath"], ',', 2);
	std::string time;
	float hertz;
	schedule_fer_.reserve (schedule.size ());
	schedule.erase (schedule.begin ());  // remove header
	for (const auto& row : schedule) {
	    time = row.at(0);
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

    time_t now = time(nullptr);
	std::string time = Operator::GetTime (now);

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pjm_a_index_; i < schedule_pjm_a_.size(); i++) {
        RowPJM& row = schedule_pjm_a_.at (i);


        if ((row.time == time) && pjm_a_index_ != i) {
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

    time_t now = time(nullptr);
	std::string time = Operator::GetTime (now);

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pjm_d_index_; i < schedule_pjm_d_.size(); i++) {
        RowPJM& row = schedule_pjm_d_.at (i);

        if ((row.time == time) && pjm_d_index_ != i) {

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

    time_t now = time(nullptr);
	std::string time = Operator::GetTime (now);

    // loop through each row of schedule looking for current utc
    for (unsigned int i = eim_index_; i < schedule_eim_.size(); i++) {
        RowEIM& row = schedule_eim_.at (i);

        if ((row.time == time) && eim_index_ != i) {

            // if the time is found then determine dispatch
            if (row.normalized_power > 0) {
            	float available_watts = vpp_ptr_->GetTotalImportPower ();
            	float dispatch_watts = available_watts * row.normalized_power;
            	std::cout << "EIM: import = " << dispatch_watts << std::endl;
                vpp_ptr_->SetImportWatts (dispatch_watts);
            } else if (row.normalized_power < 0) {
            	float available_watts = vpp_ptr_->GetTotalExportPower ();
            	float dispatch_watts = available_watts*(-row.normalized_power);
                vpp_ptr_->SetExportWatts (dispatch_watts);
            } else {
                vpp_ptr_->SetImportWatts (0);
            }
			// store index so multiple control signals are not sent
			eim_index_ = i;
        }
    }
	if (eim_index_ == (schedule_eim_.size () - 1)) {
		eim_index_ = 0;
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
	time_t time = std::time(nullptr);
	std::string f_time = Operator::GetTime (time);

    // loop through each row of schedule looking for current utc
    for (unsigned int i = pdm_index_; i < schedule_pdm_.size(); i++) {
        RowPDM& row = schedule_pdm_.at (i);

        if ((row.time == f_time) && pdm_index_ != i) {
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
		        pdm_control_ = true;
            } else if (row.temperature < 39 && hour >= 17 && hour <= 20) {
            	std::vector <std::string> targets = {""};
	        	vpp_ptr_->SetTargets(targets);
                vpp_ptr_->SetImportWatts (0);
		    	float available_watts = vpp_ptr_->GetTotalExportPower ();
		        vpp_ptr_->SetExportWatts (available_watts);
		        pdm_control_ = true;
            } else {
            	pdm_control_ = false;
            }

            // store index so multiple control signals are not sent
            pdm_index_ = i;
        }
    }
};  // end Service PDM

// Service FER
void Operator::ServiceFER () {
	unsigned int actual_time;
	float actual_hz;
	float delta_hz;
	float moving_avg;
	float floor_freq = 59.975;
	float min_slew_rate = 0.0031;
	float ceiling_freq = 120 - floor_freq;

	if (schedule_fer_.empty()) {
		Operator::GetFER ();
	}

	time_t time = std::time(nullptr);
	std::string f_time = Operator::GetTime (time);

        // loop through each row of schedule looking for current utc
        for (unsigned int i = fer_index_; i < schedule_fer_.size(); i++) {
        	RowFER& row = schedule_fer_.at (i);

		if (row.time == f_time && fer_index_ != i) {
			actual_time = time;
			actual_hz = row.frequency;
		}
	}
	delta_hz = actual_hz - prev_hz_;
	prev_hz_ = actual_hz;
	prev_freqs_.erase(prev_freqs_.begin());
	prev_freqs_.push_back(actual_hz);
	moving_avg = accumulate(prev_freqs_.begin(),prev_freqs_.end(),0)
		/ prev_freqs_.size();
	
	if (actual_hz < floor_freq && actual_hz < moving_avg && delta_hz < 0) {
		neg_deviation_ = 1;
	}
	
	if (actual_hz > ceiling_freq && actual_hz > moving_avg && delta_hz > 0) {
		pos_deviation_ = 1;
	}

	if (neg_deviation_ == 1) {
		if (oneshot0_ == 1) {
			oneshot0_ = 0;
			event_start_hz_ = abs(delta_hz) + actual_hz;
			event_min_hz_ = 99;
			event_start_time_ = actual_time;
		}

		if (actual_hz < event_min_hz_) {
			event_min_hz_ = actual_hz;
		} else if ((actual_hz - event_min_hz_) > 0.003) {
			neg_deviation_ = 0;
			event_delta_hz_ = 0;
			event_duration_sec_ = 0;
		}
	} else {
		oneshot0_ = 1;
	}

	if (pos_deviation_ == 1) {
		if (oneshot1_ == 1) {
			oneshot1_ == 0;
			event_start_hz_ = actual_hz - abs(delta_hz);
			event_max_hz_ = 0;
			event_start_time_ = actual_time;
		}

		if (actual_hz > event_max_hz_) {
			event_max_hz_ = actual_hz;
		} else if ((event_max_hz_ - actual_hz) > 0.003) {
			pos_deviation_ = 0;
			event_delta_hz_ = 0;
			event_duration_sec_ = 0;
		}
	} else {
		oneshot1_ = 1;
	}

	if (nev_deviation_ == 1 && actual_hz < moving_avg) {
		event_delta_hz_ = event_start_hz_ - actual_hz;
		event_duration_sec_ = event_start_time_ - actual_time;
	}

	if (pos_deviation_ == 1 && actual_hz > moving_avg) {
		event_delta_hz = actual_hz - event_start_hz_;
		event_duration_sec_ = event_start_time_ actual_time;
	}

	if (neg_response_timer_ == 1 && neg_response_sec_ < 180) {
		neg_event_detected_ = 1;
		neg_response_sec_ = actual_time - neg_response_start_time_;
	} else if (neg_deviation_ == 1 && event_duration_sec_ >= 1
		&& (event_delta_hz_/event_duration_sec_) >= min_slew_rate
		&& event_delta_hz_ >= (min_slew_rate * 10)) {
		neg_event_detected_ = 1;
		neg_response_timer_ = 1;
		neg_response_start_time_ = actual_time;
	} else {
		neg_reponse_timer = 0;
		neg_response_sec_ = 0;
		neg_event_detected_ = 0;
	}

	if (pos_response_timer_ == 1 && pos_response_sec_ < 180) {
		pos_event_detected_ = 1;
		pos_response_sec_ = actual_time - pos_response_start_time_;
	} else if (pos_deviation_ == 1 && event_duration_sec_ >= 1
		&& (event_delta_hz_/event_duration_sec_) >= min_slew_rate
		&& event_delta_hz_ >= (min_slew_rate * 10)) {
		pos_event_detected_ = 1;
		pos_response_timer_ = 1;
		pos_response_start_time_ = actual_time;
	} else {
		pos_reponse_timer = 0;
		pos_response_sec_ = 0;
		pos_event_detected_ = 0;
	}	

	if (neg_event_detected_ == 1) {
		pos_response_timer_ = 0;
		pos_response_sec_ = 0;
		pos_event_detected_ = 0;
	}

	//Log positive and negative event detection here

	if (neg_event_detected_ == 1) {
		//negative event response algorithm here
	} else if (pos_event_detected == 1) {
		//positive event response algorithm here
	}
	
};  // end Service FER
