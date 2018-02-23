//Assignment2_Daler

#define WIN32_LEAN_AND_MEAN 
#define NOATOM
#define NOCLIPBOARD
#define NOCOMM
#define NOCTLMGR
#define NOCOLOR
#define NODEFERWINDOWPOS
#define NODESKTOP
#define NODRAWTEXT
#define NOEXTAPI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOTIME
#define NOIMM
#define NOKANJI
#define NOKERNEL
#define NOKEYSTATES
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NONCMESSAGES
#define NOPROFILER
#define NORASTEROPS
#define NORESOURCE
#define NOSCROLL
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOSYSPARAMS
#define NOTEXTMETRIC
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINDOWSTATION
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define OEMRESOURCE
#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

#define DATALEN 56
#define KEYLEN 8  //the first 8 byte of the data is called key, which I am gonna sort by
typedef struct _RECORD {
	TCHAR key[KEYLEN]; //Key part of each record
	TCHAR data[DATALEN]; //the rest of the part
} RECORD;  


#define RECSIZE sizeof (RECORD) //Record's Size - Used below

int _tmain(int argc, LPTSTR argv[])
{

	SECURITY_ATTRIBUTES stdOutSA = /* SA for inheritable handle. */
	{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE *hProc;  /* Pointer to an array of proc handles. */

	TCHAR commandLine[200];
	HANDLE hInTempFile, hOutTempFile;
	RECORD data;
	HANDLE STDInput, STDOutput; //pointers to the resources that we have. HANDLE = Datatype, STDInput and STDOutput = variables 
	LARGE_INTEGER FileSize;  //The structure that I created for storing the file(input's) size
	DWORD BIn, Bout;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startUpRight, startUpLeft;
	STDInput = GetStdHandle(STD_INPUT_HANDLE); //gets the std input handle and puts it into our variables so we can use it
	STDOutput = GetStdHandle(STD_OUTPUT_HANDLE); //gets the std output handle and puts it into our variables so we can use it

	int processes = atoi(argv[1]); //argv[0] is the parent itself, argv[1] is the # of processes we wanna start

	GetFileSizeEx(STDInput, &FileSize); //Getting the size of the file and storing it into the FileSize (large integer variable)


	if (processes == 1) //If processes == 1, I dont have to start any extra processes, just the parent should do the work
	{
	
		RECORD *Allitems;  //creating the dynamic array
		Allitems = malloc(FileSize.QuadPart);
		int numberOfElements = FileSize.QuadPart / 64;
		for (int x = 0; x <= numberOfElements-1; x ++)  //reading the STD input into dynamic array
		{
			ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
			Allitems[x] = data;
		}

		//Sorting by the key
		int i, j;
		RECORD temp;
		for (i = 0; i < numberOfElements; i++)
		{
			for (j = 0; j < (numberOfElements - i - 1); j++)
			{
				if (strcmp (Allitems[j].key, Allitems[j + 1].key) == 1) {
					temp = Allitems[j];
					Allitems[j] = Allitems[j + 1];
					Allitems[j + 1] = temp;
				}
			}
		}

		//Writing the sorted file to the STD output
		for (int list = 0; list < numberOfElements; list++)
		{
			data = Allitems[list];
			WriteFile(STDOutput, &data, RECSIZE, &Bout, NULL);
		}

		free(Allitems);
	}
	else //(Processes > 1)
	{

		HANDLE LeftChild;
		HANDLE RightChild;

		HANDLE LeftChildIn;
		HANDLE RightChildIn;
		HANDLE LeftChildOut;
		HANDLE RightChildOut;

		//arrays for the names of the temp files 
		char LeftInFileName[100];
		char LeftOutFileName[100];
		char RightInFileName[100];
		char RightOutFileName[100];


		//creating the names for the temp files

			if (GetTempFileName(_T("."), _T("gtm"), 0, LeftInFileName) == 0) //creating the names for input
			{
				printf("Temp file failure.");
				exit(1);
			}
			if (GetTempFileName(_T("."), _T("gtm"), 0, RightInFileName) == 0) //creating the names for input
			{
				printf("Temp file failure.");
				exit(1);
			}
			if (GetTempFileName(_T("."), _T("gtm"), 0, LeftOutFileName) == 0) //creating the names for output
			{
				printf("Temp file failure.");
				exit(1);
			}
			if (GetTempFileName(_T("."), _T("gtm"), 0, RightOutFileName) == 0) //creating the names for output
			{
				printf("Temp file failure.");
				exit(1);
			}


			//creating the temp files 

			LeftChildIn =
				CreateFile(LeftInFileName,  //creating the input file
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


			RightChildIn =
				CreateFile(RightInFileName,  //creating the input file
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


			LeftChildOut =
				CreateFile(LeftOutFileName,  //creating the input file
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


			RightChildOut =
				CreateFile(RightOutFileName,  //creating the input file
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


			//half of the data to each input file

			int NumberOfTheRecords = (FileSize.QuadPart / 64);

			if (NumberOfTheRecords % 2 == 0) { //number of the records is even
				for (int i = 1; i <= (NumberOfTheRecords / 2); i++) {
					ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
					WriteFile(LeftChildIn, &data, RECSIZE, &Bout, NULL);
				}
				for (int i = 1; i <= (NumberOfTheRecords / 2); i++) {
					ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
					WriteFile(RightChildIn, &data, RECSIZE, &Bout, NULL);
				}

			}
			else //number of the records is not even
			{
				int FirstHalf = NumberOfTheRecords / 2;
				int SecondHalf = FirstHalf + 1;

				for (int i = 1; i <= FirstHalf; i++) {
					ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
					WriteFile(LeftChildIn, &data, RECSIZE, &Bout, NULL);
				}
				for (int i = 1; i <= SecondHalf; i++) {
					ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
					WriteFile(RightChildIn, &data, RECSIZE, &Bout, NULL);
				}
			}

			//Setting the pointer to the beginning again, otherwise it stays at the end

			LARGE_INTEGER spot ;
			spot.QuadPart = 0;

			SetFilePointerEx(RightChildIn, spot, NULL, FILE_BEGIN); //for right
			SetFilePointerEx(LeftChildIn, spot, NULL, FILE_BEGIN);  //for left

			//Getstartupinfo - Telling the child where it's std input, error and output comes from and goes to

			GetStartupInfo(&startUpRight);
			GetStartupInfo(&startUpLeft);

			startUpRight.dwFlags = STARTF_USESTDHANDLES;
			startUpLeft.dwFlags = STARTF_USESTDHANDLES;

			startUpRight.hStdOutput = RightChildOut;
			startUpRight.hStdError = RightChildOut;
			startUpRight.hStdInput = RightChildIn;

			startUpLeft.hStdOutput = LeftChildOut;
			startUpLeft.hStdError = LeftChildOut;
			startUpLeft.hStdInput = LeftChildIn;

			//Commandline
			int half = processes / 2;
			sprintf(commandLine, "SortMP %d", half);

			//Creating the processes

			if (!CreateProcess(NULL, commandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &startUpRight, &processInfo))
				printf("ProcCreate failed.");

			LeftChild = processInfo.hProcess;

			if (!CreateProcess(NULL, commandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &startUpLeft, &processInfo))
				printf("ProcCreate failed.");

			RightChild = processInfo.hProcess;

			//Waiting for children to get done
			WaitForSingleObject(RightChild, INFINITE);
			WaitForSingleObject(LeftChild, INFINITE);

			//Reading temp_output files into dynamic arrays

			LARGE_INTEGER RightFileSize;
			LARGE_INTEGER LeftFileSize;

			GetFileSizeEx(RightChildOut, &RightFileSize);
			GetFileSizeEx(LeftChildOut, &LeftFileSize);
			

			//moving the pointers back to the beginning 
			LARGE_INTEGER OutSpot;
			OutSpot.QuadPart = 0;

			SetFilePointerEx(RightChildOut, spot, NULL, FILE_BEGIN); //for input
			SetFilePointerEx(LeftChildOut, spot, NULL, FILE_BEGIN);  //for output


			RECORD *AllRightItems;  
			AllRightItems = malloc(RightFileSize.QuadPart);
			int numberOfRightElements = RightFileSize.QuadPart / 64;
			for (int x = 0; x <= numberOfRightElements - 1; x++)  
			{
				ReadFile(RightChildOut, &data, RECSIZE, &BIn, NULL);
				AllRightItems[x] = data;
			}

			RECORD *AllLeftItems;  
			AllLeftItems = malloc(LeftFileSize.QuadPart);
			int numberOfLeftElements = LeftFileSize.QuadPart / 64;
			for (int x = 0; x <= numberOfLeftElements - 1; x++)  
			{
				ReadFile(LeftChildOut, &data, RECSIZE, &BIn, NULL);
				AllLeftItems[x] = data;
			}

			//creating another dynamic array so I can store the merged results

			RECORD *AllMergedItems;
			AllMergedItems = malloc(RightFileSize.QuadPart + LeftFileSize.QuadPart);
			int numberOfMergedElements = (RightFileSize.QuadPart + LeftFileSize.QuadPart) / 64;

			//Merging
			
			int i = 0, j = 0, k = 0;
			
			while (i < numberOfRightElements && j < numberOfLeftElements) {
				if (strcmp(AllRightItems[i].key, AllLeftItems[j].key) == -1) {
					AllMergedItems[k++] = AllRightItems[i++];
				}
				else {
					AllMergedItems[k++] = AllLeftItems[j++];
				}
			}

			while (i < numberOfRightElements) {
				AllMergedItems[k++] = AllRightItems[i++];
			}
			while (j < numberOfLeftElements) {
				AllMergedItems[k++] = AllLeftItems[j++];
			}
			
			for (int loop = 0; loop < numberOfMergedElements; loop++) {
				data = AllMergedItems[loop];
				WriteFile(STDOutput, &data, RECSIZE, &BIn, NULL);
			}

			CloseHandle(LeftChildOut);
			CloseHandle(LeftChildIn);
			CloseHandle(RightChildOut);
			CloseHandle(RightChildIn);

			////Deleting the temp files
			DeleteFile(RightInFileName);
			DeleteFile(LeftInFileName);
			DeleteFile(RightOutFileName);
			DeleteFile(LeftOutFileName);


			free(AllRightItems);
			free(AllLeftItems);
			free(AllMergedItems);
	}

	//system("pause"); 
	return 0;
}