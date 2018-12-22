#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/Status.h>
#include "include/Aggregator.h"
#include "include/logger.h"

// constructor
// - simply initialize member properties
Aggregator::Aggregator (tsu::config_map &init) :
	config_(init),
	last_log_(0),
	log_inc_(stoul(init["Logger"]["increment"])),
	log_path_(init["Logger"]["path"]),
	total_export_energy_(0),
	total_export_power_(0),
	total_import_energy_(0),
	total_import_power_(0),
	export_watts_(0),
	import_watts_(0),
	price_(0),
	time_(0) {
	// do nothing
}  // end constructor

Aggregator::~Aggregator () {
	// do nothing
}  // end destructor

// Set Targets
// - store the new target arguments and then filter the current resources
void Aggregator::SetTargets (const std::vector <std::string> &targets) {
	targets_ = targets;
	Aggregator::FilterResources ();
}  // end Set Targets

// Set Export Watts
// - set the dispatch watts for DER to export to the grid
void Aggregator::SetExportWatts (unsigned int power) {
	import_watts_ = 0;
	if (power > total_export_power_) {
		export_watts_ = total_export_power_;
	} else {
		export_watts_ = power;
	}
}  // end Set Export Watts

// Set Import Watts
// - set the dispatch watts for DER to import from the grid
void Aggregator::SetImportWatts (unsigned int power) {
	export_watts_ = 0;
	if (power > total_import_power_) {
		import_watts_ = total_import_power_;
	} else {
		import_watts_ = power;
	}
}  // end Set Import Watts

// Set Price
// - the tenths of a cent per kWh
void Aggregator::SetPrice (unsigned int price) {
	price_ = price;
}  // end Set Price

// Set Time
// - the UTC time as seconds from epoch
void Aggregator::SetTime (unsigned int utc) {
	time_ = utc;
}  // end Set Time

// Get Total Export Energy
// - return the filtered resource Watt-hours available to export to the grid
unsigned int Aggregator::GetTotalExportEnergy () {
    return total_export_energy_;
}  // end Get Total Export Energy

// Get Total Export Power
// - return the filtered resource Watts available to export to the grid
unsigned int Aggregator::GetTotalExportPower () {
    return total_export_power_;
}  // end Get Total Export Power

// Get Total Import Energy
// - return the filtered resource Watt-hours available to import from the grid
unsigned int Aggregator::GetTotalImportEnergy () {
    return total_import_energy_;
}  // end Get Total Import Energy

// Get Total Import Power
// - return the filtered resource Watts available to import from the grid
unsigned int Aggregator::GetTotalImportPower () {
    return total_import_power_;
}  // end Get Total Import Power

// Get Price
// - return tenths of a cent per kWh for electricity
unsigned int Aggregator::GetPrice () {
	return price_;
}  // end Get Price

// Get Time
// - return the UTC time
unsigned int Aggregator::GetTime () {
	return time_;
}  // end Get Time

// Add Resource
// - This is used by the Client Listener class to add newly discovered DER.
// - it also passes the AllJoyn Proxy Bus Object that will be used to control
// - individual DER when desired
void Aggregator::AddResource (
	std::map <std::string, unsigned int>& init,
	ajn::ProxyBusObject &proxy) {
	std::string interface = config_["AllJoyn"]["client_interface"];
	std::shared_ptr <DistributedEnergyResource> 
		der (new DistributedEnergyResource (init, proxy, interface));
	resources_.push_back (std::move (der));
	Aggregator::FilterResources ();
}  // end Add Resource

// Update Resource
// - when the Client Listener gets a property update signal it will look for the
// - correct resource and update it's properties
void Aggregator::UpdateResource (std::map <std::string, unsigned int>& init,
				 				 const std::string& path) {
	bool found = false;
	for (auto& resource : resources_) {
		if (resource->GetPath() == path) {
			resource->SetRatedExportEnergy (init["rated_export_energy"]);
			resource->SetRatedExportPower (init["rated_export_power"]);
			resource->SetExportEnergy (init["export_energy"]);
			resource->SetExportPower (init["export_power"]);
			resource->SetExportRamp (init["export_ramp"]);
			resource->SetRatedImportEnergy (init["rated_import_energy"]);
			resource->SetRatedImportPower (init["rated_import_power"]);
			resource->SetImportEnergy (init["import_energy"]);
			resource->SetImportPower (init["import_power"]);
			resource->SetImportRamp (init["import_ramp"]);
			resource->SetIdleLosses (init["idle_losses"]);
			found = true;
		} 
	}

	if (found) {
		// do nothing
	} else {
		std::cout 
			<< "Property update signal recieved from unknown resource!" 
			<< std::endl;
	}
}  // end Update Resource

// Remove Resource
// - if the Client Listener recieves a object loss signal then it will remove
// - it from the resource list
void Aggregator::RemoveResource (const std::string& path) {
    // (TS): unlike a normal for loop, this loops iterator must be
    //       incremented only if the element is not deleted or you get
    //       memory leaks.
    for (auto it = resources_.begin(); it != resources_.end();) {
        if ((*it)->GetPath ().find(path) != std::string::npos) {
            it = resources_.erase (it);
        } else {
            it++;
        }
    }
    Aggregator::FilterResources ();
}  // end Remove Resource

// Loop
// - check the import and export watts to disptach remote devices and
// - digital twins. also call the log function to log all discovered DER 
// - properties
void Aggregator::Loop (float delta_time) {
    // update all resources
    for (auto &resource : resources_) {
		resource->Loop (delta_time);
    }
	Aggregator::UpdateTotals ();
	Aggregator::ExportPower ();
	Aggregator::ImportPower ();
    Aggregator::Log ();
}  // end Loop

// Log
// - log all resource properties based on the set log increment.
// - TODO (TS): I should write this function to write to file once instead of
// - for each resource, but we will see how much of performance loss we have.
void Aggregator::Log () {
    // log resources based on elapsed time
    unsigned int utc = time(0);
    if (utc != last_log_ && utc % log_inc_ == 0) {
	    for (const auto &resource : resources_) {
			Logger("DATA", log_path_)
				<< resource->GetPath () << '\t'
				<< resource->GetExportRamp () << '\t'
				<< resource->GetRatedExportPower () << '\t'
				<< resource->GetRatedExportEnergy () << '\t'
				<< resource->GetExportPower () << '\t'
				<< resource->GetExportEnergy () << '\t'
				<< resource->GetImportRamp () << '\t'
				<< resource->GetRatedImportPower () << '\t'
				<< resource->GetRatedImportEnergy () << '\t'
				<< resource->GetImportPower () << '\t'
				<< resource->GetImportEnergy ();
		    }
			last_log_ = utc;
	    }
}

void Aggregator::DisplayAllResources () {
    std::cout << "\nAll Resources:" << std::endl;
    for (const auto &resource : resources_) {
        resource->Print ();
    }
}

void Aggregator::DisplayTargetResources () {
    std::cout << "\nTarget Resources:" << std::endl;
    for (const auto &resource : sub_resources_) {
        resource->Print ();
    }
}

void Aggregator::UpdateTotals () {
	total_export_energy_ = 0;
	total_export_power_ = 0;
	total_import_energy_ = 0;
	total_import_power_ = 0;
    for (const auto &resource : sub_resources_) {
		total_export_energy_ += resource->GetExportEnergy ();
		total_export_power_ += resource->GetRatedExportPower ();
		total_import_energy_ += resource->GetImportEnergy ();
		total_import_power_ += resource->GetRatedImportPower ();    	
    }
}

void Aggregator::DisplaySummary () {
    std::cout << "\nAggregated Properties:!"
		<< "\n\tTotal Export Energy = " << total_export_energy_
		<< "\n\tTotal Export Power = " << total_export_power_
		<< "\n\tTotal Import Energy = " << total_import_energy_
		<< "\n\tTotal Import Power = " << total_import_power_ << std::endl;
}

// Filter Resources
// - filter resources by target arguments.
// - if target arguments is empty, then default to all resources
void Aggregator::FilterResources () {
    sub_resources_.clear();
    if (targets_.size() == 0) {
    	sub_resources_ = resources_;
    } else {
	    for (const auto &resource : resources_) {
	        bool found;
	        for (unsigned int i = 0; i < targets_.size(); i++) {
	            if (resource->GetPath().find(targets_[i])!= std::string::npos) {
	                found = true;
	            } else {
	                found = false;
	                break;
	            }
	        }
	        if (found) {
	            sub_resources_.push_back(resource);
	        }
	    }	
    }

}

// Export Power
// - loop through target resources and send export signal based on greatest
// - export energy available. The signal sets both the "digital twin" and the 
// - remote devices control watts;
void Aggregator::ExportPower () {
	// sort by ramp first then export energy
    std::sort(
    	sub_resources_.begin(),sub_resources_.end(), [] (
	const std::shared_ptr <DistributedEnergyResource> lhs,
	const std::shared_ptr <DistributedEnergyResource> rhs) {
		// ramp rate check
		if (lhs->GetExportRamp () != rhs->GetExportRamp ()) {
			return (lhs->GetExportRamp () > rhs->GetExportRamp ());
		}

		// energy check
		return (lhs->GetExportEnergy () > rhs->GetExportEnergy ());
	});

    unsigned int dispatch_power = export_watts_;
    unsigned int power = 0;
    for (auto &resource : sub_resources_) {
		if (dispatch_power > 0) {
			power = resource->GetRatedExportPower ();
		    if (resource->GetExportPower () == 0) {
		   		// AllJoyn Method Call and digital twin
			    resource->RemoteExportPower (power);
		    }
		    // subtract resources power from dispatch power
		    if (dispatch_power > power) {
		    	dispatch_power -= power;
		    } else {
		   		dispatch_power = 0;
		    }
		// once dispatch has been met tell other resources to stop exporting
		} else {
		    if (resource->GetExportPower () != 0) {
		    	std::cout << "DEBUG: export power off\n";
			    // AllJoyn Method Call
			    resource->RemoteExportPower (0);
		    }
		}
    }
}  // end Export Power

// Import Power
// - loop through target resources and send import signal based on greatest
// - import energy available. The signal sets both the "digital twin" and the 
// - remote devices control watts;
void Aggregator::ImportPower () {
	// sort by ramp first then by import power
    std::sort(
    	sub_resources_.begin(),sub_resources_.end(), [] (
		const std::shared_ptr <DistributedEnergyResource> lhs,
		const std::shared_ptr <DistributedEnergyResource> rhs) {
		// ramp rate check
		if (lhs->GetImportRamp () != rhs->GetImportRamp ()) {
			return (lhs->GetImportRamp () > rhs->GetImportRamp ());
		}

		// energy check
		return (lhs->GetImportEnergy () > rhs->GetImportEnergy ());
	});

    unsigned int dispatch_power = import_watts_;
    unsigned int power = 0;
    for (auto &resource : sub_resources_) {
		if (dispatch_power > 0) {
			power = resource->GetRatedImportPower ();
		    if (resource->GetImportPower () == 0) {
			    // AllJoyn Method Call and digital twin
			    resource->RemoteImportPower (power);
		    }

		    // subtract resources power from dispatch power
		    if (dispatch_power > power) {
		    	dispatch_power -= power;
		    } else {
		   		dispatch_power = 0;
		    }
		// once dispatch has been met tell other resources to stop importing
		} else {
		    if (resource->GetImportPower () != 0) {
		    	std::cout << "DEBUG: import power off\n";
			    // AllJoyn Method Call
			    resource->RemoteImportPower (0);
		    }
		}
    }
}  // end Import Power
