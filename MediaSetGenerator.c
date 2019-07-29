/*****************************************************************************
* 
* This software has been developed by LEONARDO MW Limited 
* and is wholly owned by LEONARDO MW Limited. 
*
* The copyright in this software is the property of LEONARDO MW Limited.
* The software is supplied by LEONARDO MW Limited on the express understanding 
* that it is treated as confidential and that it may not be copied, used or 
* disclosed to others in whole or in part for any purpose except as authorised in 
* writing by LEONARDO MW Limited.
*
* (c) LEONARDO MW Limited 2019
* Proprietary Information - All Rights Reserved.
*
******************************************************************************/
/*****************************************************************************
* CLASSIFICATION   : UNCLASSIFIED
* AUTHOR           : Bradley Yates
* FILENAME         : MediaSetGenerator.c
* PROJECT/NUMBER   : ES-05 RAVEN
* DATE WRITTEN     : 22-07-19
* COMPILER         : LabWindows CVI
* COMPILATION CMD  : See Makefile
* ADDITIONAL FILES : See Include Files below
* FUNCTION         : Source code for application
*
* SCR No    DATE       DESCRIPTION                                     INITLS
* ------    ----       -----------                                     ------
*  6566     22-07-19   Initial version                                   BY
*  6571     23-07-19   Fixed defects in v1 (DR_4725)                     BY
*
******************************************************************************/ 
//==============================================================================
// Include files

#include <ansi_c.h>
#include <cvirte.h>     
#include <userint.h> 
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "MediaSetGenerator.h"
#include "toolbox.h"

//==============================================================================
// Constants
#define HEADER_FILE "header.LUH"
#define LOADS_FILE "loads.LUM"
#define FILES_FILE "files.LUM"

#define H_FILE_LENGTH                      0
#define H_FILE_FORMAT                      2
#define H_PART_FLAGS                       3
    
#define H_LOAD_PART_NUMBER_LENGTH_PTR      4
#define H_NUMBER_OF_TARGETS_PTR            6
#define H_NUMBER_OF_DATA_FILES_PTR         8
#define H_NUMBER_OF_SUPPORT_FILES_PTR      10
#define H_USER_DEFINED_DATA_PTR            12
#define H_LOAD_TYPE_DESCRIPTION_LENGTH_PTR 14
#define H_NUMBER_OF_TARGET_HARDWARE_PTR    16
#define H_LOAD_CHECK_VALUE_LENGTH_PTR      18
    
#define H_LOAD_PART_NUMBER_LENGTH          20  
#define H_LOAD_PART_NUMBER                 21  //Variable Length
    
#define H_NUMBER_OF_TARGET_HARDWARE_ID     21
#define H_TARGET_HARDWARE_ID_LENGTH        22  
#define H_TARGET_HARDWARE_ID               23  //Variable Length
    
#define H_NUMBER_OF_DATA_FILES             23
#define H_DATA_FILE_PTR                    24
#define H_DATA_FILE_NAME_LENGTH            25
#define H_DATA_FILE_NAME                   26  //Variable Length
#define H_DATA_FILE_PART_NUMBER_LENGTH     26
#define H_DATA_FILE_PART_NUMBER            27  //Variable Length
#define H_DATA_FILE_LENGTH                 27
#define H_DATA_FILE_CRC                    29 
#define H_DATA_FILE_LENGTH_BYTES           30
#define H_DATA_FILE_CHECK_VALUE_LENGTH     34
    
#define H_FILE_CRC                         35
#define H_LOAD_CRC                         36

#define H_MIN_SIZE                         38

//-----------------------------------------------

#define L_FILE_LENGTH                      0
#define L_FILE_FORMAT                      2
#define L_SPARE                            3

#define L_MEDIA_SET_PART_NUMBER_PTR        4
#define L_NUMBER_OF_LOADS_PTR              6
#define L_USER_DEFINED_DATA_PTR            8

#define L_MEDIA_SET_PN_LENGTH              10
#define L_MEDIA_SET_PN                     11 //Variable Length
#define L_MEDIA_SEQ_NUM_AND_NUM_MEMBERS    11
#define L_NUM_OF_LOADS                     12
#define L_LOAD_POINTER                     13
#define L_LOAD_PN_LENGTH                   14
#define L_LOAD_PN                          15 //Variable Length
#define L_HEADER_FILE_NAME_LENGTH          15
#define L_HEADER_FILE_NAME                 16 //Variable Length
#define L_MEMBER_SEQUENCE_NUMBER           16
#define L_NUMBER_OF_TARGET_HW_ID           17
#define L_TARGET_HW_ID_LENGTH              18
#define L_TARGET_HW_ID                     19

#define L_FILE_CRC                         19

#define L_MIN_SIZE                         20 

//-----------------------------------------------

#define F_FILE_LENGTH                      0 
#define F_FILE_FORMAT                      2
#define F_SPARE                            3

#define F_MEDIA_SET_PN_LENGTH_PTR          4
#define F_NUMBER_OF_MEDIA_SET_FILES_PTR    6
#define F_USER_DEFINED_DATA_PTR            8
#define F_FILE_CHECK_VAL_LENGTH_PTR        10

#define F_MEDIA_SET_PN_LENGTH              12
#define F_MEDIA_SET_PN                     13 //Variable Length
#define F_MEDIA_SEQ_NUM_AND_NUM_MEMBERS    13
#define F_NUMBER_OF_MEDIA_SET_FILES        14
#define F_FILE_POINTER                     15
#define F_FILE_NAME_LENGTH                 16
#define F_FILE_NAME                        17 //Variable Length
#define F_FILE_PATHNAME_LENGTH             17
#define F_FILE_PATHNAME                    18 //Variable Length
#define F_MEMBER_SEQUENCE_NUMBER           18
#define F_FILE_CRC                         19
#define F_FILE_CHECK_VAL_LENGTH            20

#define F_FILES_CRC                        21

#define F_MIN_SIZE                         22

//============================================================================== 
// Types

typedef uint64_t UInt64;
typedef uint32_t UInt32;
typedef uint8_t UInt08;
typedef uint16_t UInt16;

//============================================================================== 
// Static global variables

static int panelHandle;

//============================================================================== 
// Static functions

//============================================================================== 
// Global variables

const UInt32 FILE_SIZE = 256;

char fileBuffer[256];

UInt32 crcTable[256];

//Structure to hold all data coming in from UI
struct userDefinedDataStruct
{
    char supplierID[10];
    char uniqueID[4];
    char dataFileName[256];
    char dataFilePartNumber[256];
    char targetHWID[256];
    char loadableSoftwarePN[256];
    char pathName[256];
    
}userDefinedData;

    
//==================================================================================== 
// Global functions

/*************************************************************************************/
/**		Function:		Main		                                                **/
/**		Description:	Runs the user interface                                     **/
/*************************************************************************************/
int main (int argc, char *argv[])
{
    int error = 0;
    
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "MediaSetGenerator.uir", PANEL));
    
    /* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
    errChk (RunUserInterface ());

Error:
    /* clean up */
    DiscardPanel (panelHandle);
    return 0;
}

/*************************************************************************************/
/**		Function:		panelCB		                                                **/
/**		Description:	Handles callback for user closing the interface             **/
/*************************************************************************************/

int CVICALLBACK panelCB (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface (0);
    return 0;
}

/*************************************************************************************/
/**		Function:		getFileName                                                 **/
/**		Description:	Chops a files path off leaving only name + extension        **/
/*************************************************************************************/

char*getFileName(char *pathName)
{
    const char delim[2] = "\\";
    char *token;
    char *lastToken;
    token = strtok(pathName, delim); 
         
    while(token != NULL)
    {
       lastToken = token;
       token = strtok(NULL, delim);
    }
                
    return lastToken;
}

/*************************************************************************************/
/**		Function:		browse		                                                **/
/**		Description:	Callback for when browse button is hit                      **/
/**                     Displays file selector. When file is selected all fields    **/
/**                     are un-dimmed and made available for input                  **/
/*************************************************************************************/

int CVICALLBACK browse (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    
    char pathName[MAX_PATHNAME_LEN] = "";
    
	switch (event) {
		case EVENT_COMMIT:
                
            if(FileSelectPopup("", "", "", "Browse", VAL_OK_BUTTON, 0, 0, 1, 0 ,pathName) != 0)
            {
                SetCtrlVal(panelHandle, PANEL_FILEPATH_TEXTBOX, pathName);
                SetCtrlAttribute(panelHandle, PANEL_SOFTWARE_SUPPLIER_ID, ATTR_DIMMED, 0);
    	        SetCtrlAttribute(panelHandle, PANEL_UNIQUE_PRODUCT_ID, ATTR_DIMMED, 0);
                SetCtrlAttribute(panelHandle, PANEL_DATA_FILE_NAME, ATTR_DIMMED, 0);
                SetCtrlAttribute(panelHandle, PANEL_DATA_FILE_PART_NUMBER, ATTR_DIMMED, 0);
                SetCtrlAttribute(panelHandle, PANEL_TARGET_HARDWARE_ID, ATTR_DIMMED, 0);
                SetCtrlAttribute(panelHandle, PANEL_GENERATE_MEDIA_SET, ATTR_DIMMED, 0); 
                
                strcpy(userDefinedData.pathName, pathName);
                SetCtrlVal(panelHandle, PANEL_DATA_FILE_NAME, getFileName(pathName));
            }
      break;
    }
	return 0;
}

 
//                         ******** NOT ALL MY CODE ********                         //
//===================================================================================== 

/*************************************************************************************/
/**		Function:		populateFileBuffer                                          **/
/**		Description:	NOT MY CODE                                                 **/
/*************************************************************************************/

void populateFileBuffer()
{
	for(UInt32 i = 0; i < 256; ++i)
	{
		fileBuffer[i] = (char)(i);;
	}
}

/*************************************************************************************/
/**		Function:		reflect                                                     **/
/**		Description:	NOT MY CODE                                                 **/
/*************************************************************************************/

UInt32 reflect(const UInt32 value, UInt32 width)
{
	UInt32 reflectedValue = 0;
	for(UInt32 i = 0; i < width; ++i)
	{
		if((value & (1 << i)) != 0)
		{
			reflectedValue |= (1 << ((width-1) - i));
		}
	}
	return reflectedValue;
}

/*************************************************************************************/
/**		Function:		calculateCRC                                                **/
/**		Description:	PARTIALLY MY CODE                                           **/
/*************************************************************************************/


UInt32 calculateCRC(char* data, UInt32 dataSize, UInt32 width, UInt32 polynomial, UInt32 initialValue, UInt32 finalXorValue, int inputReflect, int outputReflect, FILE* fp)
{
    //NOT MINE -------------------------------------------------------------
	UInt32 bitMask = 0;                                                   //
	switch(width)                                                         //
	{                                                                     //
		case 8: bitMask = 0xFF; break;                                    //
		case 16: bitMask = 0xFFFF; break;                                 //
		case 32: bitMask = 0xFFFFFFFF; break;                             //
		default: return 0;                                                //
	}                                                                     //
	const UInt32 msbMask = (0x01 << (width - 1));                         //
                                                                          //
	// Populate CRC table                                                 //
	for(UInt32 divident = 0; divident < 256; ++divident)                  //
	{                                                                     //
		UInt32 currentByte = (divident << (width - 8)) & bitMask;         //
		for(UInt08 bit = 0; bit < 8; ++bit)                               //
		{                                                                 //
			if((currentByte & msbMask) != 0)                              //
			{                                                             //
				currentByte <<= 1;                                        //
				currentByte ^= polynomial;                                //
			}                                                             //
			else                                                          //
			{                                                             //
				currentByte <<= 1;                                        //
			}                                                             //
		}                                                                 //
		crcTable[divident] = (currentByte & bitMask);                     //
	}                                                                     //
    // ---------------------------------------------------------------------                                                                    
    
    //MY CODE --------------------------------------------------------------
    UInt32 crc = initialValue;                                            //
    int end = 0;                                                          //
    int ret_code = 1;                                                     //
    UInt08 currentByte = 0;                                               //
                                                                          //
   	for(UInt32 i = 0; i < dataSize && end != 1; ++i)                      //
   	{                                                                     //
                                                                          //
        if(fp == NULL)                                                    //
        {                                                                 //
            currentByte = data[i];                                        //
        }else                                                             //
        {                                                                 //
            ret_code = fread(&currentByte, sizeof(currentByte), 1, fp);   //
                                                                          //
            if(ret_code == 0)                                             //
            {                                                             //
                end = 1;                                                  //
                break;                                                    //
            }                                                             //
                                                                          //
        }                                                                 //
    // ---------------------------------------------------------------------     
   
    //NOT MINE -------------------------------------------------------------
        if(inputReflect)                                                  //
        {                                                                 //
            currentByte = (unsigned char)(reflect(currentByte, 8) & 0xFF);//
        }                                                                 //
                                                                          //
       	crc = (crc ^ (currentByte << (width - 8))) & bitMask;             //
                                                                          //
       	UInt32 pos = (crc >> (width - 8)) & 0xFF;                         //
       	crc = (crc << 8) & bitMask;                                       //
       	crc = (crc ^ crcTable[pos]) & bitMask;                            //
                                                                          //
    }                                                                     //
                                                                          //
	if(outputReflect)                                                     //
	{                                                                     //
		crc = (reflect(crc, width) & bitMask);                            //
	}                                                                     //
	return ((crc ^ finalXorValue) & bitMask);                             //
    // --------------------------------------------------------------------- 
}

/*************************************************************************************/
/**		Function:		fsize                                                       **/
/**		Description:	returns size of file in bytes                               **/
/*************************************************************************************/

unsigned int fsize(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    unsigned int len = (unsigned int)ftell(fp);    
    fclose(fp);
    
    return len;
}

/*************************************************************************************/
/**		Function:		populateHeaderFile                                          **/
/**		Description:	Populates headerFile array with data                        **/
/**                     SEE ARINC-655-3 SPEC                                        **/
/*************************************************************************************/

unsigned int populateHeaderFile(short** ppsHeaderFile)
{
    int OFFSET = 0;
    int CURR_OFFSET = 0;
    short length = 0;
    unsigned int fileSize = 0;   
    int dataSize = 0;
    short* headerFile = *ppsHeaderFile;
    FILE* fp = fopen(userDefinedData.pathName, "r");
   
//HEADER DATA -------------------------------------------------------------------------
   
    //headerfile length at bottom *  
    headerFile[H_FILE_FORMAT]                                              = ToBigEndian16(0x8004);                       
    headerFile[H_PART_FLAGS]                                               = ToBigEndian16(0);                            

//POINTERS ----------------------------------------------------------------------------
    
    *((unsigned int   *)&headerFile[H_LOAD_PART_NUMBER_LENGTH_PTR])        = ToBigEndian32(20);                            //20 = 20 16-bit words into the file
    
    OFFSET = strlen(userDefinedData.loadableSoftwarePN) / 2;
    
    *((unsigned int   *)&headerFile[H_NUMBER_OF_TARGETS_PTR])              = ToBigEndian32(22+OFFSET);                     //22 = 22 16-bit words into the file + var length
    
    OFFSET += strlen(userDefinedData.targetHWID) / 2;
    
    *((unsigned int   *)&headerFile[H_NUMBER_OF_DATA_FILES_PTR])           = ToBigEndian32(25+OFFSET);                     //25 = 25 16-bit words into the file + running var lengths
    
    *((unsigned int   *)&headerFile[H_NUMBER_OF_SUPPORT_FILES_PTR])        = ToBigEndian32(0);
    *((unsigned int   *)&headerFile[H_USER_DEFINED_DATA_PTR])              = ToBigEndian32(0);
    *((unsigned int   *)&headerFile[H_LOAD_TYPE_DESCRIPTION_LENGTH_PTR])   = ToBigEndian32(0);
    *((unsigned int   *)&headerFile[H_NUMBER_OF_TARGET_HARDWARE_PTR])      = ToBigEndian32(0);
    *((unsigned int   *)&headerFile[H_LOAD_CHECK_VALUE_LENGTH_PTR])        = ToBigEndian32(0);                  
    
//LOAD PART NUMBER DATA ---------------------------------------------------------------
    
    OFFSET = 0;
    
    length = strlen(userDefinedData.loadableSoftwarePN);
    headerFile[H_LOAD_PART_NUMBER_LENGTH]                                  = ToBigEndian16(length);
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2; 
    headerFile = realloc(headerFile, (H_MIN_SIZE + CURR_OFFSET) * 2);
    strcpy((char*)&headerFile[H_LOAD_PART_NUMBER]                          , userDefinedData.loadableSoftwarePN);          //variable length
    
    OFFSET = CURR_OFFSET;                                                                        
    
//TARGET HW DATA ----------------------------------------------------------------------
    
    headerFile[H_NUMBER_OF_TARGET_HARDWARE_ID + OFFSET]                    = ToBigEndian16(1);
    length = strlen(userDefinedData.targetHWID);
    headerFile[H_TARGET_HARDWARE_ID_LENGTH + OFFSET]                       = ToBigEndian16(length); 
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    headerFile = realloc(headerFile, (H_MIN_SIZE + CURR_OFFSET) * 2);
    strcpy((char*)&headerFile[H_TARGET_HARDWARE_ID + OFFSET]               , userDefinedData.targetHWID);                  //variable length
    
    OFFSET = CURR_OFFSET; 
    
//DATA FILE DATA ----------------------------------------------------------------------   
    
    headerFile[H_NUMBER_OF_DATA_FILES + OFFSET]                            = ToBigEndian16(1);
    headerFile[H_DATA_FILE_PTR + OFFSET]                                   = ToBigEndian16(0);
    
    length = strlen(userDefinedData.dataFileName);
   
    headerFile[H_DATA_FILE_NAME_LENGTH + OFFSET]                           = ToBigEndian16(length);   
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    headerFile = realloc(headerFile, (H_MIN_SIZE + CURR_OFFSET) * 2);
    strcpy((char*)&headerFile[H_DATA_FILE_NAME + OFFSET]                   , userDefinedData.dataFileName);               //variable length   
  
    OFFSET = CURR_OFFSET; 
    
    length = strlen(userDefinedData.dataFilePartNumber);
   
    headerFile[H_DATA_FILE_PART_NUMBER_LENGTH + OFFSET]                    = ToBigEndian16(length); 
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    headerFile = realloc(headerFile, (H_MIN_SIZE + CURR_OFFSET) * 2);
    strcpy((char*)&headerFile[H_DATA_FILE_PART_NUMBER + OFFSET]            , userDefinedData.dataFilePartNumber);          //variable length
  
    OFFSET = CURR_OFFSET; 
    
    fileSize = fsize(fp);
    
    *((unsigned int   *)&headerFile[H_DATA_FILE_LENGTH + OFFSET])          = ToBigEndian32((fileSize + fileSize%2)/2);                     
    
    fp = fopen(userDefinedData.pathName, "rb"); 
    
    const UInt16 crc16 = calculateCRC("NOTUSED", -1, 16,    0x1021,     0xFFFF,     0x0000, 0, 0, fp);
    
    headerFile[H_DATA_FILE_CRC + OFFSET]                                   = ToBigEndian16(crc16);       
    
    *((UInt64  *)&headerFile[H_DATA_FILE_LENGTH_BYTES + OFFSET])           = ToBigEndian64(fileSize);
    
    headerFile[H_DATA_FILE_CHECK_VALUE_LENGTH + OFFSET]                    = ToBigEndian16(0);
    
//CRC'S--------------------------------------------------------------------------------  
    
    *((unsigned int   *)&headerFile[H_FILE_LENGTH])                        = ToBigEndian32(H_MIN_SIZE + OFFSET);                  //* should be at top, offset value required calculated late
   
    dataSize = (H_MIN_SIZE + OFFSET)*2;
        
    const UInt16 crc16_2 = calculateCRC((char*)headerFile, dataSize-6 , 16,    0x1021,     0xFFFF,     0x0000, 0, 0, NULL);       //File crc calced from 6 bytes before eof 
    
    headerFile[H_FILE_CRC + OFFSET]                                        = ToBigEndian16(crc16_2);
    
    UInt32 crc32 = calculateCRC((char*)headerFile, dataSize-4, 32, 0x4C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, NULL);                //Load crc calced from 4 bytes before eof, NOT USED IN GRAHAM'S

    int numOfFiles = 1;

    for(int i=0;i<numOfFiles;i++)
    {
        crc32 = calculateCRC("NOTUSED", 0, 32, 0x4C11DB7, crc32, 0xFFFFFFFF, 0, 0, fp);
    }
    
    *((unsigned int   *)&headerFile[H_LOAD_CRC + OFFSET])                  = ToBigEndian32(crc32); 
    
    *ppsHeaderFile = headerFile;
    
    fclose(fp);
    
    return (H_MIN_SIZE + OFFSET) * 2;
}

/*************************************************************************************/
/**		Function:		populateLoadsFile                                           **/
/**		Description:	Populates loadsFile array with data                         **/
/**                     SEE ARINC-655-3 SPEC                                        **/
/*************************************************************************************/
 
int populateLoadsFile(short** ppsLoadsFile)
{
    int OFFSET = 0;
    int CURR_OFFSET = 0;
    short length = 0;
    short* loadsFile = *ppsLoadsFile;
    
//HEADER DATA -------------------------------------------------------------------------  
   
    loadsFile[L_FILE_FORMAT]                                               = ToBigEndian16(0x8004);         //headerfile length at bottom *
    loadsFile[L_SPARE]                                                     = ToBigEndian16(1);
    
//POINTERS ----------------------------------------------------------------------------    
   
    *((unsigned int   *)&loadsFile[L_MEDIA_SET_PART_NUMBER_PTR])           = ToBigEndian32(1); 
    *((unsigned int   *)&loadsFile[L_NUMBER_OF_LOADS_PTR])                 = ToBigEndian32(1);
    *((unsigned int   *)&loadsFile[L_USER_DEFINED_DATA_PTR])               = ToBigEndian32(1);

//MEDIA SET & LOAD DATA ---------------------------------------------------------------
    
    length = strlen("MEDIASETPN");
    loadsFile[L_MEDIA_SET_PN_LENGTH]                                       = ToBigEndian16(length);
  
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    loadsFile = realloc(loadsFile, (L_MIN_SIZE + CURR_OFFSET) * 2); 
    strcpy((char*)&loadsFile[L_MEDIA_SET_PN]                               , "MEDIASETPN");

    OFFSET = CURR_OFFSET;    
    
    loadsFile[L_MEDIA_SEQ_NUM_AND_NUM_MEMBERS + OFFSET]                    = 0x0101;                        //two 8 bit number packed together

    loadsFile[L_NUM_OF_LOADS + OFFSET]                                     = ToBigEndian16(1);
    loadsFile[L_LOAD_POINTER + OFFSET]                                     = ToBigEndian16(0);
    
    length = strlen("LOADPN"); 
    loadsFile[L_LOAD_PN_LENGTH + OFFSET]                                   = ToBigEndian16(length);
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    loadsFile = realloc(loadsFile, (L_MIN_SIZE + CURR_OFFSET) * 2);   
    strcpy((char*)&loadsFile[L_LOAD_PN + OFFSET]                           , "LOADPN");
    
    OFFSET = CURR_OFFSET; 
   
    length = strlen("HEADERFILE");
    loadsFile[L_HEADER_FILE_NAME_LENGTH + OFFSET]                          = ToBigEndian16(length);
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    loadsFile = realloc(loadsFile, (L_MIN_SIZE + CURR_OFFSET) * 2); 
    strcpy((char*)&loadsFile[L_HEADER_FILE_NAME + OFFSET]                  , "HEADERFILE");
    
    OFFSET = CURR_OFFSET;
                                                                          
    loadsFile[L_MEMBER_SEQUENCE_NUMBER + OFFSET]                           = ToBigEndian16(1);
    loadsFile[L_NUMBER_OF_TARGET_HW_ID + OFFSET]                           = ToBigEndian16(1);
                                                 
    length = strlen(userDefinedData.targetHWID);
    loadsFile[L_TARGET_HW_ID_LENGTH + OFFSET]                              = ToBigEndian16(length); 
    strcpy((char*)&loadsFile[L_HEADER_FILE_NAME + OFFSET]                  , userDefinedData.targetHWID);
                                                                         
    *((unsigned int   *)&loadsFile[L_FILE_LENGTH + OFFSET])                = ToBigEndian16(L_FILE_CRC + OFFSET); 

//CRC ---------------------------------------------------------------------------------
    
    const UInt16 crc16 = calculateCRC((char*)loadsFile, (L_FILE_CRC + OFFSET)*2, 16,    0x1021,     0xFFFF,     0x0000, 0, 0, NULL);
    
    loadsFile[L_FILE_CRC]                                                  = ToBigEndian16(crc16);

    *ppsLoadsFile = loadsFile;
    
    return (L_MIN_SIZE + OFFSET) * 2;
}

/*************************************************************************************/
/**		Function:		populateFilesFile                                           **/
/**		Description:	Populates filesFile array with data                         **/
/**                     SEE ARINC-655-3 SPEC                                        **/
/*************************************************************************************/

int populateFilesFile(short** ppsFilesFile)
{
    int OFFSET = 0;
    int CURR_OFFSET = 0;
    short length = 0;
    short* filesFile = *ppsFilesFile;
    FILE* fp = fopen(userDefinedData.pathName, "r");
   
//HEADER DATA -------------------------------------------------------------------------       
    
    filesFile[F_FILE_FORMAT]                                               = ToBigEndian16(0x8004);         //headerfile length at bottom *
    filesFile[F_SPARE]                                                     = ToBigEndian16(1);
    
//POINTERS ----------------------------------------------------------------------------    
    
    *((unsigned int   *)&filesFile[F_MEDIA_SET_PN_LENGTH_PTR])             = ToBigEndian32(1); 
    *((unsigned int   *)&filesFile[F_NUMBER_OF_MEDIA_SET_FILES_PTR])       = ToBigEndian32(1);
    *((unsigned int   *)&filesFile[F_USER_DEFINED_DATA_PTR])               = ToBigEndian32(1);
    *((unsigned int   *)&filesFile[F_FILE_CHECK_VAL_LENGTH_PTR])           = ToBigEndian32(1); 
   
//MEDIA SET & FILE DATA --------------------------------------------------------------- 
    length = strlen("MEDIASETPN");
    filesFile[F_MEDIA_SET_PN_LENGTH]                                       = ToBigEndian16(length);
  
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    filesFile = realloc(filesFile, (F_MIN_SIZE + CURR_OFFSET) * 2); 
    strcpy((char*)&filesFile[F_MEDIA_SET_PN]                               , "MEDIASETPN");

    OFFSET = CURR_OFFSET;    
    
    filesFile[F_MEDIA_SEQ_NUM_AND_NUM_MEMBERS + OFFSET]                    = 0x0101;                        //two 8 bit number packed together
                                                                     
    filesFile[F_NUMBER_OF_MEDIA_SET_FILES + OFFSET]                        = ToBigEndian16(1);
    filesFile[F_FILE_POINTER + OFFSET]                                     = ToBigEndian16(0);
    
    length = strlen(userDefinedData.dataFileName);
    filesFile[F_FILE_NAME_LENGTH]                                          = ToBigEndian16(length);
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    filesFile = realloc(filesFile, (F_MIN_SIZE + CURR_OFFSET) * 2); 
    strcpy((char*)&filesFile[F_FILE_NAME]                                  , userDefinedData.dataFileName);
    
    OFFSET = CURR_OFFSET;
    
    length = strlen(userDefinedData.pathName);
    filesFile[F_FILE_PATHNAME_LENGTH]                                      = ToBigEndian16(length);
    
    CURR_OFFSET = OFFSET + (length + (length%2))/2;
    filesFile = realloc(filesFile, (F_MIN_SIZE + CURR_OFFSET) * 2); 
    strcpy((char*)&filesFile[F_FILE_PATHNAME]                              , userDefinedData.pathName);
    
    OFFSET = CURR_OFFSET;
                                                                      
    filesFile[F_MEMBER_SEQUENCE_NUMBER]                                    = ToBigEndian16(1);
    
    const UInt16 crc16 = calculateCRC("NOTUSED", 0, 16,    0x1021,     0xFFFF,     0x0000, 0, 0, fp); 
    
    filesFile[F_FILE_CHECK_VAL_LENGTH]                                     = ToBigEndian16(0);
    
    *((unsigned int   *)&filesFile[F_FILE_LENGTH + OFFSET])                = ToBigEndian16(F_FILE_CRC + OFFSET); 
    
//CRC ---------------------------------------------------------------------------------
    
    const UInt16 crc16_3 = calculateCRC((char*)filesFile, (F_FILE_CRC + OFFSET)*2, 16,    0x1021,     0xFFFF,     0x0000, 0, 0, NULL);
                                                                        
    filesFile[F_FILE_CRC]                                                  = ToBigEndian16(crc16_3);

    *ppsFilesFile = filesFile;
    
    return (F_MIN_SIZE + OFFSET) * 2;
}

/*************************************************************************************/
/**		Function:		userDataPopulated                                           **/
/**		Description:	Gets user data from screen & puts the PN in correct format  **/
/*************************************************************************************/

int userDataPopulated()
{
    char input[13] = "";
    char checkCharacters[3] = "CC";
    
    GetCtrlVal(panelHandle, PANEL_SOFTWARE_SUPPLIER_ID, userDefinedData.supplierID);
    GetCtrlVal(panelHandle, PANEL_UNIQUE_PRODUCT_ID, userDefinedData.uniqueID);
    GetCtrlVal(panelHandle, PANEL_DATA_FILE_NAME, userDefinedData.dataFileName);
    GetCtrlVal(panelHandle, PANEL_DATA_FILE_PART_NUMBER, userDefinedData.dataFilePartNumber);
    GetCtrlVal(panelHandle, PANEL_TARGET_HARDWARE_ID, userDefinedData.targetHWID);
    
    if(strlen(userDefinedData.supplierID) ==0            ||
       strlen(userDefinedData.uniqueID) ==0              ||
       strlen(userDefinedData.dataFileName) ==0          ||
       strlen(userDefinedData.dataFilePartNumber) ==0    ||
       strlen(userDefinedData.targetHWID) ==0)
    {
        SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Please enter data into all fields.");
        return 0;
    }
    
    strcpy(userDefinedData.loadableSoftwarePN, userDefinedData.uniqueID);
    strcat(userDefinedData.loadableSoftwarePN, checkCharacters);
    strcat(userDefinedData.loadableSoftwarePN, "-");
    strcat(userDefinedData.loadableSoftwarePN, userDefinedData.supplierID);
   
   return 1;
}

/*************************************************************************************/
/**		Function:		generateMediaSet                                            **/
/**		Description:	Callback for generate button                                **/
/*************************************************************************************/

int CVICALLBACK generateMediaSet (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    
    short* headerFile;
    short* loadsFile;
    short* filesFile;
    
    int headerSize = 0;
    int loadsSize = 0;
    int filesSize = 0;
    
	switch (event) {
		case EVENT_COMMIT:
            
            if(userDataPopulated() == 0)
            {
               return 1;           
            }

            SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Generating...");
            
            FILE *headerFilePtr;
            FILE *loadsFilePtr;
            FILE *filesFilePtr;
         
            headerFilePtr = fopen(HEADER_FILE,"w");
            loadsFilePtr = fopen(LOADS_FILE,"w");
            filesFilePtr = fopen(FILES_FILE,"w");
          
            if(headerFilePtr == NULL)
            {
                SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Error creating HEADER_FILE.");
                break;
            }else if(loadsFilePtr == NULL)
            {
                SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Error creating LOADS_FILE.");    
                break;
            }else if(filesFilePtr == NULL)
            {
                SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Error creating FILES_FILE.");    
                break;
            }
            
            headerFile = malloc(H_MIN_SIZE * 2);
            loadsFile = malloc(L_MIN_SIZE * 2);
            filesFile = malloc(F_MIN_SIZE * 2);
            
            headerSize = populateHeaderFile(&headerFile);
            loadsSize = populateLoadsFile(&loadsFile);
            filesSize = populateFilesFile(&filesFile);
            
            fwrite(headerFile, headerSize, 1, headerFilePtr);
            fwrite(loadsFile, loadsSize, 1, loadsFilePtr);
            fwrite(filesFile, filesSize, 1, filesFilePtr);
            
            free(headerFile);
            free(loadsFile);
            free(filesFile);
            
            fclose(headerFilePtr);
            fclose(loadsFilePtr);
            fclose(filesFilePtr);
            
            SetCtrlVal(panelHandle, PANEL_TEXTMSG, "Done");
            
           
    	break;
    }

	return 0;
}
