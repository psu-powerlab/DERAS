#include <alljoyn/Status.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/BusAttachment.h>
#include "include/Aggregator.h"
#include "include/SmartGridDevice.h"

// Constructor
// - initialize bus object interface and smart grid device properties
SmartGridDevice::SmartGridDevice (ajn::BusAttachment* bus,
								                  Aggregator* vpp,
                                  const char* name,
                                  const char* path) : ajn::BusObject(path),
                                                      bus_ptr_(bus),
                                                      vpp_ptr_(vpp),
                                                      signal_(NULL),
                                                      interface_(name),
                                                      price_(0),
                                                      time_(0),
                                                      temp_(0) {
    const ajn::InterfaceDescription* interface 
    	= bus_ptr_->GetInterface(interface_);
    assert(interface != NULL);
    AddInterface(*interface, ANNOUNCED);

}

// Destructor
SmartGridDevice::~SmartGridDevice () {
  // do nothing
}  // end Destructor

// Get
// - this method will be called by DCS looking to get the updated DERAS
// - properties
QStatus SmartGridDevice::Get (const char* interface,
                              const char* property,
                              ajn::MsgArg& value) {
    QStatus status;
    if (strcmp(interface, interface_)) {
        return ER_FAIL;
    }

    if (!strcmp(property,"time")) {
        status = value.Set("u", time_);
        return status;
    } else if (!strcmp(property,"price")) {
        status = value.Set("i", price_);
        return status;
    } else if (!strcmp(property,"temperature")) {
        status = value.Set("i", temp_);
        return status;
    } else {
        return ER_FAIL;
    }
} // end Get

QStatus SmartGridDevice::SendPropertiesUpdate () {
  const char* props[] = { "time",
                          "price",
                          "temperature"};
    QStatus status;
    status = EmitPropChanged (interface_, props, 2, ajn::SESSION_ID_ALL_HOSTED);
    std::cout << "[AllJoyn]: DERAS properties changed signal." << std::endl;
    return status;
}  // end Send Properties Update

// Loop
// - if DERAS price or time have changed then send a property update signal to
// - connected DCS. This doesn't need a loop to restrict its frequency as it is
// - dependent on DERAS's property changes.
void SmartGridDevice::Loop () {
    int price = vpp_ptr_->GetPrice ();
    int temp = vpp_ptr_->GetTemperature ();
    unsigned int time = vpp_ptr_->GetTime ();
    if (time != time_ || price != price_ || temp != temp_) {
        time_ = time;
        price_ = price;
        temp_ = temp;
    	SmartGridDevice::SendPropertiesUpdate ();
    }
}