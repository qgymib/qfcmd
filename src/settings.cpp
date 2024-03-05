#include <QSettings>
#include <QApplication>
#include "settings.hpp"

namespace qfcmd {
struct SettingsInner
{
    SettingsInner();
    ~SettingsInner();
};
} /* namespace qfcmd */

static qfcmd::Settings* s_config = nullptr;

static QString _config_get_name(qfcmd::Settings::SettingItem item)
{
    switch(item)
    {
#define QFCMD_EXPAND_SETTINGS_TABLE_AS_NAME(E, N, ...)   \
        case qfcmd::Settings::E: return N;
        QFCMD_SETTINGS_TABLE(QFCMD_EXPAND_SETTINGS_TABLE_AS_NAME);
#undef QFCMD_EXPAND_SETTINGS_TABLE_AS_NAME

    default:
        abort();
    }
}

static QVariant _get_default_config(qfcmd::Settings::SettingItem item)
{
    switch(item)
    {
#define QFCMD_EXPAND_SETTINGS_TABLE_AS_DEFVAL(E, N, V)  \
        case qfcmd::Settings::E: return V;
        QFCMD_SETTINGS_TABLE(QFCMD_EXPAND_SETTINGS_TABLE_AS_DEFVAL);
#undef QFCMD_EXPAND_SETTINGS_TABLE_AS_DEFVAL

    default:
        abort();
    }
}

qfcmd::SettingsInner::SettingsInner()
{
}

qfcmd::SettingsInner::~SettingsInner()
{
}

qfcmd::Settings::Settings()
    : m_inner(new qfcmd::SettingsInner)
{
}

qfcmd::Settings::~Settings()
{
    delete m_inner;
}

void qfcmd::Settings::init()
{
    if (s_config != nullptr)
    {
        return;
    }

    s_config = new qfcmd::Settings;

    /* Set config file format and location. */
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());
}

void qfcmd::Settings::exit()
{
    if (s_config == nullptr)
    {
        return;
    }

    delete s_config;
    s_config = nullptr;
}

void qfcmd::Settings::set(const SettingItem item, const QVariant val)
{
    QSettings config;
    QString name = _config_get_name(item);
    config.setValue(name, val);
}

QVariant qfcmd::Settings::getConfig(const SettingItem item)
{
    QSettings config;
    QString name = _config_get_name(item);
    QVariant defval = _get_default_config(item);
    return config.value(name, defval);
}
