/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
**********************************************************/
//
// Author: Tylor slay
// Description: This class will create a digital twin for each DCS that is
// - discovered and removed the digital twin when it is removed from the system.
// - It will also listen for property changes within the DCS and update the
// - digital twim properties. 

#ifndef CLIENTLISTENER_H_INCLUDED
#define CLIENTLISTENER_H_INCLUDED

#include "Aggregator.h"

class ClientListener : public ajn::MessageReceiver,
                       public ajn::Observer::Listener,
                       public ajn::ProxyBusObject::PropertiesChangedListener {
public :
    // member functions
    ClientListener (ajn::BusAttachment* bus,
                    ajn::Observer* obs,
                    Aggregator* vpp,
                    const char* client_name);
    void ObjectDiscovered (ajn::ProxyBusObject& proxy);
    void ObjectLost (ajn::ProxyBusObject& proxy);
    void PropertiesChanged (ajn::ProxyBusObject& proxy,
                            const char* interface_name,
                            const ajn::MsgArg& changed,
                            const ajn::MsgArg& invalidated,
                            void* context);
private:
    // composition
    ajn::BusAttachment* bus_ptr_;
    ajn::Observer* obs_ptr_;
    Aggregator* vpp_ptr_;


    // properties
    const char* client_interface_;
    static const char* props_[];

    // Methods
    std::map <std::string, unsigned int> MapProperties (
        const ajn::MsgArg& properties
    );
};

#endif // CLIENTLISTENER_H_INCLUDED
