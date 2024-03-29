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

#ifndef Service_INCLUDED
#define Service_INCLUDED

#include <queue>
#include <stack>

#include <Poco/DOM/DOMException.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/AttrMap.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Attr.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DocumentFragment.h>
#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/Condition.h>

#include "AvStream.h"

namespace Omm {
namespace Dvb {

class Transponder;
class Stream;
class PatSection;
class TransportStreamPacket;
class ByteQueueIStream;

class Service
{
    friend class Transponder;
    friend class Frontend;
    friend class Demux;
    friend class Remux;

public:
    static const std::string TypeDigitalTelevision;
    static const std::string TypeDigitalRadioSound;
    static const std::string TypeTeletext;
    static const std::string TypeNvodReference;
    static const std::string TypeNodTimeShifted;
    static const std::string TypeMosaic;
    static const std::string TypeFmRadio;
    static const std::string TypeDvbSrm;
    static const std::string TypeAdvancedCodecDigitalRadioSound;
    static const std::string TypeAdvancedCodecMosaic;
    static const std::string TypeDataBroadcastService;
    static const std::string TypeRcsMap;
    static const std::string TypeRcsFls;
    static const std::string TypeDvbMhp;
    static const std::string TypeMpeg2HdDigitalTelevision;
    static const std::string TypeAdvancedCodecSdDigitalTelevision;
    static const std::string TypeAdvancedCodecSdNvodTimeShifted;
    static const std::string TypeAdvancedCodecSdNvodReference;
    static const std::string TypeAdvancedCodecHdDigitalTelevision;
    static const std::string TypeAdvancedCodecHdNvodTimeShifted;
    static const std::string TypeAdvancedCodecHdNvodReference;
    static const std::string TypeAdvancedCodecFrameCompatiblePlanoStereoscopicHdTelevision;
    static const std::string TypeAdvancedCodecFrameCompatiblePlanoStereoscopicTimeShifted;
    static const std::string TypeAdvancedCodecFrameCompatiblePlanoStereoscopicReference;

    static const unsigned int InvalidPcrPid;

    static const std::string StatusUndefined;
    static const std::string StatusNotRunning;
    static const std::string StatusStartsShortly;
    static const std::string StatusPausing;
    static const std::string StatusRunning;
    static const std::string StatusOffAir;

    Service(Transponder* pTransponder, const std::string& name, unsigned int sid, unsigned int pmtid);
    Service(const Service& service);
    ~Service();

    void addStream(Stream* pStream);
    void readXml(Poco::XML::Node* pXmlService);
    void writeXml(Poco::XML::Element* pTransponder);

    std::string getType();
    static std::string typeToString(Poco::UInt8 status);
    static std::string statusToString(Poco::UInt8 status);

    bool isAudio();
    bool isSdVideo();
    bool isHdVideo();
    std::string getName();
    std::string getStatus();
    bool getScrambled();
    Transponder* getTransponder();
    Stream* getFirstAudioStream();
    Stream* getFirstVideoStream();
    bool hasPacketIdentifier(Poco::UInt16 pid);

    std::istream* getStream();
    AvStream::ByteQueue* getByteQueue();
    void stopStream();
    void flush();
    void queueTsPacket(TransportStreamPacket* pPacket);
    void startQueueThread();
    void stopQueueThread();
    void waitForStopQueueThread();

private:
    void queueThread();
    bool queueThreadRunning();

    bool                                _clone;
    Transponder*                        _pTransponder;
    std::string                         _type;
    std::string                         _providerName;
    std::string                         _name;
    unsigned int                        _sid;
    unsigned int                        _pmtPid;
    unsigned int                        _pcrPid;
    std::string                         _status;
    bool                                _scrambled;

    std::vector<Stream*>                _streams;
    // set of service pids makes calls to hastPacketIdentifier() more efficient
    std::set<Poco::UInt16>              _pids;

    AvStream::ByteQueue                 _byteQueue;
    ByteQueueIStream*                   _pIStream;
    PatSection*                         _pPat;
    TransportStreamPacket*              _pPatTsPacket;
    std::queue<TransportStreamPacket*>  _packetQueue;
    const int                           _packetQueueTimeout;
    const int                           _packetQueueSize;
    Poco::Thread*                       _pQueueThread;
    Poco::RunnableAdapter<Service>      _queueThreadRunnable;
    bool                                _queueThreadRunning;
    Poco::Condition                     _queueReadCondition;
    Poco::FastMutex                     _serviceLock;
};

}  // namespace Omm
}  // namespace Dvb

#endif
