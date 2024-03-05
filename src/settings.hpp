#ifndef QFCMD_SETTINGS_HPP
#define QFCMD_SETTINGS_HPP

#include <QVariant>
#include <QKeySequence>

/**
 * @brief Settings table.
 *
 * The table contains following fields:
 * + The enumeration of the setting.
 * + The name of the setting.
 * + The default value of the setting.
 */
#define QFCMD_SETTINGS_TABLE(xx)  \
    xx(CLOSE_TAB,       "Shortcuts/CloseTab",       QKeySequence(Qt::CTRL | Qt::Key_W))  \
    xx(DUPLICATE_TAB,   "Shortcuts/DuplicateTab",   QKeySequence(Qt::CTRL | Qt::Key_T))

namespace qfcmd {

struct SettingsInner;

class Settings
{
public:
    enum SettingItem
    {
#define QFCMD_EXPAND_SETTINGS_TABLE_AS_ENUM(e, ...)    e,
        QFCMD_SETTINGS_TABLE(QFCMD_EXPAND_SETTINGS_TABLE_AS_ENUM)
#undef QFCMD_EXPAND_SETTINGS_TABLE_AS_ENUM
    };

public:
    virtual ~Settings();

public:
    /**
     * @brief Initialize the settings mansger.
     */
    static void init();

    /**
     * @breif Exit the settings manager.
     */
    static void exit();

public:
    /**
     * @breif Get the value of the setting.
     * @param[in] item  The enumeration of the setting.
     */
    template<typename T>
    static T get(const SettingItem item)
    {
        const QVariant val = getConfig(item);
        return val.value<T>();
    }

    /**
     * @brief Set the value of the setting.
     * @param[in] item  The enumeration of the setting.
     * @param[in] val   The value of the setting.
     */
    static void set(const SettingItem item, const QVariant val);

private:
    explicit Settings();

private:
    static QVariant getConfig(const SettingItem item);

private:
    SettingsInner* m_inner;
};

} /* namespace qfcmd */

#endif // SETTINGS_HPP
