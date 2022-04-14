#include "rtklib.h"
#include <cstring>
#include <iostream>

//ƒ¨»œ≈‰÷√
prcopt_t PrcOpt = prcopt_default;
solopt_t SolOpt = solopt_default;

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

int main(int argc,char** argv)
{


    char s[100];
    solopt_t solopt[2];
    double pos[3], nmeapos[3];
    int itype[] = {
        STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP
    };
    int otype[] = {
        STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPCAS,STR_FILE
    };
    int i, strs[MAXSTRRTK] = { 0 }, sat, ex, stropt[8] = { 0 };
    char* paths[8], * cmds[3] = { 0 }, * cmds_periodic[3] = { 0 }, * rcvopts[3] = { 0 };
    char buff[1024], * p;
    char file[1024], * type, errmsg[20148];
    FILE* fp;
    gtime_t time = timeget();
    pcvs_t pcvr = { 0 }, pcvs = { 0 };
    pcv_t* pcv, pcv0 = { 0 };

    trace(3, "SvrStart\n");


    traceopen("trace.log");
    tracelevel(3);

    
    for (i = 0; i < MAXSAT; i++) {
        PrcOpt.exsats[i] = 0;
    }

    if (!readpcv("./data/igs14_2148.atx", &pcvr)) {
        return 0;
    }

    PrcOpt.pcvr[0] = PrcOpt.pcvr[1] = pcv0; // initialize antenna PCV


    if (PrcOpt.sateph == EPHOPT_PREC || PrcOpt.sateph == EPHOPT_SSRCOM) {
        if (!readpcv("./data/igs14_2148.atx", &pcvs)) {
            return 0;
        }
        for (i = 0; i < MAXSAT; i++) {
            if (!(pcv = searchpcv(i + 1, "", time, &pcvs))) continue;
            rtksvr.nav.pcvs[i] = *pcv;
        }
        free(pcvs.pcv);
    }

    else {
        PrcOpt.baseline[0] = 0.0;
        PrcOpt.baseline[1] = 0.0;
    }
    for (i = 0; i < 3; i++) strs[i] = StreamC[i] ? itype[Stream[i]] : STR_NONE;
    for (i = 3; i < 5; i++) strs[i] = StreamC[i] ? otype[Stream[i]] : STR_NONE;
    for (i = 5; i < 8; i++) strs[i] = StreamC[i] ? otype[Stream[i]] : STR_NONE;
    for (i = 0; i < 8; i++) {
        if (strs[i] == STR_NONE) paths[i] = (char*)"";
        else if (strs[i] == STR_SERIAL) paths[i] = Paths[i][0].c_str();
        else if (strs[i] == STR_FILE) paths[i] = Paths[i][2].c_str();
        else if (strs[i] == STR_FTP || strs[i] == STR_HTTP) paths[i] = Paths[i][3].c_str();
        else paths[i] = Paths[i][1].c_str();
    }
    for (i = 0; i < 3; i++) {
        if (strs[i] == STR_SERIAL) {
            if (CmdEna[i][0]) cmds[i] = Cmds[i][0].c_str();
            if (CmdEna[i][2]) cmds_periodic[i] = Cmds[i][2].c_str();
        }
        else if (strs[i] == STR_TCPCLI || strs[i] == STR_TCPSVR ||
            strs[i] == STR_NTRIPCLI) {
            if (CmdEnaTcp[i][0]) cmds[i] = CmdsTcp[i][0].c_str();
            if (CmdEnaTcp[i][2]) cmds_periodic[i] = CmdsTcp[i][2].c_str();
        }
        rcvopts[i] = RcvOpt[i].c_str();
    }
    NmeaCycle = NmeaCycle < 1000 ? 1000 : NmeaCycle;
    pos[0] = NmeaPos[0] * D2R;
    pos[1] = NmeaPos[1] * D2R;
    pos[2] = NmeaPos[2];
    pos2ecef(pos, nmeapos);

    strsetdir(LocalDirectory.c_str());
    strsetproxy(ProxyAddr.c_str());

    for (i = 3; i < 8; i++) {
        if (strs[i] == STR_FILE && !ConfOverwrite(paths[i])) return;
    }
    if (DebugStatusF > 0) {
        rtkopenstat(STATFILE, DebugStatusF);
    }
    if (SolOpt.geoid > 0 && GeoidDataFileF != "") {
        opengeoid(SolOpt.geoid, GeoidDataFileF.c_str());
    }
    if (DCBFileF != "") {
        readdcb(DCBFileF.c_str(), &rtksvr.nav, NULL);
    }
    for (i = 0; i < 2; i++) {
        solopt[i] = SolOpt;
        solopt[i].posf = Format[i + 3];
    }
    stropt[0] = TimeoutTime;
    stropt[1] = ReconTime;
    stropt[2] = 1000;
    stropt[3] = SvrBuffSize;
    stropt[4] = FileSwapMargin;
    strsetopt(stropt);
    strcpy(rtksvr.cmd_reset, ResetCmd.c_str());
    rtksvr.bl_reset = MaxBL;

    // start rtk server
    if (!rtksvrstart(&rtksvr, SvrCycle, SvrBuffSize, strs, paths, Format, NavSelect,
        cmds, cmds_periodic, rcvopts, NmeaCycle, NmeaReq, nmeapos,
        &PrcOpt, solopt, &monistr, errmsg)) {
        trace(2, "rtksvrstart error %s\n", errmsg);
        traceclose();
        return 0;
    }
}


