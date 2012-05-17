#include "Resource.h"
#include "Types.h"

#include <cstdio>


res_ref::res_ref()
{
	name[0] = '\0';
}


res_ref::res_ref(const char *string)
{
	int32 len = string ? strlen(string) : 0;
	len = std::min(len, 8);
	memcpy(name, string, len);
	if (len < 8)
		name[len] = '\0';
}


res_ref::res_ref(const res_ref &ref)
{
	memcpy(name, ref.name, 8);
}


res_ref::operator const char*() const
{
	static char str[9];
	str[8] = '\0';
	memcpy(str, name, 8);
	return (const char *)str;
};

res_ref&
res_ref::operator=(const res_ref& other)
{
	memcpy(name, other.name, 8);
	return *this;
}


/*
char&
res_ref::operator [](int index)
{
	return name[index];
};
*/

bool
operator<(const res_ref &ref1, const res_ref &ref2)
{
	return strncasecmp(ref1.name, ref2.name, 8) < 0;
}


bool
operator==(const res_ref &ref1, const res_ref &ref2)
{
	return strncasecmp(ref1.name, ref2.name, 8) == 0;
}


bool
operator<(const ref_type &ref1, const ref_type &ref2)
{
	int nameDiff = strncasecmp(ref1.name.name, ref2.name.name, 8);
	if (nameDiff < 0)
		return true;

	if (nameDiff > 0)
		return false;

	return ref1.type < ref2.type;
}


std::ostream &
operator<<(std::ostream &os, res_ref ref)
{
	for (int32 i = 0; i < 8; i++) {
		if (ref.name[i] == '\0')
			break;
		std::cout << ref.name[i];
	}
	return os;
};

