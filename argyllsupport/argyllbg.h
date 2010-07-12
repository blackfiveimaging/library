#ifndef ARGYLL_BLACKGENERATION_H
#define ARGYLL_BLACKGENERATION_H

#include <string>
#include <sstream>

enum Argyll_BlackGeneration {ARGYLLBG_ZERO,ARGYLLBG_HALF,ARGYLLBG_MAX,ARGYLLBG_RAMP,ARGYLLBG_CUSTOM,ARGYLLBG_TRANSFER,ARGYLLBG_RETAIN};

class Argyll_BlackGenerationCurve
{
	public:
	Argyll_BlackGenerationCurve(enum Argyll_BlackGeneration mode=ARGYLLBG_ZERO)
		: blackgenerationmode(mode), locusmode(true), startlevel(0.0), startpos(0.0), endpos(1.0), endlevel(1.0), curveshape(1.0)
	{
	}

	void SetLocusMode(bool mode)
	{
		locusmode=mode;		// Use -K instead of -k, to specify black locus rather than absolute black level.
	}

	bool GetLocusMode()
	{
		return(locusmode);
	}

	void SetMode(enum Argyll_BlackGeneration mode)
	{
		blackgenerationmode=mode;
	}

	enum Argyll_BlackGeneration GetMode()
	{
		return(blackgenerationmode);
	}

	void SetStartLevel(double stle)
	{
		startlevel=stle<0.01 ? 0 : stle;
	}

	double GetStartLevel()
	{
		return(startlevel);
	}

	void SetEndLevel(double enle)
	{
		endlevel=enle<0.01 ? 0 : enle;
	}

	double GetEndLevel()
	{
		return(endlevel);
	}

	void SetStartPos(double stpo)
	{
		startpos=stpo<0.01 ? 0 : stpo;
	}

	double GetStartPos()
	{
		return(startpos);
	}

	void SetEndPos(double enpo)
	{
		endpos=enpo<0.01 ? 0 : enpo;
	}

	double GetEndPos()
	{
		return(endpos);
	}

	void SetShape(double shape)
	{
		curveshape=shape<0.01 ? 0 : shape;
	}

	double GetShape()
	{
		return(curveshape);
	}

	double GetLevel(double in)
	{
		switch(blackgenerationmode)
		{
			case ARGYLLBG_TRANSFER:
			case ARGYLLBG_RETAIN:
			case ARGYLLBG_ZERO:
				return(0);
				break;
			case ARGYLLBG_HALF:
				return(0.5);
				break;
			case ARGYLLBG_MAX:
				return(1.0);
				break;
			case ARGYLLBG_RAMP:
				return(in);
				break;
			case ARGYLLBG_CUSTOM:
				if(in<startpos)
					return(startlevel);
				else if(in>endpos)
					return(endlevel);
				else
				{
					double width=endpos-startpos;
					if(width<0.0001)
						return(startlevel);

					double rv = (in - startpos)/width;
					double g = curveshape/2.0;

					rv = rv/((1.0/g - 2.0) * (1.0 - rv) + 1.0);
					rv = rv * (endlevel - startlevel) + startlevel;
					return(rv);
				}
				break;
			default:
				throw "Uknown Black Generation mode!";
				break;
		}
	}
	std::string GetCmdLine()
	{
		std::string result;
		result += locusmode ? "-K" : "-k";
		switch(blackgenerationmode)
		{
			case ARGYLLBG_TRANSFER:
				result+="t ";
				break;
			case ARGYLLBG_RETAIN:
				result+="e ";
				break;
			case ARGYLLBG_ZERO:
				result+="z ";
				break;
			case ARGYLLBG_HALF:
				result+="h ";
				break;
			case ARGYLLBG_MAX:
				result+="x ";
				break;
			case ARGYLLBG_RAMP:
				result+="r ";
				break;
			case ARGYLLBG_CUSTOM:
				{
					std::stringstream tmp;
					tmp << "p ";
					tmp << startlevel << " " << startpos << " " << endpos << " " << endlevel << " " << curveshape << " ";
					result+=tmp.str();
				}
				break;
		}
		return(result);
	}
	void SetCmdLine(std::string cmd)
	{
		std::stringstream tmp(cmd);
		std::string key;
		tmp >> key;

		locusmode=false;
		for(unsigned int i=0; i<key.length();++i)
		{
			if(key[i]=='K')
				locusmode=true;
		}

		char mode=key[key.length()-1];
		mode &=(~32);	// Force to upper case...
		if(mode=='K')
		{
			tmp >> key;
			mode=key[key.length()-1];
			mode &=(~32);	// Force to upper case...
		}
		switch(mode)
		{
			case 'T':
				blackgenerationmode=ARGYLLBG_TRANSFER;
				break;
			case 'E':
				blackgenerationmode=ARGYLLBG_RETAIN;
				break;
			case 'Z':
				blackgenerationmode=ARGYLLBG_ZERO;
				break;
			case 'H':
				blackgenerationmode=ARGYLLBG_HALF;
				break;
			case 'X':
				blackgenerationmode=ARGYLLBG_MAX;
				break;
			case 'R':
				blackgenerationmode=ARGYLLBG_RAMP;
				break;
			case 'P':
				blackgenerationmode=ARGYLLBG_CUSTOM;
				tmp >> startlevel >> startpos >> endpos >> endlevel >> curveshape;
				break;
		}
	}
	protected:
	enum Argyll_BlackGeneration blackgenerationmode;
	bool locusmode;		// Use -K rather than -k?
	// Parameters - range 0.0 - 1.0 except shape, which is 0.0+epsilon to 2.0-epsilon, with 1.0 being a linear ramp.
	double startlevel;	// K level from white to start of transition
	double startpos;	// start point of transition \ 0.0 = white
	double endpos;		// end point of transition   / 1.0 = black
	double endlevel;	// K level from end of transition to black
	double curveshape;	// 0.0-1.0 concave, 1.0-2.0 convex
};

#endif

