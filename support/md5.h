#ifndef MD5_H
#define MD5_H

#include <ostream>


class MD5Context
{
	public:
	MD5Context()
	{
		InitMD5Context();
	}
	~MD5Context()
	{
	}
	void InitMD5Context();
	void UpdateMD5Context(unsigned char const *buf,unsigned int len);
	void FinalizeMD5Context(unsigned char digest[16]);
	private:
	unsigned int buf[4];
	int bits[2];
	unsigned char in[64];
	bool finalized;
};


class MD5Digest : public MD5Context
{
	public:
	MD5Digest();
	MD5Digest(const char *message,long length);
	MD5Digest(const MD5Digest &other);

	void Update(const char *message,long length);
	void Finalize();

	const unsigned char *GetDigest();
	const char *GetPrintableDigest();

	MD5Digest &operator=(const MD5Digest &other);
	bool operator==(const MD5Digest &other);
	bool operator!=(const MD5Digest &other);
	private:
	unsigned char digest[16];
	char digestprintable[33];
	friend std::ostream& operator<<(std::ostream &s,MD5Digest &c);
};

#endif

