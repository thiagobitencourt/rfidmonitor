#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

#include "synchronizationpacket.h"

DataSummary SynchronizationPacket::dataContent() const
{
    return m_dataSummary;
}

void SynchronizationPacket::setDataContent(const DataSummary &dataContent)
{
    m_dataSummary = dataContent;
}

void SynchronizationPacket::read(const QJsonObject &json)
{
    m_id = json["id"].toInt();
    m_name = json["name"].toString();
    m_macAddress = json["macaddress"].toString();
    m_ipAddress = json["ipaddress"].toString();
    QJsonObject dataSummaryObj = json["datasummary"].toObject();
    m_dataSummary.read(dataSummaryObj);
}

void SynchronizationPacket::write(QJsonObject &json) const
{
    json["id"] = m_id;
    json["name"] = m_name;
    json["macaddress"] = m_macAddress;
    json["ipaddress"] = m_ipAddress;
    m_dataSummary.write(json);
}

QString SynchronizationPacket::ipAddress() const
{
    return m_ipAddress;
}

void SynchronizationPacket::setIpAddress(const QString &ipAddress)
{
    m_ipAddress = ipAddress;
}

QString SynchronizationPacket::macAddress() const
{
    return m_macAddress;
}

void SynchronizationPacket::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

QString SynchronizationPacket::name() const
{
    return m_name;
}

void SynchronizationPacket::setName(const QString &name)
{
    m_name = name;
}

int SynchronizationPacket::id() const
{
    return m_id;
}

void SynchronizationPacket::setId(int id)
{
    m_id = id;
}


int Data::id() const
{
    return m_id;
}

void Data::setId(int id)
{
    m_id = id;
}
int Data::idcollectorPoint() const
{
    return m_idcollectorPoint;
}

void Data::setIdcollectorPoint(int idcollectorPoint)
{
    m_idcollectorPoint = idcollectorPoint;
}
int Data::idantena() const
{
    return m_idantena;
}

void Data::setIdantena(int idantena)
{
    m_idantena = idantena;
}
QDateTime Data::dateTime() const
{
    return m_dateTime;
}

void Data::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

void Data::read(const QJsonObject &json)
{
    m_id = json["id"].toInt();\
    m_idcollectorPoint = json["idcollectorpoint"].toInt();
    m_idantena = json["idantena"].toInt();
    m_applicationCode = json["applicationcode"].toInt();
    m_identificationCode = json["identificationcode"].toInt();
    m_dateTime = QDateTime::fromString(json["datetime"].toString(), Qt::ISODate);
}

void Data::write(QJsonObject &json) const
{
    json["id"] = m_id;\
    json["idcollectorpoint"] = m_idcollectorPoint;
    json["idantena"] = m_idantena;
    json["applicationcode"] = m_applicationCode;
    json["identificationcode"] = m_identificationCode;
    json["datetime"] = m_dateTime.toString(Qt::ISODate);
}

int Data::applicationCode() const
{
    return m_applicationCode;
}

void Data::setApplicationCode(int applicationCode)
{
    m_applicationCode = applicationCode;
}

int Data::identificationCode() const
{
    return m_identificationCode;
}

void Data::setIdentificationCode(int identificationCode)
{
    m_identificationCode = identificationCode;
}





int DataSummary::idEnd() const
{
    return m_idEnd;
}

void DataSummary::setIdEnd(int idEnd)
{
    m_idEnd = idEnd;
}

QString DataSummary::md5diggest() const
{
    return m_md5diggest;
}

void DataSummary::setMd5diggest(const QString &md5diggest)
{
    m_md5diggest = md5diggest;
}

QList<Data> DataSummary::data() const
{
    return m_data;
}

void DataSummary::setData(const QList<Data> &data)
{
    m_data = data;
}

void DataSummary::read(const QJsonObject &json)
{
    m_idBegin = json["idbegin"].toInt();
    m_idEnd = json["idend"].toInt();
    m_md5diggest = json["md5diggest"].toString();
    QJsonArray dataArray = json["data"].toArray();
    for(int i = 0; i < dataArray.size(); i++){
        QJsonObject obj = dataArray[i].toObject();
        Data data;
        data.read(obj);
        m_data.append(data);
    }
}

void DataSummary::write(QJsonObject &json) const
{
    json["idbegin"] = m_idBegin;
    json["idend"] = m_idEnd;
    json["md5diggest"] = m_md5diggest;
    QJsonArray dataArray;
    foreach (const Data data, m_data) {
        QJsonObject obj;
        data.write(obj);
        dataArray.append(obj);
    }
    json["data"] = dataArray;
}
int DataSummary::idBegin() const
{
    return m_idBegin;
}

void DataSummary::setIdBegin(int idBegin)
{
    m_idBegin = idBegin;
}
