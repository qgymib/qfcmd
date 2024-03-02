#include <QByteArray>

#include "win32.hpp"

#if defined(_WIN32)

qfcmd::wchar::wchar(const wchar_t* str)
{
	m_str = _wcsdup(str);
}

qfcmd::wchar::~wchar()
{
	free(m_str);
}

const wchar_t* qfcmd::wchar::data() const
{
	return m_str;
}

qfcmd::wchar qfcmd::Win32::StringToWide(const QString& str)
{
	QByteArray arr = str.toUtf8();
	const char* c_str = arr.data();

	int multi_byte_sz = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, NULL, 0);
	size_t buf_sz = multi_byte_sz * sizeof(WCHAR);

	WCHAR* buf = (WCHAR*)malloc(buf_sz);
	int ret = MultiByteToWideChar(CP_UTF8, 0, c_str, -1, (WCHAR*)buf, multi_byte_sz);
	assert(ret == multi_byte_sz);

	qfcmd::wchar w_ret(buf);
	free(buf);

	return w_ret;
}

#endif
