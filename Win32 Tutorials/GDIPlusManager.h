#pragma once

#include "RedSkyWin.h"

class GDIPlusManager
{
public:
	GDIPlusManager();
	~GDIPlusManager();

private:
	static ULONG_PTR token;

	//counts the amount of references to the GDIPlusManager there are, this is incremented in the constructor
	//and decreased in the destructor
	static int refCount;
};

