#include <iostream>
#include <iomanip>
#include <time.h>
#include <algorithm>
using namespace std;

#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)
#define		SETCACHE_SIZE	(16*1024)
int totalaccesstime = 0;
int totalaccesstimel2; //accestime for twolevel
int totalaccesstimel3; //accestime for twolevel


int numofmiss = 0, LRUmisses = 0, LFUmisses = 0; //num of misses used through the simuation, LRUmisses and LFU misses are used in adaptive replacement
int cache[1000000][2];

int cacheL1[10000000][2];
int cacheL2[10000000][2];
int cacheL3[10000000][2];
int setCache[1024][16];
int FirstIO[1024][16];

int *FACache; //declare a dynamic array to control the cache size
int *LRU; //declare a dynamic array that corresponds with the indices of the FACach to find the Least Recently Used
int *LFU; //declare a dynamic array that corresponds with the indices of the FACach to find the Least Frequently Used
int MRU; // save the index of mru

enum cacheResType { MISS = 0, HIT = 1 };

unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */

void resetarray()
{
	for (int x = 0; x < 10000000; x++)
	{
		cache[x][1] = 0;
	}
}

void resetarray2()
{
	for (int x = 0; x < 10000000; x++)
	{
		cacheL1[x][1] = 0;
	}

	for (int x = 0; x < 10000000; x++)
	{
		cacheL2[x][1] = 0;
	}
	totalaccesstimel2 = 0; //accestime for twolevel
	totalaccesstimel3 = 0;
}
void resetarray3()
{
	for (int x = 0; x < 10000000; x++)
	{
		cacheL1[x][1] = 0;
	}

	for (int x = 0; x < 10000000; x++)
	{
		cacheL2[x][1] = 0;
	}
	for (int x = 0; x < 10000000; x++)
	{
		cacheL3[x][1] = 0;
	}

	totalaccesstimel2 = 0; //accestime for twolevel
	totalaccesstimel3 = 0;
}

void FAreset(int x) {
	for (int i = 0; i < x; i++) //emptying the FAcache
	{
		FACache[i] = -1;
	}
}

void Replacementreset(int x) {
	for (int i = 0; i < x; i++) //reset the counters of the replacement arrays
	{
		LRU[i] = 0;
		LFU[i] = 0;
	}
}

unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}


unsigned int memGen1()
{
	static unsigned int addr = 0;
	return (addr += (16 * 1024)) % (256 * 1024);
}

unsigned int memGen2()
{
	static unsigned int addr = 0;
	return (addr += 4) % (DRAM_SIZE);
}

unsigned int memGen3()
{
	return rand_() % (256 * 1024);
}

int min(int a[], int n) //function to return the index of the min element in array (used in LRU replacement)
{
	int index = 0;
	for (int i = 1; i < n; i++)
		if (a[i] < a[index])
			index = i;
	return index;
}

int max(int a[], int n) //function to return the index of the max element in array (used in LFU replacement)
{
	int index = 0;
	for (int i = 1; i < n; i++)
		if (a[i] > a[index])
			index = i;
	return index;
}

bool checkSet(int indexa, int kways) {
	for (int i = 0; i < kways; i++) {
		if (FirstIO[indexa][i] == -1) {
			return false;
		}
	}
	return true;
}

int findFirstInd(int indexa, int kways) {
	int min = 20000000;
	for (int i = 0; i < kways; i++) {
		if (FirstIO[indexa][i] < min) {
			min = FirstIO[indexa][i];
		}
	}
	return min;
}

void resetSetCache(int numofsets, int kways) {
	for (int i = 0; i < numofsets; i++) {
		for (int j = 0; j < kways; j++) {
			setCache[i][j] = -1;
		}
	}
}

void resetFIFOarray(int numofsets, int kways) {
	for (int i = 0; i < numofsets; i++) {
		for (int j = 0; j < kways; j++) {
			FirstIO[i][j] = -1;
		}
	}
}

int lines = 0;
// Cache Simulator


cacheResType cacheSim(int cachetype, int indexa, int taga, int indexaL2, int tagaL2, int indexaL3, int tagaL3, int linesFA, int replacementType, int kways)
{
	srand(time(NULL));
	int accesstimel1 = 11; //first level cache and L2
	int accesstimel2 = 4; //L1 cycles
	int mainmemoryaccesstime = 100;

	if ((cachetype == 1) || (cachetype == 2))
	{
		if ((cache[indexa][0] == taga) && (cache[indexa][1] == 1)) //if same tag and valid bit =1
		{
			if (cachetype == 2)
			{
				totalaccesstime += 11;
			}
			
			return HIT;
		}
		else
		{
			cache[indexa][0] = taga;
			cache[indexa][1] = 1; //valid bit =1;

			if (cachetype == 1) //dm
				numofmiss++;
			else if (cachetype == 2) //dm
			{
				totalaccesstime = 11 + mainmemoryaccesstime + totalaccesstime;
			}
			
		}
	}

	else if (cachetype == 22) //twolevel simulator
	{
		if ((cacheL1[indexa][0] == taga) && (cacheL1[indexa][1] == 1)) //if same tag and valid bit =1
		{
			totalaccesstimel2 = 4 + totalaccesstimel2;
			return HIT;
		}
		else if ((cacheL2[indexaL2][0] == tagaL2) && (cacheL2[indexaL2][1] == 1))
		{


			cacheL1[indexa][0] = taga;
			cacheL1[indexa][1] = 1; //valid bit =1;

			totalaccesstimel2 = 4 + 11 + totalaccesstimel2;
		}
		else //main memory
		{


			cacheL2[indexaL2][0] = tagaL2;
			cacheL2[indexaL2][1] = 1; //valid bit =1;

			cacheL1[indexa][0] = taga;
			cacheL1[indexa][1] = 1; //valid bit =1;

			totalaccesstimel2 = 11 + 4 + mainmemoryaccesstime + totalaccesstimel2;
		}
	}

	else if (cachetype == 23) //threelevel simulator
	{
		if ((cacheL1[indexa][0] == taga) && (cacheL1[indexa][1] == 1)) //if same tag and valid bit =1
		{
			totalaccesstimel3 = 4 + totalaccesstimel3;
			return HIT;
		}
		else if ((cacheL2[indexaL2][0] == tagaL2) && (cacheL2[indexaL2][1] == 1))
		{
			cacheL1[indexa][0] = taga;
			cacheL1[indexa][1] = 1; //valid bit =1;

			totalaccesstimel3 = 4 + 11 + totalaccesstimel3;
		}
		else if ((cacheL3[indexaL3][0] == tagaL3) && (cacheL3[indexaL3][1] == 1))
		{
			cacheL2[indexaL2][0] = tagaL2;
			cacheL2[indexaL2][1] = 1; //valid bit =1;

			cacheL1[indexa][0] = taga;
			cacheL1[indexa][1] = 1; //valid bit =1;

			totalaccesstimel3 = 4 + 11 + 30 + totalaccesstimel3;
		}
		else //main memory
		{
			cacheL3[indexaL3][0] = tagaL3;
			cacheL3[indexaL3][1] = 1; //valid bit =1;

			cacheL2[indexaL2][0] = tagaL2;
			cacheL2[indexaL2][1] = 1; //valid bit =1;

			cacheL1[indexa][0] = taga;
			cacheL1[indexa][1] = 1; //valid bit =1;

			totalaccesstimel3 = 4 + 11 + 30 + mainmemoryaccesstime + totalaccesstimel3;
		}
	}

	else if (cachetype == 3)
	{
		for (int i = 0; i < linesFA; i++)
		{
			if (FACache[i] == taga)
			{
				MRU = i;
				return HIT;
			}
			else if (FACache[i] == -1)
			{
				MRU = i;
				numofmiss++;
				FACache[i] = taga;
				return MISS;
			}
		}
		numofmiss++;
		FACache[MRU] = taga;
	}
	else if (cachetype == 4)
	{
		if (replacementType == 1)
		{
			for (int i = 0; i < linesFA; i++)
			{
				if (FACache[i] == taga)
				{
					return HIT;
				}
				else if (FACache[i] == -1)
				{
					numofmiss++;
					FACache[i] = taga;
					return MISS;
				}
			}
			numofmiss++;
			FACache[rand() % linesFA] = taga;
		}
		if (replacementType == 2)
		{
			for (int i = 0; i < linesFA; i++)
			{
				LRU[i]++;
				if (FACache[i] == taga)
				{
					LRU[i]--;
					return HIT;
				}
				else if (FACache[i] == -1)
				{
					numofmiss++;
					LRUmisses++;
					LRU[i]--;
					FACache[i] = taga;
					return MISS;
				}
			}
			numofmiss++;
			LRUmisses++;
			FACache[max(LRU, linesFA)] = taga;
		}
		if (replacementType == 3)// when a block is accessed its lfu counter is incremented, then when we need to replace the lfu we just need to replace the min of the lfu counters array
		{
			for (int i = 0; i < linesFA; i++)
			{
				if (FACache[i] == taga)
				{
					LFU[i]++;
					return HIT;
				}
				else if (FACache[i] == -1)
				{
					numofmiss++;
					LFUmisses++;
					LFU[i]++;
					FACache[i] = taga;
					return MISS;
				}
			}
			numofmiss++;
			LFUmisses++;
			FACache[min(LFU, linesFA)] = taga;
		}
	}
	else if (cachetype == 5)
	{
		bool setIsFull;
		for (int i = 0; i < kways; i++) {
			if (setCache[indexa][i] == taga) {
				return HIT;
			}
		}
		setIsFull = checkSet(indexa, kways);
		if (setIsFull) {
			int FOindex = (findFirstInd(indexa, kways)) % kways;
			FirstIO[indexa][FOindex] += kways;
			setCache[indexa][FOindex] = taga;
		}
		else {
			for (int i = 0; i < kways; i++) {
				if (FirstIO[indexa][i] == -1) {
					FirstIO[indexa][i] = i;
					setCache[indexa][i] = taga;
					return MISS;
				}
			}
		}
		return MISS;
	}
	return MISS;
}

int main()
{
	srand(time(NULL));
	int cacheType;
	int iter;
	cacheResType r;
	const char *msg[4] = { "Random","LRU", "LFU" };

	unsigned int addr;
	cout << "Enter: 1 -> Exp 1, 2 -> Exp 2, 3 -> Exp 3, 4 -> Exp 4, 5 -> Exp 5" << endl;
	cin >> cacheType;

	int blocksize, cachesize;
	int offsitbits;
	int indexbits;
	int setbits;
	int indexadrr;
	int tagadrr;
	int valid;
	int kways;
	int numofsets;

	if (cacheType == 1) //DM
	{
		cout << "Experiment 1: " << endl;
		//for loop
		for (blocksize = 4; blocksize <= 128; blocksize = blocksize * 2)
		{
			resetarray();

			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();

				lines = CACHE_SIZE / blocksize;

				offsitbits = log2(blocksize);
				indexbits = log2(lines);
				indexadrr = (addr >> offsitbits) % lines;
				tagadrr = (addr >> (offsitbits + indexbits));

				cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);

			}
			float missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 1 with block size " << blocksize << " : " << 1 - missratio << endl;

		}
		cout << endl;

		for (blocksize = 4; blocksize <= 128; blocksize = blocksize * 2)
		{

			resetarray();


			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();

				lines = CACHE_SIZE / blocksize;

				offsitbits = log2(blocksize);
				indexbits = log2(lines);
				indexadrr = (addr >> offsitbits) % lines;
				tagadrr = (addr >> (offsitbits + indexbits));

				cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);

			}

			float missratio = numofmiss * 1.0 / 1000000;

			cout << "Hit Ratio with memGen 2 with block size " << blocksize << " : " << 1 - missratio << endl;
		}
		cout << endl;

		for (blocksize = 4; blocksize <= 128; blocksize = blocksize * 2)
		{
			resetarray();

			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();
				lines = CACHE_SIZE / blocksize;

				offsitbits = log2(blocksize);
				indexbits = log2(lines);
				indexadrr = (addr >> offsitbits) % lines;
				tagadrr = (addr >> (offsitbits + indexbits));

				

				cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);
				
				

			}
			float missratio = (numofmiss * 1.0) / 1000000;

			cout << "Hit Ratio with memGen 1 with block size " << blocksize << " : " << 1 - missratio << endl;
		}
	}

	else if (cacheType == 2)
	{
		cout << "Experiment 2: " << endl;
		{
			{
				resetarray();
				blocksize = 64;
				int cahcesizel1 = CACHE_SIZE * 4; //256 BLOCKSIZE
				lines = cahcesizel1 / blocksize;

				for (iter = 0; iter < 1000000; iter++)
				{
					addr = memGen1();
					offsitbits = log2(blocksize);
					indexbits = log2(lines);
					indexadrr = (addr >> offsitbits) % lines;
					tagadrr = (addr >> (offsitbits + indexbits));
					cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);
				}
				cout << "AMAT for a 1-level cache with memGen1(): " << totalaccesstime << endl;
				totalaccesstime = 0;
				resetarray();
				for (iter = 0; iter < 1000000; iter++)
				{
					addr = memGen2();
					offsitbits = log2(blocksize);
						indexbits = log2(lines);
					indexadrr = (addr >> offsitbits) % lines;
					tagadrr = (addr >> (offsitbits + indexbits));
					cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);
				}
				cout << "AMAT for a 1-level cache with memGen2(): " << totalaccesstime << endl;
			}

			{
				totalaccesstime = 0;
				resetarray();
				for (iter = 0; iter < 1000000; iter++)
				{
					addr = memGen3();
					blocksize = 64;
					int cahcesizel1 = CACHE_SIZE * 4; //256 CacheSize
					lines = cahcesizel1 / blocksize;

					offsitbits = log2(blocksize);
					indexbits = log2(lines);
					indexadrr = (addr >> offsitbits) % lines;
					tagadrr = (addr >> (offsitbits + indexbits));

					cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, 0);
				}
				cout << "AMAT for a 1-level cache with memGen3(): " << totalaccesstime << endl;
			}
			cout << endl;
		}

		{//two level
			int offsitbitsL1;
			int offsitbitsL2;
			int indexL1;
			int indexL2;
			int indexadrrL1;
			int indexadrrL2;
			int tagadrrL1;
			int tagadrrL2;

			totalaccesstimel2 = 0;
			resetarray2();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //32 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				cacheSim(22, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, 0, 0, 0, 0, 0);
			}
			cout << "AMAT for a 2-level cache with memGen1(): " << totalaccesstimel2 << endl;

			totalaccesstimel2 = 0;
			resetarray2();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //32 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				cacheSim(22, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, 0, 0, 0, 0, 0);
			}
			cout << "AMAT for a 2-level cache with memGen2(): " << totalaccesstimel2 << endl;

			totalaccesstimel2 = 0;
			resetarray2();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //32 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				cacheSim(22, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, 0, 0, 0, 0, 0);
			}
			cout << "AMAT for a 2-level cache with memGen3(): " << totalaccesstimel2 << endl;
			cout << endl;
		}

		{//threelevel
			int offsitbitsL1;
			int offsitbitsL2;
			int indexL1;
			int indexL2;
			int indexadrrL1;
			int indexadrrL2;
			int tagadrrL1;
			int tagadrrL2;
			int offsitbitsL3;
			int indexL3;
			int indexadrrL3;
			int tagadrrL3;

			totalaccesstimel3 = 0;
			resetarray3();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //232 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				int cahcesizeL3 = CACHE_SIZE * 128; //8192 BLOCKSIZE
				int linesL3 = cahcesizeL3 / blocksize;

				offsitbitsL3 = log2(blocksize);
				indexL3 = log2(linesL3);
				indexadrrL3 = (addr >> offsitbitsL3) % linesL3;
				tagadrrL3 = (addr >> (offsitbitsL3 + indexL3));

				cacheSim(23, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, indexadrrL3, tagadrrL3, 0, 0, 0);
			}
			cout << "AMAT for a 3-level cache with memGen1(): " << totalaccesstimel3 << endl;

			totalaccesstimel3 = 0;
			resetarray3();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //32 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				int cahcesizeL3 = CACHE_SIZE * 128; //8192 BLOCKSIZE
				int linesL3 = cahcesizeL3 / blocksize;

				offsitbitsL3 = log2(blocksize);
				indexL3 = log2(linesL3);
				indexadrrL3 = (addr >> offsitbitsL3) % linesL3;
				tagadrrL3 = (addr >> (offsitbitsL3 + indexL3));

				cacheSim(23, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, indexadrrL3, tagadrrL3, 0, 0, 0);
			}
			cout << "AMAT for a 3-level cache with memGen2(): " << totalaccesstimel3 << endl;

			totalaccesstimel3 = 0;
			resetarray3();
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();
				blocksize = 64;
				int cahcesizeL1 = CACHE_SIZE / 2; //232 BLOCKSIZE
				int linesL1 = cahcesizeL1 / blocksize;

				offsitbitsL1 = log2(blocksize);
				indexL1 = log2(linesL1);
				indexadrrL1 = (addr >> offsitbitsL1) % linesL1;
				tagadrrL1 = (addr >> (offsitbitsL1 + indexL1));

				int cahcesizeL2 = CACHE_SIZE * 4; //256 BLOCKSIZE
				int linesL2 = cahcesizeL2 / blocksize;

				offsitbitsL2 = log2(blocksize);
				indexL2 = log2(linesL2);
				indexadrrL2 = (addr >> offsitbitsL2) % linesL2;
				tagadrrL2 = (addr >> (offsitbitsL2 + indexL2));

				int cahcesizeL3 = CACHE_SIZE * 128; //8192 BLOCKSIZE
				int linesL3 = cahcesizeL3 / blocksize;

				offsitbitsL3 = log2(blocksize);
				indexL3 = log2(linesL3);
				indexadrrL3 = (addr >> offsitbitsL3) % linesL3;
				tagadrrL3 = (addr >> (offsitbitsL3 + indexL3));

				cacheSim(23, indexadrrL1, tagadrrL1, indexadrrL2, tagadrrL2, indexadrrL3, tagadrrL3, 0, 0, 0);
			}
			cout << "AMAT for a 3-level cache with memGen3(): " << totalaccesstimel3 << endl;

			cout << endl;
		}
	}

	else if (cacheType == 3) //FA Cache 
	{
		int constblocksize = 16;

		cout << "Experiment 3 (Fully Associative Cache): " << endl;
		//for loop
		for (int i = 4; i <= 64; i *= 2)
		{
			cout << "For Cache Size = " << i << "KB:" << endl;
			cachesize = i * 1024;
			lines = cachesize / constblocksize;
			FACache = new int[lines];

			FAreset(lines);
			numofmiss = 0;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();

				tagadrr = (addr >> 4);

				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, 0, 0);
			}
			float missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 1: " << 1 - missratio << endl;

			FAreset(lines);
			numofmiss = 0;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();

				tagadrr = (addr >> 4);

				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, 0, 0);
			}
			missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 2: " << 1 - missratio << endl;

			FAreset(lines);
			numofmiss = 0;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();

				tagadrr = (addr >> 4);

				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, 0, 0);
			}
			missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 3: " << 1 - missratio << endl;
		}
		cout << endl;
	}

	else if (cacheType == 4)
	{
		int constblocksize = 32; //block size in exp 4 = 32 bytes
		cachesize = 4 * CACHE_SIZE;
		lines = cachesize / constblocksize;
		FACache = new int[lines];
		LRU = new int[lines];
		LFU = new int[lines];
		cout << "Experiment 4: " << endl;


		for (int i = 1; i <= 3; i++)
		{
			FAreset(lines);
			Replacementreset(lines);
			numofmiss = 0;

			cout << "Using " << msg[i - 1] << " Replacement Policy: " << endl;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();
				tagadrr = (addr >> 5); //remove the byte select bits (5 cuz there is 32 bytes) to get the tag
				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, i, 0);//call cachSim with the cache type, the tag, and num of lines in the FAcache, replacement policy choice
			}

			float missratio = numofmiss * 1.0 / 1000000;// calculate miss ratio
			cout << "Hit Ratio with memGen1(): " << 1 - missratio << endl; // output hit ratio (1-missratio)

			FAreset(lines);
			Replacementreset(lines);
			numofmiss = 0;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();
				tagadrr = (addr >> 5);
				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, i, 0);
			}
			missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 2: " << 1 - missratio << endl;

			FAreset(lines);
			Replacementreset(lines);
			numofmiss = 0;
			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();
				tagadrr = (addr >> 5);
				cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, i, 0);
			}
			missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 3: " << 1 - missratio << endl;
		}

		FAreset(lines);
		Replacementreset(lines);
		numofmiss = 0;
		cout << "Using Adaptive Replacement Policy: " << endl;
		int adaptive = 2;
		for (iter = 0; iter < 1000000; iter++)
		{
			addr = memGen1();
			tagadrr = (addr >> 5);
			cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, adaptive, 0); //u need to replace this with work on casheSim itself (u can't use numofmisses)
			if ((LRUmisses > 10000) && (adaptive == 2))
			{
				adaptive = 3;
				LRUmisses = 0;
			}
			if ((LFUmisses > 10000) && (adaptive == 3))
			{
				adaptive = 2;
				LFUmisses = 0;
			}
		}
		float missratio = numofmiss * 1.0 / 1000000;
		cout << "Hit Ratio with memGen 1: " << 1 - missratio << endl;

		FAreset(lines);
		Replacementreset(lines);
		numofmiss = 0;
		LFUmisses = 0;
		LRUmisses = 0;
		for (iter = 0; iter < 1000000; iter++)
		{
			addr = memGen2();
			tagadrr = (addr >> 5);
			cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, adaptive, 0); //u need to replace this with work on casheSim itself (u can't use numofmisses)
			if ((LRUmisses > 10000) && (adaptive == 2))
			{
				adaptive = 3;
				LRUmisses = 0;
			}
			if ((LFUmisses > 10000) && (adaptive == 3))
			{
				adaptive = 2;
				LFUmisses = 0;
			}
		}
		missratio = numofmiss * 1.0 / 1000000;
		cout << "Hit Ratio with memGen 2: " << 1 - missratio << endl;

		FAreset(lines);
		Replacementreset(lines);
		numofmiss = 0;
		LRUmisses = 0;
		LFUmisses = 0;
		for (iter = 0; iter < 1000000; iter++)
		{
			addr = memGen3();
			tagadrr = (addr >> 5);
			cacheSim(cacheType, 0, tagadrr, 0, 0, 0, 0, lines, adaptive, 0); //u need to replace this with work on casheSim itself (u can't use numofmisses)
			if ((LRUmisses > 10000) && (adaptive == 2))
			{
				adaptive = 3;
				LRUmisses = 0;
			}
			if ((LFUmisses > 10000) && (adaptive == 3))
			{
				adaptive = 2;
				LFUmisses = 0;
			}
		}
		missratio = numofmiss * 1.0 / 1000000;
		cout << "Hit Ratio with memGen 3: " << 1 - missratio << endl;
		cout << endl;
	}

	else if (cacheType == 5)
	{
		cout << "Experiment 5:\n";
		for (kways = 2; kways <= 16; kways = kways * 2)
		{
			blocksize = 8;
			numofsets = SETCACHE_SIZE / (kways * blocksize);
			resetSetCache(numofsets, kways);
			resetFIFOarray(numofsets, kways);
			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen1();
				cacheResType check;

				offsitbits = log2(blocksize);
				setbits = log2(numofsets);
				indexadrr = (addr >> offsitbits) % numofsets;
				tagadrr = (addr >> (offsitbits + setbits));

				check = cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, kways);
				if (check == MISS) {
					numofmiss++;
				}
			}
			float missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 1 with " << kways << "-ways set assoicative cache: " << 1 - missratio << endl;
		}
		cout << endl;

		for (kways = 2; kways <= 16; kways = kways * 2)
		{

			blocksize = 8;
			numofsets = SETCACHE_SIZE / (kways * blocksize);
			resetSetCache(numofsets, kways);
			resetFIFOarray(numofsets, kways);
			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen2();
				cacheResType check;

				offsitbits = log2(blocksize);
				setbits = log2(numofsets);
				indexadrr = (addr >> offsitbits) % numofsets;
				tagadrr = (addr >> (offsitbits + setbits));

				check = cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, kways);
				if (check == MISS) {
					numofmiss++;
				}
			}
			float missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 2 with " << kways << "-ways set assoicative cache: " << 1 - missratio << endl;
		}
		cout << endl;

		for (kways = 2; kways <= 16; kways = kways * 2)
		{
			blocksize = 8;
			numofsets = SETCACHE_SIZE / (kways * blocksize);
			resetSetCache(numofsets, kways);
			resetFIFOarray(numofsets, kways);
			numofmiss = 0;
			//clear valid bits for cache array


			for (iter = 0; iter < 1000000; iter++)
			{
				addr = memGen3();
				cacheResType check;

				offsitbits = log2(blocksize);
				setbits = log2(numofsets);
				indexadrr = (addr >> offsitbits) % numofsets;
				tagadrr = (addr >> (offsitbits + setbits));

				check = cacheSim(cacheType, indexadrr, tagadrr, 0, 0, 0, 0, 0, 0, kways);
				if (check == MISS) {
					numofmiss++;
				}
			}
			float missratio = numofmiss * 1.0 / 1000000;
			cout << "Hit Ratio with memGen 3 with " << kways << "-ways set assoicative cache: " << 1 - missratio << endl;
		}
	}


	system("pause");
}
