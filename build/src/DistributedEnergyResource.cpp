#include <iostream>
#include <string>
#include <map>

#include "include/DistributedEnergyResource.h"

#define DEBUG(x) std::cout << x << std::endl

// Constructor
// - this will be used to populate all the DER properties when alljoyn finds
// - a new device advertised
DistributedEnergyResource::DistributedEnergyResource (
    std::map <std::string, unsigned int> &init,
    ajn::ProxyBusObject &proxy,
    std::string interface) :
    proxy_(proxy),
    rated_export_power_(init["rated_export_power"]),
    rated_export_energy_(init["rated_export_energy"]),
    export_ramp_(init["export_ramp"]),
    rated_import_power_(init["rated_import_power"]),
    rated_import_energy_(init["rated_import_energy"]),
    import_ramp_(init["import_ramp"]),
    idle_losses_(init["idle_losses"]),
    export_power_(init["export_power"]),
    export_energy_(init["export_energy"]),
    import_power_(init["import_power"]),
    import_energy_(init["import_energy"]),
    export_watts_(init["export_power"]),
    import_watts_(init["import_power"]),
    delta_time_(0),
    interface_(interface) {
    //ctor
}

DistributedEnergyResource::~DistributedEnergyResource () {
    //dtor
}

// Remote Export Power
// - send control signal to remote device through alljoyn proxy method call
void DistributedEnergyResource::RemoteExportPower (unsigned int power) {
    DistributedEnergyResource::SetExportWatts (power);
    ajn::MsgArg arg("u",export_watts_);
    proxy_.MethodCall(interface_.c_str(),
                      "ExportPower",
                      &arg,
                      1,
                      ajn::ALLJOYN_FLAG_NO_REPLY_EXPECTED
    );
}  // end Remote Export Power

// Remote Import Power
// - send control signal to remote device through alljoyn proxy method call
void DistributedEnergyResource::RemoteImportPower (unsigned int power) {
    DistributedEnergyResource::SetImportWatts (power);
    ajn::MsgArg arg("u",import_watts_);
    proxy_.MethodCall(interface_.c_str(),
                      "ImportPower",
                      &arg,
                      1,
                      ajn::ALLJOYN_FLAG_NO_REPLY_EXPECTED
    );
}  // end Remote Import Power

// Control
// - check state of import / export power properties from main loop on a timer
void DistributedEnergyResource::Loop (float delta_time) {
    delta_time_ = delta_time;

    if (import_watts_ > 0) {
        DistributedEnergyResource::ImportPower ();
    } else if (export_watts_ > 0) {
        DistributedEnergyResource::ExportPower ();
    } else {
        IdleLoss ();
    }
}  // end Control

// Print
// - a method of quickly printing important properties of the DER
void DistributedEnergyResource::Print () {
    std::cout << "\n[DER]: " << proxy_.GetPath() << std::endl;
    std::cout
        << "\tExport Energy:\t" << export_energy_ << '\n'
        << "\tExport Power:\t" << export_power_ << '\n'
        << "\tExport watts:\t" << export_watts_ << '\n'
        << "\tImport Energy:\t" << import_energy_ << '\n'
        << "\tImport Power:\t" << import_power_ << '\n'
        << "\tImport watts:\t" << import_watts_ << '\n'<< std::endl;
}

// Set Export Watts
// - export watts is used as a control setpoint by ExportPower and turns import
// - power off.
void DistributedEnergyResource::SetExportWatts (unsigned int power) {
    import_watts_ = 0;
    import_power_ = 0;

    if (power > rated_export_power_) {
        export_watts_ = rated_export_power_;
    } else {
        export_watts_ = power;
    }
}  // end Set Export Watts

// Set Rated Export Power
// - set the watt value available to export to the grid
void DistributedEnergyResource::SetRatedExportPower (unsigned int power) {
    rated_export_power_ = power;
}  // end Rated Export Power

// Set Rated Export Energy
// - set the watt-hour value available to export to the grid
void DistributedEnergyResource::SetRatedExportEnergy (unsigned int energy) {
    rated_export_energy_ = energy;
}  // end Set Export Energy

// Set Export Power
// - regulates export power
void DistributedEnergyResource::SetExportPower (float power) {
    if (power > export_watts_) {
        export_power_ = export_watts_;
    } else if (power <= 0) {
        export_power_ = 0;
    } else {
        export_power_ = power;
    }
}  // end Set Export Power

// Set Export Energy
// - regulates export energy
void DistributedEnergyResource::SetExportEnergy (float energy) {
    if (energy > rated_export_energy_) {
        export_energy_ = rated_export_energy_;
    } else if (energy <= 0) {
        export_energy_ = 0;
    } else {
        export_energy_ = energy;
    }
}  // end Set Export Energy

// Set Export Ramp
// - set the watt per second value available to export to the grid
void DistributedEnergyResource::SetExportRamp (unsigned int ramp) {
    export_ramp_ = ramp;
}  // end Set Export Ramp

// Get Rated Export Power
// - get the rated watt value available to export to the grid
unsigned int DistributedEnergyResource::GetRatedExportPower () {
    return rated_export_power_;
}  // end Get Rated Export Power

// Get Rated Export Energy
// - get the watt value available to import from the grid
unsigned int DistributedEnergyResource::GetRatedExportEnergy () {
    return rated_export_energy_;
}  // end Rated Export energy

// Get Export Power
// - get the watt value available to export to the grid
unsigned int DistributedEnergyResource::GetExportPower () {
    unsigned int power = export_power_;
    return power;
}  // end Get Export Power

// Get Export Energy
// - get the watt-hour value available to export to the grid
unsigned int DistributedEnergyResource::GetExportEnergy () {
    unsigned int energy = export_energy_;
    return energy;
}  // end Get Export Energy

// Get Export Ramp
// - get the watt per second value available to export to the grid
unsigned int DistributedEnergyResource::GetExportRamp () {
    return export_ramp_;
}  // end Get Export Ramp

// Get Export Watts
// - get the control watts
unsigned int DistributedEnergyResource::GetExportWatts () {
    return export_watts_;
}  // end Get Export Watts

// Set Import Watts
// - turn off export power and set control setting for ImportPower method
void DistributedEnergyResource::SetImportWatts (unsigned int power) {
    export_watts_ = 0;
    export_power_ = 0;
    if (power > rated_import_power_) {
        import_watts_ = rated_import_power_;
    } else {
        import_watts_ = power;
    }
}  // end Set Import Watts

// Set Rated Import Power
// - set the watt value available to import from the grid
void DistributedEnergyResource::SetRatedImportPower (unsigned int power) {
    rated_import_power_ = power;
}  // end Set Rated Import Power

// Set Rated Import Energy
// - set the watt-hour value available to import from the grid
void DistributedEnergyResource::SetRatedImportEnergy (unsigned int energy) {
    rated_import_energy_ = energy;
}  // end Set Import Energy

// Set Import Power
// - regulates import power
void DistributedEnergyResource::SetImportPower (float power) {
    if (power > import_watts_) {
        import_power_ = import_watts_;
    } else if (power <= 0) {
        import_power_ = 0;
    } else {
        import_power_ = power;
    }
}  // end Set Import Power

// Set Import Energy
// - regulates import energy balance export energy
void DistributedEnergyResource::SetImportEnergy (float energy) {
    if (energy > rated_import_energy_) {
        import_energy_ = rated_import_energy_;
    } else if (energy <= 0) {
        import_energy_ = 0;
        import_watts_ = 0;  // stop importing
    } else {
        import_energy_ = energy;
    }
}  // end Set Import Energy

// Set Import Ramp
// - set the watt per second value available to import from the grid
void DistributedEnergyResource::SetImportRamp (unsigned int ramp) {
    import_ramp_ = ramp;
}  // end Set Import Ramp

// Get Rated Import Power
// - get the rated watt value available to import from the grid
unsigned int DistributedEnergyResource::GetRatedImportPower () {
    return rated_import_power_;
}  // end Rated Import Power

// Get Rated Import Energy
// - get the watt value available to import from the grid
unsigned int DistributedEnergyResource::GetRatedImportEnergy () {
    return rated_import_energy_;
}  // end Rated Import energy

// Get Import Power
// - get the watt value available to import from the grid
unsigned int DistributedEnergyResource::GetImportPower () {
    unsigned int power = import_power_;
    return power;
}  // end Get Import Power

// Get Import Energy
// - get the watt-hour value available to import from the grid
unsigned int DistributedEnergyResource::GetImportEnergy () {
    unsigned int energy = import_energy_;
    return energy;
}  // end Get Import Energy

// Get Import Ramp
// - get the watt per second value available to import from the grid
unsigned int DistributedEnergyResource::GetImportRamp () {
    return import_ramp_;
}  // end Get Import Ramp

// Get Import Watts
// - get the control watts
unsigned int DistributedEnergyResource::GetImportWatts () {
    return import_watts_;
}  // end Get Import Watts

// Set Idle Losses
// - set the watt-hours per hour loss when idle
void DistributedEnergyResource::SetIdleLosses (unsigned int losses) {
    idle_losses_ = losses;
}  // end Set Idle Losses

// Get Idle Losses
// - get the watt-hours per hour loss when idle
unsigned int DistributedEnergyResource::GetIdleLosses () {
    return idle_losses_;
}  // end Get Idle Losses

// Get Path
// - get the path to the DER
std::string DistributedEnergyResource::GetPath () {
    return proxy_.GetPath ();
}  // end Get Idle Losses

// Get UID
// - get the unique ID to the DER
std::string DistributedEnergyResource::GetUID () {
    return proxy_.GetUniqueName ();
}  // end Get UID

// Import Power
// - calculate power/energy change 
// - degrement import energy and increment export energy
void DistributedEnergyResource::ImportPower () {
    // regulate import power
    float seconds = delta_time_ / 1000;
    float ramp_watts = import_ramp_ * seconds;
    if (import_power_ == import_watts_){
        // do nothing
    } else if (import_power_ < import_watts_) {
        DistributedEnergyResource::SetImportPower (import_power_ + ramp_watts);
    } else if (import_power_ > import_watts_) {
        DistributedEnergyResource::SetImportPower (import_watts_ - ramp_watts);
    }

    // regulate energy
    float hours = seconds / (60*60);
    float watt_hours = 0;

    // if the import power didn't reach rated then include ramp in energy calc
    // this function does not account for the cycle where it reaches rated,
    // but the error is negligable comparted to rated energy
    if (import_power_ < rated_import_power_) {
        // area under the linear function
        watt_hours += import_power_ * hours;  // area under triangle
        watt_hours += ramp_watts * hours/2;   // area of triangle
        DistributedEnergyResource::SetImportEnergy(import_energy_ - watt_hours);
        DistributedEnergyResource::SetExportEnergy(export_energy_ + watt_hours);
    } else {
        // area under the linear function excluding ramp.
        // note: this will exclude some energy if it reached peak within bounds
        watt_hours += import_power_ * hours;
        DistributedEnergyResource::SetImportEnergy(import_energy_ - watt_hours);
        DistributedEnergyResource::SetExportEnergy(export_energy_ + watt_hours);
    }
}  // end Import Power

// Export Power
// - calculate power/energy change 
// - decrement export energy and increment import energy
void DistributedEnergyResource::ExportPower () {
    float seconds = delta_time_ / 1000;
    float ramp_watts = export_ramp_ * seconds;
    if (export_power_ == export_watts_) {
        // do nothing
    } else if (export_power_ < export_watts_) {
        DistributedEnergyResource::SetExportPower (export_power_ + ramp_watts);
    } else if (export_power_ > export_watts_) {
        DistributedEnergyResource::SetExportPower (export_power_ - ramp_watts);
    }

    // regulate energy
    float hours = seconds / (60*60);
    float watt_hours = 0;

    // if the import power didn't reach rated then include ramp in energy calc
    // this function does not account for the cycle where it reaches rated,
    // but the error is negligable comparted to rated energy
    if (export_power_ < rated_export_power_) {
        // area under the linear function
        watt_hours += export_power_ * hours;  // area under triangle
        watt_hours += ramp_watts * hours/2;   // area of triangle
        DistributedEnergyResource::SetImportEnergy(import_energy_ + watt_hours);
        DistributedEnergyResource::SetExportEnergy(export_energy_ - watt_hours);
    } else {
        // area under the linear function excluding ramp.
        // note: this will exclude some energy if it reached peak within bounds
        watt_hours += export_power_ * hours;
        DistributedEnergyResource::SetImportEnergy(import_energy_ + watt_hours);
        DistributedEnergyResource::SetExportEnergy(export_energy_ - watt_hours);
    }
}  // end Export Power

// Idle Loss
// - update energy available based on energy lost
void DistributedEnergyResource::IdleLoss () {
    // set import/export power to zero
    DistributedEnergyResource::SetImportPower (0);
    DistributedEnergyResource::SetExportPower (0);
    // set import / export idle loss energy changes
    float seconds = delta_time_ / 1000;
    float hours = seconds / (60*60);
    float energy_loss = idle_losses_ * hours;
    DistributedEnergyResource::SetImportEnergy(import_energy_ + energy_loss);
    DistributedEnergyResource::SetExportEnergy(export_energy_ - energy_loss);
}  // end Idle Loss