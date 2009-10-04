#ifndef MD5_H
#define MD5_H

#include <ostream>


class MD5Context
{
	public:
	MD5Context()
	{
		Init();
	}
	~MD5Context()
	{
	}
	void Init();
	void Update(unsigned char const *buf,unsigned int len);
	void Finalize(unsigned char digest[16]);
	protected:
	unsigned int buf[4];
	int bits[2];
	unsigned char in[64];
	bool finalized;
};


class MD5Digest
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
	MD5Context context;
	friend std::ostream& operator<<(std::ostream &s,MD5Digest &c);
};

#endif

