/*======================================================================================================
 Name:           binary2tree_kashima.C
 Created by:     Akira Sato<sato@phys.sci.osaka-u.ac.jp>
 Modified by:    Shunichi Kashima <>
 Date:           Dec 10, 2024

 Purpose:        Example macro to convert a binary data file saved by DRSOsc to a root tree file.

 Notion:         mainly optimmized the calculation in time[numOfBoards][4][1024]. Extremely faster.
 */
/*======================================================================================================
 Name:           binary2tree_sato4.C
 Created by:     Akira Sato<sato@phys.sci.osaka-u.ac.jp>
 Modified by:    Shunichi Kashima <>
 Date:           Nov 20, 2024

 Purpose:        Example macro to convert a binary data file saved by DRSOsc to a root tree file.

 Notion:         When with DRS4 V4 board, time bin data acquired when timing calibration excuted are common among each channels. So the timing calibration section has the same data repeated the times of the number of connected channels.
                 DEBUG: treeDRS4BoardInfo has two duplicate serialNumber[numOfBoards] and timeBinWidth[numOfBoards][4][1024]. I removed extra loop.
 */
/*======================================================================================================
 Name:           binary2tree_sato3.C
 Created by:     Akira Sato<sato@phys.sci.osaka-u.ac.jp>
 Date:           Jan 31, 2019

 Purpose:        Example macro to convert a binary data file saved by DRSOsc to a root tree file.

How to use:

$ root
$ root[] .L binary2tree_sato3.C
$ root[] binary2tree_sato3(<binary data file name>)

ex) root[] binary2tree_sato3("../data/test001.dat")
======================================================================================================*/

/*
 Name:           read_binary.cpp
 Created by:     Stefan Ritt <stefan.ritt@psi.ch>
 Date:           July 30th, 2014

 Purpose:        Example file to read binary data saved by DRSOsc.

 Compile and run it with:

 gcc -o read_binary read_binary.cpp

 ./read_binary <filename>

 This program assumes that a pulse from a signal generator is split
 and fed into channels #1 and #2. It then calculates the time difference
 between these two pulses to show the performance of the DRS board
 for time measurements.

 $Id: read_binary.cpp 22321 2016-08-25 12:26:12Z ritt $
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define DEBUG 0
#define TIME_FLAG 1

#ifdef DEBUG
#define DEBUG_PRINT(level, fmt, ...) \
    if (DEBUG >= level)              \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(level, fmt, ...)
#endif

typedef struct
{
    char tag[3]; // "DRS"
    char version;
} FileHeader;

typedef struct
{
    char time_header[4]; // "TIME"
} TimeHeader;

typedef struct
{
    char bn[2]; // "B#" ?
    unsigned short board_serial_number;
} BoardHeader;

typedef struct
{
    char event_header[4]; // "EHDR"
    unsigned int event_serial_number;
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short millisecond;
    unsigned short range; //Range centre (RC) in mV
} EventHeader;

typedef struct
{
    char tc[2]; // "T#"
    unsigned short trigger_cell;
} TriggerCellHeader;

typedef struct
{
    char c[1];      // 'C'
    char chName[3]; // chName[2] is chn_inde; "000", "001", "002", "003"
} ChannelHeader;

/*-----------------------------------------------------------------------------*/
void PrintBoardHeader(BoardHeader *p)
{
    printf("Board header ================================\n");
    printf("\t%s\n", p->bn);
    printf("\tserial number: %d\n", p->board_serial_number);
}

/*-----------------------------------------------------------------------------*/
void PrintEventHeader(EventHeader *p)
{
    printf("Event header ================================\n");
    printf("\t%s\n", p->event_header);
    printf("\tserial number: %d\n", p->event_serial_number);
    printf("\ttime: %d-%d-%d, %d:%d:%d.%03d\n", p->year, p->month, p->day,
           p->hour, p->minute, p->second, p->millisecond);
    printf("\trange: %d\n", p->range);
}

/*-----------------------------------------------------------------------------*/
void PrintTriggerCellHeader(TriggerCellHeader *p)
{
    printf("Trigger cell header ================================\n");
    printf("\t%s\n", p->tc);
    printf("\ttrigger cell: %d\n", p->trigger_cell);
}

/*-----------------------------------------------------------------------------*/
void PrintChannelHeader(ChannelHeader *p)
{
    printf("Channelheader ================================\n");
    printf("\t%s\n", p->c);
    printf("\tchannel name: %s\n", p->chName);
}

/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
#include "TTree.h"
#include "TFile.h"
#include "TTimeStamp.h"
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
int binary2tree_kashima(const Char_t *binaryDataFile = "../data/test001.dat", const Double_t thr_V = 0.0, const Int_t debug_frag = 0)
{
    Int_t flag_b4exp_event_selection = 0;
    int flag_b4exp_trig = 0;
    if(thr_V != 0.0){
        flag_b4exp_event_selection = 1;
    }
    FileHeader fileHeader;
    TimeHeader timeHeader;
    BoardHeader boardHeader;
    EventHeader eventHeader;
    TriggerCellHeader triggerCellHeader;
    ChannelHeader channelHeader;

    unsigned int scaler_buf;
    unsigned short voltage[1024];
    double waveform_buf[16][4][1024], time_buf[16][4][1024];
    float bin_width[16][4][1024];
    // float bin_width[MAX_NUMBER_OF_BOARDS][NUMBER_OF_CH][NUMBER_OF_CELLS];
    int chID;
    double t1, t2, dt;
    char filename[256];

    int ndt;
    double threshold, sumdt, sumdt2;
    double voltage_buf;
    double cumulative_time_buf;

    // open the binary waveform file
    FILE *f = fopen(binaryDataFile, "rb");
    if (f == NULL)
    {
        printf("Cannot find file \'%s\'\n", binaryDataFile);
        return 0;
    }

    TFile f_root(Form("%s.root", binaryDataFile), "recreate");
    
    int serialNumber_buf[16]; //daisy-chain is allowed with up to 16 boards.


    //--------------------------------------------------
    // Read first headers
    //--------------------------------------------------
    // read file header
    fread(&fileHeader, sizeof(fileHeader), 1, f);
    if (fileHeader.tag[0] != 'D' || fileHeader.tag[1] != 'R' || fileHeader.tag[2] != 'S')
    {
        printf("Found invalid file header in file \'%s\', aborting.\n", filename);
        return 0;
    }
    if (fileHeader.version != '2')
    {
        printf("Found invalid file version \'%c\' in file \'%s\', should be \'2\', aborting.\n", fileHeader.version, filename);
        return 0;
    }
    DEBUG_PRINT(1, "File format Version: %c\n", fileHeader.version);
    // read time header
    fread(&timeHeader, sizeof(timeHeader), 1, f);
    if (memcmp(timeHeader.time_header, "TIME", 4) != 0)
    {
        printf("Invalid time header in file \'%s\', aborting.\n", filename);
        return 0;
    }

    //--------------------------------------------------
    // Get time calibration for each channels of each boards
    //--------------------------------------------------
    DEBUG_PRINT(1, "Getting time caliblation data %d ...\n", 1);
    int how_many_boards;
    for (how_many_boards = 0;; how_many_boards++)
    { // === Loop for boards
        // read board header
        fread(&boardHeader, sizeof(boardHeader), 1, f);
        if (memcmp(boardHeader.bn, "B#", 2) != 0)
        {
            // probably event header found
            fseek(f, -4, SEEK_CUR);
            break;
        }
        printf("\tBoard found : %d board(s)\n", how_many_boards);
        printf("\t\tBoard serial number: %d\n", boardHeader.board_serial_number);
        serialNumber_buf[how_many_boards] = boardHeader.board_serial_number; // Set Tree data
        // read time bin widths
        // memset(bin_width[iboard], sizeof(bin_width[0]), 0); // original
        memset(bin_width[how_many_boards], 0, sizeof(bin_width[0])); // Modified by A.Sato //bin_width[iBoard]の中身を全部ゼロで初期化
        for (int ich = 0; ich < 5; ich++)
        { // === Loop for channels
            fread(&channelHeader, sizeof(channelHeader), 1, f);
            if (channelHeader.c[0] != 'C')
            {
                // event header found
                fseek(f, -4, SEEK_CUR);
                break;
            }
            chID = channelHeader.chName[2] - '0' - 1; // = 0,1,2,3
            printf("\nTime Bin Width found\n");
            printf("\t\tBoard ID : %d (zero index) || ", how_many_boards);
            printf("Channel %d (ch1-4):\n", chID + 1);
            fread(&bin_width[how_many_boards][chID][0], sizeof(float), 1024, f);
            // fix for 2048 bin mode: double channel
            if (bin_width[how_many_boards][chID][1023] > 10 || bin_width[how_many_boards][chID][1023] < 0.01)
            {
                for (int icell = 0; icell < 512; icell++)
                    bin_width[how_many_boards][chID][icell + 512] = bin_width[how_many_boards][chID][icell];
            }
        }
    }

        if(0){
            for(int icell=0; icell<1024; icell++){
                printf("iCell : %d || ",icell);
                    for(int ib=0; ib<how_many_boards; ib++){
                        for(int ich=0; ich<4; ich++){
                            printf("%f ",bin_width[ib][ich][icell]);
                        }
                        printf(" || ");
                    }
                    printf("\n");
            }
        }
    Int_t numOfBoards = how_many_boards;

    //efficiently calculate time[numOfBoard][4][1024]
    printf("\n\tDEBUG : efficiently calculate time\n");
    float cumulative_time_bin[numOfBoards][4][1024];
    float sum_time_bin_buf;
    for(int iboard=0; iboard<numOfBoards; iboard++){
        for(int ich=0; ich<4; ich++){
            for(int icell=0; icell<1024; icell++){
                sum_time_bin_buf = 0;
                for(int icell_index=0; icell_index<=icell; icell_index++){
                    sum_time_bin_buf += bin_width[iboard][ich][icell_index];
                }
                cumulative_time_bin[iboard][ich][icell] = sum_time_bin_buf;
            }
        }
    }
    printf("\n\tDEBUG : efficiently calculate time\n");


    

    Int_t serialNumber[numOfBoards]; //"numOfBoards" is not zero-index.
    for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        // set tree data
        serialNumber[iBoard] = serialNumber_buf[iBoard];
    }

    Double_t timeBinWidth[numOfBoards][4][1024];

    Int_t triggerCell[numOfBoards];
    UInt_t scaler[numOfBoards][4];
    Double_t waveform[numOfBoards][4][1024];
    Double_t time[numOfBoards][4][1024];
    Double_t adcSum[numOfBoards][4];

    //--------------------------------------------------
    // Define a tree for board infomation
    //--------------------------------------------------
    auto treeDRS4BoardInfo = new TTree("treeDRS4BoardInfo", "a tree for information of each DRS4 boards");
    treeDRS4BoardInfo->Branch("numOfBoards", &numOfBoards, "numOfBoards/I");
    //
    treeDRS4BoardInfo->Branch("serialNumber", serialNumber, Form("serialNumber[%d]/I", numOfBoards));
    treeDRS4BoardInfo->Branch("timeBinWidth", timeBinWidth, Form("timeBinWidth[%d][4][1024]/D", numOfBoards));

    treeDRS4BoardInfo->Fill();
    treeDRS4BoardInfo->Print();
    treeDRS4BoardInfo->Write();

    //--------------------------------------------------
    // Define a tree for board events
    //--------------------------------------------------
    auto treeDRS4BoardEvent = new TTree("treeDRS4BoardEvent", "a tree for events of each DRS4 boards");
    // treeDRS4BoardEvent->Branch("numOfBoards", &numOfBoards, "numOfBoards/I"); //Infoにあるからいらないよね。
    TTimeStamp *eventTime = new TTimeStamp;
    treeDRS4BoardEvent->Branch("eventTime", "TTimeStamp", &eventTime);
    //

    //iBoardについてforループがあったけど、いらないと判断したので削除
    treeDRS4BoardEvent->Branch("triggerCell", triggerCell, Form("triggerCell[%d]/I", numOfBoards));
    // treeDRS4BoardEvent->Branch("scaler", scaler, "scaler[numOfBoards][4]/i"); //よくわからないブランチ。値を見てもゼロだった。
    treeDRS4BoardEvent->Branch("waveform", waveform, Form("waveform[%d][4][1024]/D", numOfBoards));
    if(TIME_FLAG){
        treeDRS4BoardEvent->Branch("time", time, Form("time[%d][4][1024]/D", numOfBoards));
    }
    treeDRS4BoardEvent->Branch("adcSum", adcSum, Form("adcSum[%d][4]/D", numOfBoards));

    for(Int_t iBoard=0; iBoard<numOfBoards; iBoard++){
        // set tree data
        for(Int_t iCh=0; iCh<4; iCh++){
            for(Int_t iCell=0; iCell<1024; iCell++){
                timeBinWidth[iBoard][iCh][iCell] = bin_width[iBoard][iCh][iCell];
            }
        }
    }


    //--------------------------------------------------
    // Initialize statistics
    //--------------------------------------------------
    ndt = 0;
    sumdt = sumdt2 = 0;

    //--------------------------------------------------
    // Loop over all events in the data file
    //--------------------------------------------------
    for (int n = 0;; n++)
    {
        // read event header
        int ret = fread(&eventHeader, sizeof(eventHeader), 1, f);
        if (ret < 1)
        {
            break;
        }
        if(eventHeader.event_serial_number%10000 == 0){
            printf("Found event #%d %d %d\n", eventHeader.event_serial_number, eventHeader.second, eventHeader.millisecond);
        }
        
        if (debug_frag >= 1)
            PrintEventHeader(&eventHeader);
        eventTime->Set((Int_t)eventHeader.year, (Int_t)eventHeader.month, (Int_t)eventHeader.day,
                       (Int_t)eventHeader.hour, (Int_t)eventHeader.minute, (Int_t)eventHeader.second,
                       (Int_t)eventHeader.millisecond * 1E6,
                       1, 0); // last 1,0-> UTC true, offset zero

        //--------------------------------------------------
        // Loop over all boards in data file for event data
        //--------------------------------------------------
        for (Int_t iBoard = 0; iBoard < numOfBoards; iBoard++)
        {
            flag_b4exp_trig = 0;

            // read board header
            fread(&boardHeader, sizeof(boardHeader), 1, f);
            if (memcmp(boardHeader.bn, "B#", 2) != 0)
            {
                printf("Invalid board header in file \'%s\', aborting.\n", filename);
                return 0;
            }
            if (debug_frag >= 1)
                PrintBoardHeader(&boardHeader);
            // read trigger cell <- Number of first readout cell
            fread(&triggerCellHeader, sizeof(triggerCellHeader), 1, f);
            if (memcmp(triggerCellHeader.tc, "T#", 2) != 0)
            {
                printf("Invalid trigger cell header in file \'%s\', aborting.\n", filename);
                return 0;
            }
            else
            {
                DEBUG_PRINT(1, "   Trigger cell: %d\n", triggerCellHeader.trigger_cell);
                triggerCell[iBoard] = triggerCellHeader.trigger_cell; // Set Tree data
            }
            if (numOfBoards > 1)
            {
                DEBUG_PRINT(1, "Found data for board #%d\n", boardHeader.board_serial_number);
            }
            //--------------------------------------------------
            // Read channel data
            //--------------------------------------------------
            for (int ich = 0; ich < 4; ich++)
            {
                // read channel header
                fread(&channelHeader, sizeof(channelHeader), 1, f);
                if (channelHeader.c[0] != 'C')
                {
                    // event header found
                    fseek(f, -4, SEEK_CUR);
                    break;
                }
                chID = channelHeader.chName[2] - '0' - 1; // = 0,1,2,3

                fread(&scaler_buf, sizeof(int), 1, f); //scaler means ??
                fread(voltage, sizeof(short), 1024, f); //Voltage Bin is data encoded in 2-Byte(16bits) integars. 0=RC-0.5V and 65535=RC+0.5V

                adcSum[iBoard][chID] = 0;
                for (int icell = 0; icell < 1024; icell++)
                {
                    // convert data to volts
                    voltage_buf = (voltage[icell] / 65536.0 + eventHeader.range / 1000.0 - 0.5);
                    if(flag_b4exp_event_selection == 0){
                        flag_b4exp_trig =1;
                    }
                    if(iBoard*4+chID +1 >= 4){
                        if(voltage_buf < thr_V*0.001){
                            flag_b4exp_trig = 1;
                        }
                    }
                    
                    waveform[iBoard][chID][icell] =  voltage_buf; //set tree data
                    // waveform[iboard][chID][icell] = waveform_buf[iboard][chID][icell]; // Set Tree data

                    if(TIME_FLAG && flag_b4exp_trig != 0){
                        cumulative_time_buf = cumulative_time_bin[iBoard][chID][(icell+triggerCell[iBoard])%1024] - cumulative_time_bin[iBoard][chID][triggerCell[iBoard]];

                        if(cumulative_time_buf >= 0){
                            time[iBoard][chID][icell] = cumulative_time_buf;
                        }
                        else{
                            time[iBoard][chID][icell] = cumulative_time_bin[iBoard][chID][1023] + cumulative_time_buf;
                        }

                        // // calculate time for this cell
                        // time[iBoard][chID][icell] = 0;
                        // for (int j = 0; j < icell; j++)
                        // {
                        //     time[iBoard][chID][icell] += bin_width[iBoard][chID][(j + triggerCellHeader.trigger_cell) % 1024];
                        // }
                        // // time[iboard][chID][icell] = time_buf[iboard][chID][icell]; // Set Tree data
                    }
                    
                    adcSum[iBoard][chID] += waveform[iBoard][chID][icell];     // Set Tree data
                    DEBUG_PRINT(3, "bd%d ch%d cell%d:, v=%f, sum=%f\n", iBoard, chID, icell, waveform[iBoard][chID][icell], adcSum[iBoard][chID]);
                }
                DEBUG_PRINT(2, "bd%d ch%d, adcSum=%f\n", iBoard, chID, adcSum[iBoard][chID]);
            }
        }
        if(flag_b4exp_trig != 0){
            treeDRS4BoardEvent->Fill();
        }
    } // end of event loop;

    treeDRS4BoardEvent->Print();
    treeDRS4BoardEvent->Write();

    return treeDRS4BoardEvent->GetEntries();
}
