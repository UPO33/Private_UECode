
#include <sstream>

typedef std::wostringstream StringStreamOut;
typedef std::wistringstream StringStreamIn;


inline void SPrintAuto(StringStreamOut& out, const TCHAR* format)
{
	while (*format)
	{
		if (*format == TEXT('%'))
		{
			check(false);
		}
		out << *format++;
	}
}
template<typename T, typename... TArgs> void SPrintAuto(StringStreamOut& out, const TCHAR* format, const T& value, const TArgs&... args)
{
	while (*format)
	{
		if (*format == TEXT('%'))
		{
			out << value;
			format++;
			SPrintAuto(out, format, args...);
			return;
		}
		out << *format++;
	}
}


StringStreamOut& operator << (StringStreamOut& stream, const FVector& v)
{
	stream << std::wstring(L"{") << v.X << std::wstring(L", ") << v.Y << std::wstring(L", ") << v.Z << std::wstring(L" }");
	return stream;
}
StringStreamOut& operator << (StringStreamOut& stream, const FName name)
{
	stream << std::wstring(*name.ToString());
	return stream;
}
StringStreamOut& operator << (StringStreamOut& stream, const FString& str)
{
	stream << std::wstring(*str);
	return stream;
}

/*
	smart && simpler logging
	example:
	FHitResult hit;
	UE_LOG_SMART(LogTemp, Warning, TEXT("bone: % point: % faceIndex: %"), hit.BoneName, hit.ImpactPoint, hit.FaceIndex);
*/
#define UE_LOG_SMART(Category, Verbosity, Format, ...) \
		{ \
			StringStreamOut strLog; \
			SPrintAuto(strLog, Format, __VA_ARGS__); \
			UE_LOG(Category, Verbosity, TEXT("%s"), strLog.str().c_str()); \
		} \

