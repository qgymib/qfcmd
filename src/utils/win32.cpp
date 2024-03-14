#include <QByteArray>

#include "win32.hpp"

#if defined(QFCMD_WIN32_HPP)

qfcmd::wchar::wchar(const wchar_t* str)
{
    m_str = _wcsdup(str);
    m_size = wcslen(m_str);
}

qfcmd::wchar::wchar(const QString& str)
{
#if 0 /* This is the standard way, but has two steps conversion. */

    QByteArray arr = str.toUtf8();
    const char* c_str = arr.data();

    m_size = ::MultiByteToWideChar(CP_UTF8, 0, c_str, -1, NULL, 0);
    size_t buf_sz = m_size * sizeof(WCHAR);

    m_str = (WCHAR*)::malloc(buf_sz);
    int ret = ::MultiByteToWideChar(CP_UTF8, 0, c_str, -1, m_str, m_size);
    assert(ret == m_size);

#else /* We take the fast way. */

    const ushort* buf = str.utf16();
    qsizetype bufsz = str.size();

    /* Check type size to ensure this approach works. */
    static_assert(sizeof(*buf) == sizeof(*m_str),
                  "sizeof(ushort) != sizeof(wchar_t)");

    m_size = bufsz + 1;
    size_t buf_sz = m_size * sizeof(WCHAR);

    m_str = (wchar_t*)::malloc(buf_sz);
    memcpy(m_str, buf, buf_sz);
#endif
}

qfcmd::wchar::wchar(const wchar& orig)
{
    size_t buf_sz = orig.m_size * sizeof(WCHAR);
    m_size = orig.m_size;
    m_str = (WCHAR*)::malloc(buf_sz);
    memcpy(m_str, orig.m_str, buf_sz);
}

qfcmd::wchar::~wchar()
{
    if (m_str != NULL)
    {
        ::free(m_str);
        m_str = NULL;
    }
    m_size = 0;
}

const wchar_t* qfcmd::wchar::data() const
{
    return m_str;
}

QString qfcmd::wchar::toQString() const
{

#if 0 /* This is the standard way, but has two steps conversion. */

    int utf8_sz = WideCharToMultiByte(CP_UTF8, 0, m_str, -1, NULL, 0, NULL, NULL);
    QByteArray arr(utf8_sz + 1, Qt::Uninitialized);
    char* utf8 = arr.data();

    int ret = WideCharToMultiByte(CP_UTF8, 0, m_str, -1, utf8, utf8_sz, NULL, NULL);
    assert(ret == utf8_sz);
    utf8[utf8_sz] = '\0';

    return QString::fromUtf8(arr);

#else /* We take the fast way. */

    static_assert(sizeof(char16_t) == sizeof(wchar_t));

    return QString::fromUtf16((const char16_t*)m_str, m_size);

#endif
}

#endif /* defined(QFCMD_WIN32_HPP) */
