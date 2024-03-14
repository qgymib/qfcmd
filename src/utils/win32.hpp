#if !defined(QFCMD_WIN32_HPP) && defined(_WIN32)
#define QFCMD_WIN32_HPP

#include <QString>
#include <Windows.h>

namespace qfcmd {

class wchar
{
public:
    wchar(const wchar_t* str);
    wchar(const QString& str);
    wchar(const wchar& orig);
    virtual ~wchar();

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
    wchar_t*    m_str;  /**< The wchar_t string. */
    int         m_size; /**< The size of wchar_t string, including the terminating null. */
};

} /* namespace qfcmd */

#endif // QFCMD_WIN32_HPP
