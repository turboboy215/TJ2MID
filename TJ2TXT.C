/*Technos Japan (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

const char MagicBytesA[9] = { 0x85, 0x6F, 0x3E, 0x00, 0x8C, 0x67, 0x5E, 0x23, 0x56 };
const char MagicBytesB[5] = { 0xCE, 0x00, 0x67, 0x5E, 0x23 };

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[4];
long songPtr;
int chanMask;
long bankAmt;
int foundTable = 0;
long firstPtr = 0;

unsigned static char* romData;

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Technos Japan (GB/GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: TJ2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Try to search the bank for song table loader - Method 1: Double Dragon 1*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesA, 9)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
				}
			}

			/*Method 2: Nintendo World Cup/Double Dragon 2*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesB, 5)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i - 4;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = romData[tablePtrLoc - bankAmt] + (romData[tablePtrLoc + 3 - bankAmt] * 0x100);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
				}
			}

			if (foundTable == 1)
			{
				i = tableOffset - bankAmt;
				firstPtr = ReadLE16(&romData[i]);
				songNum = 1;
				while (ReadLE16(&romData[i]) > bankAmt && ReadLE16(&romData[i]) < (bankSize * 2))
				{
					songPtr = ReadLE16(&romData[i]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					if (songPtr != 0)
					{
						song2txt(songNum, songPtr);
					}
					i += 2;
					songNum++;
				}
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				exit(-1);
			}
			printf("The operation was successfully completed!\n");
			return 0;
		}
	}
}

void song2txt(int songNum, long ptr)
{
	unsigned char mask = 0;
	long channels[4] = { 0, 0, 0, 0 };
	long romPos = 0;
	long seqPos = 0;
	int curTrack = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	long jumpPos = 0;
	long jumpPosRet = 0;
	long macroPos = 0;
	long macroRet = 0;
	int inMacro = 0;
	int repeat1Times = 0;
	int repeat1Pos = 0;
	int repeat2Times = 0;
	int repeat2Pos = 0;
	int transpose = 0;
	int tempo = 0;
	long insPtr = 0;
	long loopPt = 0;
	int repNote = 0;
	int repNoteMode = 0;
	unsigned char command[4];
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;

	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file seqs.txt!\n");
		exit(2);
	}
	else
	{
		romPos = ptr - bankAmt;

		/*Get the channel mask*/
		mask = romData[romPos];
		fprintf(txt, "Channel mask: %01X\n", mask);
		romPos++;

		while (romData[romPos] != 0xFF)
		{
			switch (romData[romPos])
			{
				case 0x00:
					channels[0] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "Music channel 1: 0x%04X\n", channels[0]);
					break;
				case 0x01:
					channels[1] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "Music channel 2: 0x%04X\n", channels[1]);
					break;
				case 0x02:
					channels[2] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "Music channel 3: 0x%04X\n", channels[2]);
					break;
				case 0x03:
					channels[3] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "Music channel 4: 0x%04X\n", channels[3]);
					break;
				case 0x04:
					channels[0] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "SFX channel 1: 0x%04X\n", channels[0]);
					break;
				case 0x05:
					channels[1] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "SFX channel 2: 0x%04X\n", channels[1]);
					break;
				case 0x06:
					channels[2] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "SFX channel 3: 0x%04X\n", channels[2]);
					break;
				case 0x07:
					channels[3] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "SFX channel 4: 0x%04X\n", channels[3]);
					break;
				default:
					channels[0] = ReadLE16(&romData[romPos + 1]);
					fprintf(txt, "Music channel 1: 0x%04X\n", channels[0]);
					break;
			}
			romPos += 3;
		}

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			if (channels[curTrack] >= bankAmt)
			{
				seqPos = channels[curTrack] - bankAmt;
				fprintf(txt, "Channel %i:\n", (curTrack + 1));
				seqEnd = 0;
				inMacro = 0;
				transpose = 0;
				repNoteMode = 0;
				fprintf(txt, "\n");

				while (seqEnd == 0)
				{
					command[0] = romData[seqPos];
					command[1] = romData[seqPos + 1];
					command[2] = romData[seqPos + 2];
					command[3] = romData[seqPos + 3];

					if (command[0] < 0x40)
					{
						curNoteLen = command[0];
						fprintf(txt, "Set note length: %01X\n", curNoteLen);
						seqPos++;
					}
					
					else if (command[0] >= 0x40 && command[0] < 0x80)
					{
						curNote = command[0];
						fprintf(txt, "Play note: %01X\n", curNote);
						seqPos++;
					}

					else if (command[0] >= 0x80 && command[0] < 0xC0)
					{
						curNote = command[0];
						fprintf(txt, "Play note with custom size?: %01X, %01X\n", curNote, command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC0)
					{
						fprintf(txt, "Set noise type\n");
						seqPos++;
					}

					else if (command[0] == 0xC1)
					{
						fprintf(txt, "Sweep effect: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC2)
					{
						fprintf(txt, "Set tuning: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC3)
					{
						fprintf(txt, "Portamento effect: %01X, %01X\n", command[1], command[2]);
						seqPos += 3;
					}

					else if (command[0] == 0xC4)
					{
						fprintf(txt, "Set volume/envelope: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC5)
					{
						fprintf(txt, "Turn on 'buzz' effect?\n");
						seqPos++;
					}

					else if (command[0] == 0xC6)
					{
						fprintf(txt, "Turn off 'buzz' effect?\n");
						seqPos++;
					}

					else if (command[0] == 0xC7)
					{
						fprintf(txt, "Set note size: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC8)
					{
						fprintf(txt, "Wave setting?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xC9)
					{
						fprintf(txt, "Rest\n");
						seqPos++;
					}

					else if (command[0] == 0xCA)
					{
						fprintf(txt, "Turn on effect mode\n");
						seqPos++;
					}

					else if (command[0] == 0xCB)
					{
						fprintf(txt, "Turn off effect mode\n");
						seqPos++;
					}

					else if (command[0] == 0xCC)
					{
						tempo = command[1];
						fprintf(txt, "Set tempo: %01X\n", tempo);
						seqPos += 2;
					}

					else if (command[0] == 0xCD)
					{
						insPtr = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Set instrument (pointer): 0x%04X\n", insPtr);
						seqPos += 3;
					}

					else if (command[0] == 0xCE)
					{
						repeat1Times = command[1];
						repeat1Pos = ReadLE16(&romData[seqPos + 2]);
						fprintf(txt, "Repeat section %i times (v1): 0x%04X\n", repeat1Times, repeat1Pos);
						seqPos += 3;
					}

					else if (command[0] == 0xCF)
					{
						repeat2Times = command[1];
						repeat2Pos = ReadLE16(&romData[seqPos + 2]);
						fprintf(txt, "Repeat section %i times (v2): 0x%04X\n", repeat2Times, repeat2Pos);
						seqPos += 3;
					}

					else if (command[0] == 0xD0)
					{
						loopPt = ReadLE16(&romData[seqPos + 1]);
						fprintf(txt, "Go to loop point: 0x%04X\n\n", loopPt);
						seqEnd = 1;
					}

					else if (command[0] == 0xD1)
					{
						macroPos = ReadLE16(&romData[seqPos + 1]);
						macroRet = seqPos + 3;
						fprintf(txt, "Go to macro: 0x%04X\n", macroPos);
						seqPos += 3;
					}

					else if (command[0] == 0xD2)
					{
						fprintf(txt, "Return from macro\n");
						seqPos++;
					}

					else if (command[0] == 0xD3)
					{
						fprintf(txt, "Stop channel\n\n");
						seqEnd = 1;
					}

					else if (command[0] == 0xD4)
					{
						fprintf(txt, "Tom tom effect: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xD5)
					{
						fprintf(txt, "Disable tom tom?\n");
						seqPos++;
					}

					else if (command[0] == 0xD6)
					{
						fprintf(txt, "Turn on 'distortion pitch'\n");
						seqPos++;
					}

					else if (command[0] == 0xD7)
					{
						fprintf(txt, "Turn off 'distortion pitch'\n");
						seqPos++;
					}

					else if (command[0] == 0xD8)
					{
						transpose = (signed char)command[1];
						fprintf(txt, "Set transpose: %i\n", transpose);
						seqPos += 2;
					}

					else if (command[0] == 0xD9)
					{
						fprintf(txt, "Play drum?: %01X\n", command[1]);
						seqPos += 2;
					}

					else if (command[0] == 0xDA)
					{
						repNote = command[1];
						fprintf(txt, "Repeat the following note %i times\n", repNote);
						seqPos += 2;
					}

					else if (command[0] == 0xDB)
					{
						fprintf(txt, "Set vibrato: %01X, %01X, %01X\n", command[1], command[2], command[3]);
						seqPos += 4;
					}

					else if (command[0] == 0xDC)
					{
						fprintf(txt, "Enable vibrato?\n");
						seqPos++;
					}

					else if (command[0] == 0xDD)
					{
						fprintf(txt, "Disable vibrato?\n");
						seqPos++;
					}

					else
					{
						fprintf(txt, "Unknown command: %01X\n", command[0]);
						seqPos++;
					}
				}

			}
		}

		fclose(txt);
	}
}