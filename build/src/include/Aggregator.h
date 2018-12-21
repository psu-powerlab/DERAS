// Author: Tylor Slay
// Description: this class is designed to storing, adding, and removing new
// - DER descovered by AllJoyn. The aggregator is used to send remote control
// - signals to each DER through their respective DCS and emulates the DER
// - response to reduce network traffic. 

#ifndef AGGREGATOR_H_INCLUDED
#define AGGREGATOR_H_INCLUDED

#include <string>
#include <vector>
#include "tsu.h"
#include "DistributedEnergyResource.h"

class Aggregator {
public:
    // constructor / destructor
    Aggregator (tsu::config_map &init);
    virtual ~Aggregator ();
    // accessor / mutators
    void SetTargets (const std::vector <std::string> &targets);
    void SetExportWatts (unsigned int power);
    void SetImportWatts (unsigned int power);
    void SetTime (unsigned int utc);
    void SetPrice (unsigned int price);
    unsigned int GetTotalExportEnergy ();
    unsigned int GetTotalExportPower ();
    unsigned int GetTotalImportEnergy ();
    unsigned int GetTotalImportPower ();
    unsigned int GetTime ();
    unsigned int GetPrice ();
    // aggregator methods
    void AddResource (std::map <std::string, unsigned int>& init,
                      ajn::ProxyBusObject &proxy
    );
    void UpdateResource (std::map <std::string, unsigned int>& init,
                         const std::string& path
    );
    void RemoveResource (const std::string& path);
    void Loop (float delta_time);
    void DisplayAllResources ();
    void DisplayTargetResources ();
    void DisplaySummary ();

private:
    // config map
    tsu::config_map config_;
    // logging
    unsigned int last_log_;
    unsigned int log_inc_;
    std::string log_path_;
    // aggregate
    std::vector <std::shared_ptr <DistributedEnergyResource>> resources_;
    std::vector <std::shared_ptr <DistributedEnergyResource>> sub_resources_;
    // dispatch variables
    // - these variables represent the filtered total resources
    std::vector <std::string> targets_;
    unsigned int total_export_energy_;
    unsigned int total_export_power_;
    unsigned int total_import_energy_;
    unsigned int total_import_power_;
    // control properties
    unsigned int export_watts_;
    unsigned int import_watts_;
    unsigned int price_;
    unsigned int time_;
    // control methods
    void FilterResources ();
    void ExportPower ();
    void ImportPower ();
    void UpdateTotals ();
    void Log ();
};

#endif // AGGREGATOR_H_INCLUDED