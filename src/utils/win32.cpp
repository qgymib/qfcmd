#include <QByteArray>

#include "win32.hpp"

#if defined(QFCMD_WIN32_HPP)

qfcmd::wchar::wchar()
{
    m_str = NULL;
    m_size = 0;
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

qfcmd::wchar qfcmd::wchar::fromQString(const QString& str)
{
    qfcmd::wchar w_ret;

    QByteArray arr = str.toUtf8();
    const char* c_str = arr.data();

    w_ret.m_size = ::MultiByteToWideChar(CP_UTF8, 0, c_str, -1, NULL, 0);
    size_t buf_sz = w_ret.m_size * sizeof(WCHAR);

    w_ret.m_str = (WCHAR*)::malloc(buf_sz);
    int ret = ::MultiByteToWideChar(CP_UTF8, 0, c_str, -1, w_ret.m_str, w_ret.m_size);
    assert(ret == w_ret.m_size);

    return w_ret;
}

const wchar_t* qfcmd::wchar::data() const
{
    return m_str;
}

QString qfcmd::wchar::toQString() const
{
    int utf8_sz = WideCharToMultiByte(CP_UTF8, 0, m_str, -1, NULL, 0, NULL, NULL);
    QByteArray arr(utf8_sz + 1, Qt::Uninitialized);
    char* utf8 = arr.data();

    int ret = WideCharToMultiByte(CP_UTF8, 0, m_str, -1, utf8, utf8_sz, NULL, NULL);
    assert(ret == utf8_sz);
    utf8[utf8_sz] = '\0';

    return QString::fromUtf8(arr);
}

#endif /* defined(QFCMD_WIN32_HPP) */
