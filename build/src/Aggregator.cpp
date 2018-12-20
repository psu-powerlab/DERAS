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
	total_export_energy_(0),
	total_export_power_(0),
	total_import_energy_(0),
	total_import_power_(0),
	export_watts_(0),
	import_watts_(0) {
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

    // check import / export watts
	if (import_watts_ > 0) {
        Aggregator::ImportPower ();
    } else if (export_watts_ > 0) {
        Aggregator::ExportPower ();
    }
    Aggregator::Log ();
}  // end Loop

// Log
// - log all resource properties based on the set log increment.
// - unlike normal logging, all resource properties are stored in a string
// - matrix before writting to a file to reduce the number of times the file
// - must be opened and closed.
void Aggregator::Log () {
    // log resources based on elapsed time
    unsigned int utc = time(0);
    if (utc != last_log_ && utc % log_inc_ == 0) {
    	Logger ("DATA", log_path_);
    for (const auto &resource : resources_) {
		Logger("DATA")
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
		last_log_ = seconds;
    }
}

void Aggregator::DisplayAllResources () {
    std::cout << "\nAll Resources:" << std::endl;
    SortImportEnergy();
    for (const auto &resource : resources_) {
        resource->Print ();
    }
}

void Aggregator::DisplayTargetResources () {
    std::cout << "\nTarget Resources:" << std::endl;
    SortImportEnergy();
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
		total_export_power_ += resource->GetExportPower ();
		total_import_energy_ += resource->GetImportEnergy ();
		total_import_power_ += resource->GetImportPower ();    	
    }
}

void Aggregator::DisplayTotals () {
    std::cout << "\nAggregated Properties:!"
		<< "\n\tTotal Export Energy = " << total_export_energy_
		<< "\n\tTotal Export Power = " << total_export_power_
		<< "\n\tTotal Import Energy = " << total_import_energy_
		<< "\n\tTotal Import Power = " << total_import_power_ << std::endl;
}

void Aggregator::FilterResources () {
    sub_resources_.clear();
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

// Sort Import Energy
// - sort resources by ramp rate, then energy available.
// - TODO (TS): since EWH are the fastest ramping devices we have this ensure 
// - we use their energy as soon as possbile, but we need to create a better
// - system for dispatching our varied resources.
void Aggregator::SortImportEnergy () {
	// lambda function to compare resources
	bool CompareResources = [] (
			const std::shared_ptr <DistributedEnergyResource> lhs,
			const std::shared_ptr <DistributedEnergyResource> rhs) {
		// ramp rate check
		if (lhs->GetRatedImportRamp () != rhs->GetRatedImportRamp ()) {
			return (lhs->GetRatedImportRamp () > rhs->GetRatedImportRamp ());
		}

		// energy check
		return (lhs->GetImportEnergy () > rhs->GetImportEnergy ());
	};

    std::sort(sub_resources_.begin(),sub_resources_.end(), CompareResources);
}

// Sort Export Energy
// - sort resources by ramp rate, then energy available.
// - TODO (TS): create a better system for dispatching our varied resources.
void Aggregator::SortExportEnergy () {
	// lambda function to compare resources
	bool CompareResources = [] (
			const std::shared_ptr <DistributedEnergyResource> lhs,
			const std::shared_ptr <DistributedEnergyResource> rhs) {
		// ramp rate check
		if (lhs->GetRatedExportRamp () != rhs->GetRatedExportRamp ()) {
			return (lhs->GetRatedExportRamp () > rhs->GetRatedExportRamp ());
		}

		// energy check
		return (lhs->GetExportEnergy () > rhs->GetExportEnergy ());
	};

    std::sort(sub_resources_.begin(),sub_resources_.end(), CompareResources);
}

// Export Power
// - loop through target resources and send export signal based on greatest
// - export energy available. The signal sets both the "digital twin" and the 
// - remote devices control watts;
// - (TS): note we could also check to see if it is already dispatched to reduce 
// - 	   data transfer.
// - (TS): depending on the service we may need to check to see if the resource
// - 	   can support the dispatch power for the full period.
void Aggregator::ExportPower (unsigned int dispatch_power) {
    Aggregator::SortExportEnergy ();

    unsigned int power = 0;
    for (auto &resource : sub_resources_) {
		if (dispatch_power > 0) {
		    // Digital Twin
		    power = resource->GetRatedExportPower ();
		    resource->SetExportWatts (power);

		    // AllJoyn Method Call
		    resource->RemoteExportPower (power);

		    // subtract resources power from dispatch power
		    if (dispatch_power > power) {
		    	dispatch_power -= power;
		    } else {
		   		dispatch_power = 0;
		    }
		} else {
		    break;
		}
    }
}  // end Export Power

// Import Power
void Aggregator::ImportPower (unsigned int dispatch_power) {
    Aggregator::SortImportEnergy ();

    unsigned int power = 0;
    for (auto &resource : sub_resources_) {
		if (dispatch_power > 0) {
		    // Digital Twin
		    power = resource->GetRatedImportPower ();
		    resource->SetImportWatts (power);

		    // AllJoyn Method Call
		    resource->RemoteImportPower (power);

		    // subtract resources power from dispatch power
		    if (dispatch_power > power) {
		    	dispatch_power -= power;
		    } else {
		   		dispatch_power = 0;
		    }
		} else {
		    break;
		}
    }
}  // end Import Power
