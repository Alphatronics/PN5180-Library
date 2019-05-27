// NAME: PN5180_trace.h
//
// DESC: Helper methods for debugging
//
// Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
//
// This file is part of the PN5180 library for the Arduino environment.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
#ifndef PN5180_TRACE_H
#define PN5180_TRACE_H

#include "mbed_trace.h"

#ifndef MBED_CONF_MBED_TRACE_ENABLE
#define MBED_CONF_MBED_TRACE_ENABLE 0
#endif
#if MBED_CONF_MBED_TRACE_ENABLE == 1
#define TRACE_GROUP     "PN5180"
#define DEBUG_PN5180    1
extern char * formatHex(const uint8_t val);
extern char * formatHex(const uint16_t val);
extern char * formatHex(const uint32_t val);

#else
#define DEBUG_PN5180    0
#endif //MBED_CONF_MBED_TRACE_ENABLE

#endif // PN5180_TRACE_H
