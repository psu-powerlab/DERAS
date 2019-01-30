// Author: Tylor Slay
// Description: The operator is used to set the vpp controls based on specified
// - services. Each services as a formated schedule that dictates the required
// - inputs for operation. 

#ifndef OPERATOR_H
#define OPERATOR_H

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
	void SetService (std::string service);

private: 
	// class composition
	Aggregator* vpp_ptr_;

	std::map <std::string, std::string> configs_;
	std::string service_;
	int tou_tier_;

	// each schedule will save the last index value to reduce search time
	unsigned int pjm_a_index_;
	unsigned int pjm_d_index_;
	unsigned int eim_index_;
	unsigned int tou_index_;
	unsigned int pdm_index_;
	unsigned int fer_index_;

	// schedules
	std::vector <RowPJM> schedule_pjm_a_;
	std::vector <RowPJM> schedule_pjm_d_;
	std::vector <RowEIM> schedule_eim_;
	std::vector <RowTOU> schedule_tou_;
	std::vector <RowPDM> schedule_pdm_;
	std::vector <RowFER> schedule_fer_;

	// read service schedules
	void GetPJMRegA ();
	void GetPJMRegD ();
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
        unsigned int utc;
        float normalized_power;

        RowPJM (unsigned int time, float percent_power)
            : utc(time), normalized_power(percent_power) {
        };
    };

    struct RowEIM {
        unsigned int utc;
        float normalized_power;

        RowEIM (unsigned int time, float percent_power)
            : utc(time), normalized_power(percent_power) {
        };
    };

    struct RowTOU {
        unsigned int utc;
        float real_time_price;

        RowTOU (unsigned int time, float real_time)
            : utc(time), real_time_price(real_time) {
        };
    };

    struct RowPDM {
        unsigned int utc;
        int temperature;

        RowPDM (unsigned int time, int Fahrenheit)
            : utc(time), temperature(Fahrenheit) {
        };
    };

    struct RowFER {
        unsigned int utc;
        float frequency;

        RowFER (unsigned int time, float hertz)
            : utc(time), frequency(hertz) {
        };
    };

	enum Months {
		JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC
	};

	enum Tiers {
		OFF_PEAK, MID_PEAK, ON_PEAK
	};
}

#endif  // OPERATOR_H