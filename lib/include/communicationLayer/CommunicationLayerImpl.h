//
// Created by Peter Zdankin on 07.03.18.
//

#ifndef COMMUNICATIONMODULE_COMMUNICATIONLAYERIMPL_H
#define COMMUNICATIONMODULE_COMMUNICATIONLAYERIMPL_H

#include "lib/include/communicationLayer/CommunicationLayer.h"
#include "lib/include/RuntimeLibraryInterface.h"
#include "lib/include/Peripheral/PeripheralInterface.h"

CommunicationLayer *CL_createCommunicationLayer(PeripheralInterface *spi, Allocator allocate);

#endif //COMMUNICATIONMODULE_COMMUNICATIONLAYERIMPL_H
