#ifndef QFCMD_UTILS_SERIALIZATION_HPP
#define QFCMD_UTILS_SERIALIZATION_HPP

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace qfcmd {

QByteArray Serialize(const QJsonObject& obj);

} /* namespace qfcmd */

#endif
