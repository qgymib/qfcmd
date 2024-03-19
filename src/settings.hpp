#ifndef QFCMD_SETTINGS_HPP
#define QFCMD_SETTINGS_HPP

#include <QVariant>
#include <QKeySequence>
#include <QStringList>

/**
 * @brief Settings table.
 *
 * The table contains following fields:
 * + The enumeration of the setting.
 * + The name of the setting.
 * + The default value of the setting.
 */
#define QFCMD_SETTINGS_TABLE(xx)  \
    xx(VIEW_SHOW_TOOLBAR,       "View/ShowToolBar",         true)                                   \
    xx(SHORTCUT_CLOSE_TAB,      "Shortcuts/CloseTab",       QKeySequence(Qt::CTRL | Qt::Key_W))     \
    xx(SHORTCUT_DUPLICATE_TAB,  "Shortcuts/DuplicateTab",   QKeySequence(Qt::CTRL | Qt::Key_T))     \
    xx(SHORTCUT_GO_BACK,        "Shortcuts/GoBack",         QKeySequence(Qt::ALT | Qt::Key_Left))   \
    xx(SHORTCUT_GO_FORWARD,     "Shortcuts/GoForward",      QKeySequence(Qt::ALT | Qt::Key_Right))  \
    xx(TABS_PANEL_0,            "Tabs/Panel_0",             QStringList())                          \
    xx(TABS_PANEL_1,            "Tabs/Panel_1",             QStringList())                          \
    xx(TABS_PANEL_0_ACTIVATE,   "Tabs/Panel_0_Activate",    0)                                      \
    xx(TABS_PANEL_1_ACTIVATE,   "Tabs/Panel_1_Activate",    0)

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

        MAX_SETTING_ITEM, /**< The number of setting items. */
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
    Settings(const Settings& orig) = delete;
    Settings();

private:
    static QVariant getConfig(const SettingItem item);

private:
    SettingsInner* m_inner;
};

} /* namespace qfcmd */

#endif // SETTINGS_HPP
