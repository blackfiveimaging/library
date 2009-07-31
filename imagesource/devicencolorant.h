#ifndef DEVICENCOLORANT_H
#define DEVICENCOLORANT_H

class DeviceNColorant;
class DeviceNColorantList
{
	public:
	DeviceNColorantList();
	virtual ~DeviceNColorantList();
	virtual int GetColorantCount();
	virtual int GetColorantIndex(const char *name);
	virtual char *GetEnabledColorants();
	virtual void SetEnabledColorants(const char *col);
	virtual DeviceNColorant *FirstColorant();
	virtual DeviceNColorant *operator[](int idx);
	protected:
	DeviceNColorant *first;
	friend class DeviceNColorant;
};


class DeviceNColorant
{
	public:
	// If you provide a name but no RGB values the colorant will be filled in if the name is
	// recognised, and a const char * exception will be thrown if not.
	DeviceNColorant(DeviceNColorantList &header,const char *name,const char *displayname=NULL);
	DeviceNColorant(DeviceNColorantList &header,const char *name,const char *displayname,int red,int green,int blue);
	virtual ~DeviceNColorant();
	const char *GetDisplayName();
	void Enable();
	void Disable();
	bool GetEnabled();
	virtual DeviceNColorant *NextColorant();
	virtual DeviceNColorant *PrevColorant();
	virtual const char *GetName();
	int red,green,blue;
	protected:
	DeviceNColorantList &header;
	bool enabled;
	char *name;
	char *displayname;
	DeviceNColorant *next,*prev;
};

#endif

