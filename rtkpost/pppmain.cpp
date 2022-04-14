#include "rtklib.h"
#include <cstring>
#include <iostream>
#include "pppmain.h"



// VS debug for langlinfei 2021--2-13
extern void settspan(gtime_t ts, gtime_t te) {}
extern void settime(gtime_t time) {}

extern int showmsg(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
	fprintf(stderr, "\r");
	return 0;
}


#if OPTCONFIG

void LoadOpt(prcopt_t* prcopt, solopt_t* solopt, filopt_t* filopt, char* file)
{
	char buff[1024] = "", * p, id[32];
	int sat;
	resetsysopts();
	if (!loadopts(file, sysopts)) return;
	getsysopts(prcopt, solopt, filopt);
}


void SaveOpt(prcopt_t* prcopt, solopt_t* solopt, filopt_t* filopt, char* filecfg)
{
	char comment[256], s[64];
	time2str(utc2gpst(timeget()), s, 0);
	sprintf(comment, "rtkpost options (%s, v.%s %s)", s, VER_RTKLIB, PATCH_LEVEL);
	setsysopts(prcopt, solopt, filopt);
	if (!saveopts(filecfg, "w", comment, sysopts)) return;
}

#endif // OPTCONFIG

int main(int argc,char* argv[]) {
	prcopt_t prcopt = prcopt_default;
	solopt_t solopt = solopt_default;
	filopt_t filopt = { "" };
	gtime_t ts = { 0 }, te = { 0 };
	double ti = 0.0, tu = 0.0;
	int i, n = 0, stat;
	char  * infile[6];


#if 1   /*RTK*/

	char infile_[6][1024] = { 
		"D:\\Projects\\rtklib\\PPP\\rtkdata\\move20210114.21o",
		"D:\\Projects\\rtklib\\PPP\\rtkdata\\base20210114.21o",
		"D:\\Projects\\rtklib\\PPP\\rtkdata\\base20210114.21p"
	};
	char* outfile = (char*)"D:\\Projects\\rtklib\\PPP\\rtkdata\\move20210114.pos";
	char* cfgfile = (char*)"D:\\Projects\\rtklib\\PPP\\rtkdata\\rtkpost.ini";	
	prcopt.refpos = 3;
	n = 3;

#else	/*PPP*/
	char infile_[6][1024] = {
	"D:\\Projects\\rtklib\\PPP\\data\\bjfs1380.21o",
	"D:\\Projects\\rtklib\\PPP\\data\\brdc138.21p",
	"D:\\Projects\\rtklib\\PPP\\data\\gfz21582.clk",
	"D:\\Projects\\rtklib\\PPP\\data\\gfz21582.sp3"
	};
	n = 4;
	char* outfile = (char*)"D:\\Projects\\rtklib\\PPP\\data\\move20210114.pos";
	char* cfgfile = (char*)"D:\\Projects\\rtklib\\PPP\\rtkpost\\rtkpost.ini"; 

#endif // PPP

	// load config 
	LoadOpt(&prcopt, &solopt, &filopt, cfgfile);

	// Output solution stat
	solopt.trace = 5;
	
	// set input/output files
	for (i = 0; i < 6; i++) infile[i] = infile_[i];

	showmsg((char*)"reading...");

	// post processing positioning
	if ((stat = postpos(ts, te, ti, tu, &prcopt, &solopt, &filopt, infile, n, outfile,
		NULL, NULL)) == 1) {
		showmsg((char*)"aborted");
	}

	// save optera
	//SaveOpt(&prcopt, &solopt, &filopt, cfgfile);
	return stat;
}


