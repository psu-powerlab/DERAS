#include <iostream>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Observer.h>
#include "include/ClientListener.h"
#include "include/logger.h"

// (TS): this is the only way I could find to initialize a const char* array.
//       AllJoyn documentation states "NULL" for registering all properties, but
//       that didn't seem to work. 
const char* ClientListener::props_[] = {"rated_export_power",
                                        "rated_export_energy",
                                        "export_power",
                                        "export_energy",
                                        "export_ramp",
                                        "rated_import_power",
                                        "rated_import_energy",
                                        "import_power",
                                        "import_energy",
                                        "import_ramp",
                                        "idle_losses"};

ClientListener::ClientListener(
    ajn::BusAttachment* bus,
    ajn::Observer* obs,
    Aggregator* vpp,
    const char* client_name) : bus_ptr_(bus),
                               obs_ptr_(obs),
                               vpp_ptr_(vpp),
                               client_interface_(client_name){
} // end ClientListener

// ObjectDiscovered
// - a DCS has advertised the interface we are looking for so DERAS
// - request and update of it's current properties to ensure the digital twin is
// - up to date.
void ClientListener::ObjectDiscovered (ajn::ProxyBusObject& proxy) {
    std::string path = proxy.GetPath();
    std::string service_name = proxy.GetServiceName();
    std::string name = proxy.GetUniqueName ();

    std::cout << "\n[LISTENER]\n";
    std::cout << "\tPath = " << path << '\n';
    std::cout << "\tService Name = " << service_name << '\n';
    std::cout << "\tUID = " << name << '\n';

    Logger("Resources", "/home/deras/dev/LOGS/")
        << name << '\t'
        << path << '\t'
        << "found";


    bus_ptr_->EnableConcurrentCallbacks();
    proxy.RegisterPropertiesChangedListener(
        client_interface_, props_, 11, *this, NULL
    );

    ajn::MsgArg values;
    proxy.GetAllProperties (client_interface_, values);

    std::map <std::string, unsigned int> init;
    init = ClientListener::MapProperties (values);

    vpp_ptr_->AddResource (init, proxy);
} // end ObjectDiscovered

// ObjectLost
// - the remote device is no longer available
// - RemoveResource from aggregator using path
void ClientListener::ObjectLost (ajn::ProxyBusObject& proxy) {
    std::string name = proxy.GetUniqueName();
    std::string path = proxy.GetPath();

    std::cout << "\n[LISTENER] : " << name << " connection lost\n";
    std::cout << "\tPath : " << path << " no longer exists\n";

    Logger("Resources", "/home/deras/dev/LOGS/")
        << name << '\t'
        << path << '\t'
        << "lost";

    vpp_ptr_->RemoveResource (name);
} // end ObjectLost

// PropertiesChanged
// - callback to recieve property changed event from remote bus object
void ClientListener::PropertiesChanged (ajn::ProxyBusObject& obj,
                                        const char* interface_name,
                                        const ajn::MsgArg& changed,
                                        const ajn::MsgArg& invalidated,
                                        void* context) {
    std::map <std::string, unsigned int> init;
    init = ClientListener::MapProperties (changed);
    vpp_ptr_->UpdateResource (init, obj.GetUniqueName ());
} // end PropertiesChanged

std::map <std::string, unsigned int> ClientListener::MapProperties (
    const ajn::MsgArg& properties) {
    std::map <std::string, unsigned int> init;
    size_t nelem = 0;
    ajn::MsgArg* elems = NULL;
    QStatus status = properties.Get("a{sv}", &nelem, &elems);
    if (status == ER_OK) {
        const char* name;
        unsigned int property;
        ajn::MsgArg* val;
        for (size_t i = 0; i < nelem; i++) {
            status = elems[i].Get("{sv}", &name, &val);
            val->Get("u", &property);
            init[name] = property;
        }
    }
    return init;
}
