#include "serialization.hpp"

QByteArray qfcmd::Serialize(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson();
}
