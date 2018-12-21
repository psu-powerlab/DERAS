// Author: Tylor Slay
// Description: This class serves as a base class for DER. It is used as the 
// - digitial twin of the remote device and allows the aggregator to control
// - through alljoyn proxy bus object method calls.

#ifndef DISTRIBUTED_ENERGY_RESOURCE_H_
#define DISTRIBUTED_ENERGY_RESOURCE_H_

#include <map>
#include <string>
#include <alljoyn/ProxyBusObject.h>

class DistributedEnergyResource {
    public:
        // constructor / destructor
        DistributedEnergyResource (
            std::map <std::string, unsigned int> &init,
            ajn::ProxyBusObject &proxy,
            std::string interface
        );
        virtual ~DistributedEnergyResource ();
        void RemoteExportPower (unsigned int power);
        void RemoteImportPower (unsigned int power);
        void Loop (float delta_time);
        void Print ();

    public:
        //  accessors
        // export        
        void SetExportWatts (unsigned int power);
        void SetRatedExportPower (unsigned int watts);
        void SetRatedExportEnergy (unsigned int watt_hours);
        void SetExportPower(float power);
        void SetExportEnergy (float power);
        void SetExportRamp (unsigned int watts_per_second);
        unsigned int GetRatedExportPower ();
        unsigned int GetRatedExportEnergy ();
        unsigned int GetExportPower ();
        unsigned int GetExportEnergy ();
        unsigned int GetExportRamp ();
        // set import methods
        void SetImportWatts (unsigned int power);
        void SetRatedImportPower (unsigned int watts);
        void SetRatedImportEnergy (unsigned int watt_hours);
        void SetImportPower (float power);
        void SetImportEnergy (float power);
        void SetImportRamp (unsigned int watts_per_second);
        unsigned int GetRatedImportPower ();
        unsigned int GetRatedImportEnergy ();
        unsigned int GetImportPower ();
        unsigned int GetImportEnergy ();
        unsigned int GetImportRamp ();
        // set idle methods
        void SetIdleLosses (unsigned int energy_per_hour);
        unsigned int GetIdleLosses ();  
        std::string GetPath ();

    private:
        // class composition
        ajn::ProxyBusObject proxy_;
        
    private:
        // controls
        void ImportPower ();
        void ExportPower ();
        void IdleLoss ();

    private:
        // rated export properties
        unsigned int rated_export_power_;       // (W) to grid
        unsigned int rated_export_energy_;      // (Wh)
        unsigned int export_ramp_;              // (W s^-1)
        // rated import properties
        unsigned int rated_import_power_;       // (W) from grid
        unsigned int rated_import_energy_;      // (Wh)
        unsigned int import_ramp_;              // (W s^-1)
        // rated idle properties
        unsigned int idle_losses_;              // (Wh h^-1)
        // dynamic properties
        float export_power_;
        float export_energy_;
        float import_power_;
        float import_energy_;     
        // control properties
        unsigned int export_watts_;
        unsigned int import_watts_;
        float delta_time_;  // milliseconds
        std::string interface_;
};

#endif // DISTRIBUTED_ENERGY_RESOURCE_H_