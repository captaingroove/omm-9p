/***************************************************************************|
|  OMM - Open Multimedia                                                    |
|                                                                           |
|  Copyright (C) 2009, 2010, 2011, 2012, 2022                               |
|  Jörg Bakker                                                              |
|                                                                           |
|  This file is part of OMM.                                                |
|                                                                           |
|  OMM is free software: you can redistribute it and/or modify              |
|  it under the terms of the MIT License                                    |
 ***************************************************************************/

#include <fcntl.h>

#include "DvbLogger.h"
#include "DvbUtil.h"
#include "Dvr.h"
#include "Device.h"
#include "Remux.h"


namespace Omm {
namespace Dvb {

Dvr::Dvr(Adapter* pAdapter, int num) :
_pAdapter(pAdapter),
_num(num),
_pRemux(0),
_fileDescDvr(-1)
{
    _deviceName = _pAdapter->_deviceName + "/dvr" + Poco::NumberFormatter::format(_num);
}


Dvr::~Dvr()
{
}


void
Dvr::openDvr()
{
    if ((_fileDescDvr = open(_deviceName.c_str(), O_RDONLY | O_NONBLOCK)) < 0) {
        LOG(dvb, error, "failed to open dvb rec device \"" + _deviceName + "\": " + strerror(errno));
    }
    _pRemux = new Remux(_fileDescDvr);
    _pRemux->startRemux();
}


void
Dvr::closeDvr()
{
    if (_pRemux) {
        _pRemux->stopRemux();
        _pRemux->waitForStopRemux();
        _pRemux->flush();
        delete _pRemux;
        if (close(_fileDescDvr)) {
            LOG(dvb, error, "failed to close dvb rec device \"" + _deviceName + "\": " + strerror(errno));
        }
        _fileDescDvr = -1;
    }
}


Service*
Dvr::addService(Service* pService)
{
    if (_pRemux) {
        return _pRemux->addService(pService);
    }
    else {
        return 0;
    }
}


void
Dvr::delService(Service* pService)
{
    if (_pRemux) {
        _pRemux->delService(pService);
    }
}

}  // namespace Omm
}  // namespace Dvb
