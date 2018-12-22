/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *    Copyright (c) V2 Systems, LLC.  All rights reserved.
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
// Description: This class will be used to set the price of power for DER and
// - sycronize the time. 

#ifndef SMARTGRIDDEVICE_HPP_INCLUDED
#define SMARTGRIDDEVICE_HPP_INCLUDED

class SmartGridDevice : public ajn::BusObject {
public:
    // member methods
    SmartGridDevice (ajn::BusAttachment* bus,
    				 Aggregator* vpp,
                     const char* name, 
                     const char* path);
    virtual ~SmartGridDevice ();
    QStatus Get (const char* interface, 
                 const char* property, 
                 ajn::MsgArg& value
    );
    QStatus SendPropertiesUpdate ();
    void Loop ();

private:
    // class composition
    ajn::BusAttachment* bus_ptr_;
    Aggregator* vpp_ptr_;
    const ajn::InterfaceDescription::Member* signal_;
    // properties
    const char* interface_;
    const char* name_;
    int price_;
    unsigned int time_;


};

#endif // SMARTGRIDDEVICE_HPP_INCLUDED
