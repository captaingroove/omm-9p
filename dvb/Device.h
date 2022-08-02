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

#ifndef DvbDevice_INCLUDED
#define DvbDevice_INCLUDED

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>

#include <Poco/Timestamp.h>
#include <Poco/Thread.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Mutex.h>
#include <Poco/Logger.h>
#include <Poco/Format.h>
#include <Poco/StringTokenizer.h>
#include <Poco/DOM/Document.h>
#include "Poco/Notification.h"

#include "AvStream.h"

namespace Omm {
namespace Dvb {

class UnixFileIStream;
class Section;
class Stream;
class Service;
class Device;
class Frontend;
class Transponder;
class SignalCheckThread;
class Lnb;
class Frontend;
class Demux;
class Mux;
class Dvr;
class Adapter;


class ScanNotification : public Poco::Notification
{
    friend class Frontend;
public:
    Service* getService() { return _pService; }
    
private:
    ScanNotification(Service* pService) : _pService(pService) {}

    Service*    _pService;
};


class Adapter
{
    friend class Device;
    friend class Frontend;
    friend class Demux;
    friend class Dvr;

public:
    Adapter(int num);
    ~Adapter();

    typedef std::vector<Frontend*>::iterator FrontendIterator;
    FrontendIterator frontendBegin();
    FrontendIterator frontendEnd();

    void addFrontend(Frontend* pFrontend);
    void openAdapter();
    void closeAdapter();

    std::string getId();
    void setId(const std::string& id);

    void readXml(Poco::XML::Node* pXmlAdapter);
    void writeXml(Poco::XML::Element* pDvbDevice);

private:
    int                         _num;
    std::string                 _id;
    std::string                 _deviceName;
    std::vector<Frontend*>      _frontends;
};


class Device
{
    friend class Adapter;
    friend class SignalCheckThread;

public:
    typedef enum { ModeDvr, ModeMultiplex, ModeDvrMultiplex, ModeElementaryStreams } Mode;

    static Device* instance();

    typedef std::map<std::string, std::vector<Transponder*> >::iterator ServiceIterator;
    ServiceIterator serviceBegin();
    ServiceIterator serviceEnd();

    typedef std::map<std::string, Adapter*>::iterator AdapterIterator;
    AdapterIterator adapterBegin();
    AdapterIterator adapterEnd();

    void addInitialTransponders(const std::string& frontendType, const std::string& initialTransponders);
    void detectAdapters();
    void open();
    void close();
    void scan();
    void readXml(std::istream& istream);
    void writeXml(std::ostream& ostream);

    Transponder* getFirstTransponder(const std::string& serviceName);
    std::vector<Transponder*>& getTransponders(const std::string& serviceName);

    std::istream* getStream(const std::string& serviceName);
    AvStream::ByteQueue* getByteQueue(const std::string& serviceName);
    void freeStream(std::istream* pIstream);
    void freeByteQueue(AvStream::ByteQueue* pIstream);

private:
    Device();
    ~Device();

    Adapter* addAdapter(const std::string& id, int adapterNum);
    void initServiceMap();
    void clearServiceMap();
    void clearAdapters();
    Transponder* tuneToService(const std::string& serviceName, bool unscrambledOnly = true);
    Service* startService(Service* pService);
    void stopService(Service* pService);
    void stopServiceStreamsOnTransponder(Transponder* pTransponder);

    static Device*                                      _pInstance;

    std::map<std::string, Adapter*>                     _adapters;
    std::map<std::string, std::vector<Transponder*> >   _serviceMap;
    std::map<std::istream*, Service*>                   _streamMap;
    std::map<AvStream::ByteQueue*, Service*>            _bytequeueMap;
    std::map<std::string, std::set<std::string> >       _initialTransponders;

    Poco::FastMutex                                     _deviceLock;
};

}  // namespace Omm
}  // namespace Dvb

#endif
