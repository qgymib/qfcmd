#if !defined(WIN32_HPP) && defined(_WIN32)
#define WIN32_HPP

#include <QString>
#include <Windows.h>

namespace qfcmd
{

class wchar
{
public:
	wchar(const wchar_t* str);
	virtual ~wchar();

public:
	const wchar_t* data() const;

private:
	wchar_t* m_str;
};

class Win32
{
public:
	static wchar StringToWide(const QString& str);
};

};

#endif // WIN32_HPP
