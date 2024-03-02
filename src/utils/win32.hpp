#if !defined(QFCMD_WIN32_HPP) && defined(_WIN32)
#define QFCMD_WIN32_HPP

#include <QString>
#include <Windows.h>

namespace qfcmd {

class wchar
{
public:
    wchar(const wchar& orig);
    virtual ~wchar();

public:
    /**
     * @brief Convert QString to wchar_t.
     * @param[in] str   The QString.
     * @return          The wchar_t string.
     */
    static wchar fromQString(const QString& str);

public:
    /**
     * @brief Get the wchar_t string.
     * @return The wchar_t string.
     */
    const wchar_t* data() const;

    /**
     * @brief Convert wchar_t to QString.
     * @return The QString.
     */
    QString toQString() const;

private:
    /* Default constructor is not allowed. */
    wchar();

private:
    wchar_t*    m_str;
    int         m_size;
};

} /* namespace qfcmd */

#endif // QFCMD_WIN32_HPP
