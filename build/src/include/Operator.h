// Author: Tylor Slay
// Description: The operator is used to set the vpp controls based on specified
// - services. Each services as a formated schedule that dictates the required
// - inputs for operation. 

#ifndef OPERATOR_H_INCLUDED
#define OPERATOR_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include "Aggregator.h"

class Operator {
public:
	Operator (
		std::map <std::string, std::string>& init, Aggregator* vpp_pointer
	);
	virtual ~Operator ();
	void Loop ();
	std::string GetTime (time_t utc);
	void SetService (std::string service);
	void Summary ();

private: 
	// class composition
	Aggregator* vpp_ptr_;

	std::map <std::string, std::string> configs_;
	std::string service_;
	int tou_tier_;
	bool pdm_control_;
	bool fer_control_;

	// each schedule will save the last index value to reduce search time
	unsigned int pjm_a_index_;
	unsigned int pjm_d_index_;
	unsigned int eim_index_;
	unsigned int tou_index_;
	unsigned int pdm_index_;
	unsigned int fer_index_;

	// read service schedules
	void GetPJMA ();
	void GetPJMD ();
	void GetEIM ();
	void GetTOU ();
	void GetPDM ();
	void GetFER ();

	// service methods.
	void ServicePJMA ();
	void ServicePJMD ();
	void ServiceEIM ();
	void ServiceTOU ();
	void ServicePDM ();
	void ServiceFER ();

	// data structures
    struct RowPJM {
        std::string time;
        float normalized_power;

        RowPJM (std::string time, float percent_power)
            : time(time), normalized_power(percent_power) {
        };
    };

    struct RowEIM {
        std::string time;
        float normalized_power;

        RowEIM (std::string time, float percent_power)
            : time(time), normalized_power(percent_power) {
        };
    };

    struct RowTOU {
        std::string time;
        float real_time_price;
        float day_ahead_price;

        RowTOU (std::string time, float real_time, float day_ahead)
            : time(time), real_time_price(real_time), day_ahead_price(day_ahead){
        };
    };

    struct RowPDM {
        std::string time;
        int temperature;

        RowPDM (std::string time, int Fahrenheit)
            : time(time), temperature(Fahrenheit) {
        };
    };

    struct RowFER {
        std::string time;
        float frequency;

        RowFER (std::string time, float hertz)
            : time(time), frequency(hertz) {
        };
    };

	// schedules
	std::vector <RowPJM> schedule_pjm_a_;
	std::vector <RowPJM> schedule_pjm_d_;
	std::vector <RowEIM> schedule_eim_;
	std::vector <RowTOU> schedule_tou_;
	std::vector <RowPDM> schedule_pdm_;
	std::vector <RowFER> schedule_fer_;

	enum Months {
		JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC
	};

	enum Tiers {
		OFF_PEAK, MID_PEAK, ON_PEAK
	};

private:
	float prev_hz_ = 60.0;
	std::vector <float> prev_freqs_;
	bool pos_deviation_ = 0;
	bool neg_deviation_ = 0;
	bool oneshot0_ = 1;
	bool oneshot1_ = 1;
	float event_min_hz_;
	float event_max_hz_;
	float event_delta_hz_ = 0;
	unsigned int event_start_time_;
	float event_start_hz_;
	unsigned int event_duration_sec_;
	bool neg_event_detected_;
	unsigned int neg_response_start_time_;
	unsigned int neg_response_sec_;
	bool neg_response_timer_;
	bool pos_event_detected_;
	unsigned int pos_response_start_time_;
	unsigned int pos_response_sec_;
	bool pos_response_timer_;
	unsigned int import_power_request_;
};

#endif  // OPERATOR_H_INCLUDED
