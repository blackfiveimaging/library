#include "debug.h"

#include <cstdlib>

using namespace std;

int main(int argc,char **argv)
{
	DebugLevel level=WARN;
	if(argc>1)
		level=DebugLevel(atoi(argv[1]));

	Debug.SetLevel(level);

	Debug[ERROR] << "Debugging..." << endl;
	Debug[WARN] << "Warning..." << endl;
	Debug[COMMENT] << "Commenting..." << endl;
	Debug[TRACE] << "Tracing..." << endl;

	Debug.PushLevel(TRACE);

	Debug[ERROR] << "Debugging..." << endl;
	Debug[WARN] << "Warning..." << endl;
	Debug[COMMENT] << "Commenting..." << endl;
	Debug[TRACE] << "Tracing..." << endl;

	Debug.PopLevel();

	Debug[ERROR] << "Debugging..." << endl;
	Debug[WARN] << "Warning..." << endl;
	Debug[COMMENT] << "Commenting..." << endl;
	Debug[TRACE] << "Tracing..." << endl;


	if(argc>2)
	{
		Debug.SetLogFile(argv[2]);
		Debug[ERROR] << "Debugging..." << endl;
		Debug[WARN] << "Warning..." << endl;
		Debug[COMMENT] << "Commenting..." << endl;
		Debug[TRACE] << "Tracing..." << endl;
	}
}

