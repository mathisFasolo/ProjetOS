#ifndef FAT_H_
#define FAT_H_

//FAT valeurs utiles
#define END_CLUSTER_32 0x0FFFFFF8
#define BAD_CLUSTER_32 0x0FFFFFF7
#define FREE_CLUSTER_32 0x00000000

#define FILE_READ_ONLY 0x01
#define FILE_HIDDEN 0x02
#define FILE_SYSTEM 0x04
#define FILE_VOLUME_ID 0x08
#define FILE_DIRECTORY 0x10
#define FILE_LONG_NAME (FILE_READ_ONLY|FILE_HIDDEN|FILE_SYSTEM|FILE_VOLUME_ID)
#define ENTRY_FREE 0xE5
#define ENTRY_END 0x00

#define LOWERCASE_ISSUE	0x01
#define BAD_CHARACTER	0x02
#define NOT_CONVERTED_YET 0x08
#define TOO_MANY_DOTS 0x10

#define GET_CLUSTER_FROM_ENTRY(x) (x.low_bits | (x.high_bits << (fatType / 2)))
#define GET_ENTRY_LOW_BITS(x) (x & ((fatType /2) -1))
#define GET_ENTRY_HIGH_BITS(x) (x >> (fatType / 2))

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL short
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef DISK_READ_LOCATION
#define DISK_READ_LOCATION 0x40000
#endif
#ifndef DISK_WRITE_LOCATION
#define DISK_WRITE_LOCATION 0x40000
#endif


//Fonctions fournis par l'OS permettant la lecture du boot média
//fonction de type read_int13h
extern int LectureBootMedia(unsigned long sector, unsigned char num);
extern int LectureBootMediaAvecOffset(unsigned long sector, unsigned char num, unsigned long memoffset);
extern int EcritureBootMedia(unsigned long sector, unsigned char num);
extern int EcritureBootMediaAvecOffset(unsigned long sector, unsigned char num, unsigned long memoffset);
//Fonction fournis par la partie affichage de l'OS
extern void AfficheText(int charnum);



//FAT directory and bootsector structures
typedef struct FATBs32
{
    //extended fat32 stuff
    unsigned int		table_size_32;
    unsigned short		extended_flags;
    unsigned short		fat_version;
    unsigned int		root_cluster;
    unsigned short		fat_info;
    unsigned short		backup_BS_sector;
    unsigned char 		reserved_0[12];
    unsigned char		drive_number;
    unsigned char 		reserved_1;
    unsigned char		boot_signature;
    unsigned int 		volume_id;
    unsigned char		volume_label[11];
    unsigned char		fat_type_label[8];

}
    __attribute__((packed))
        FATBs32;

typedef struct FATBs16
{
    //extended fat12 and fat16 stuff
    unsigned char		bios_drive_num;
    unsigned char		reserved1;
    unsigned char		boot_signature;
    unsigned int		volume_id;
    unsigned char		volume_label[11];
    unsigned char		fat_type_label[8];

}
    __attribute__((packed))
        FATBS16;

typedef struct FATBs
{
    unsigned char 		bootjmp[3];
    unsigned char 		oem_name[8];
    unsigned short 	    bytes_per_sector;
    unsigned char		sectors_per_cluster;
    unsigned short		reserved_sector_count;
    unsigned char		table_count;
    unsigned short		root_entry_count;
    unsigned short		total_sectors_16;
    unsigned char		media_type;
    unsigned short		table_size_16;
    unsigned short		sectors_per_track;
    unsigned short		head_side_count;
    unsigned int 		hidden_sector_count;
    unsigned int 		total_sectors_32;
    unsigned char		extended_section[54];

}
    __attribute__((packed))
        FATBs;


typedef struct DirEntry
{
    unsigned char file_name[11];
    unsigned char attributes;
    unsigned char reserved0;
    unsigned char creation_time_tenths;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short last_accessed;
    unsigned short high_bits;
    unsigned short last_modification_time;
    unsigned short last_modification_date;
    unsigned short low_bits;
    unsigned int file_size;
}
    __attribute__((packed))
        dirEntry;

//Global variables
extern unsigned int fatType;
extern unsigned int FirstFatSector;
extern unsigned int FirstDataSector;
extern unsigned int TotCluster;
extern FATBs BootSector;

int InitialisationFAT();
int ReadFAT(unsigned int clusterNum);
int WriteFAT(unsigned int clusterNum, unsigned int clusterVal);
unsigned int FreeFATAllocation();
int LectureCluster(unsigned int clusterNum, unsigned int clusterOffset);
int EcritureCluster(void* contentsToWrite, unsigned int contentSize, unsigned int contentBuffOffset, unsigned int clusterNum);
int ListeDossier(const unsigned int cluster, unsigned char attributesToAdd, short exclusive);
int FindDossier(const char* filepart, const unsigned int cluster, dirEntry* file, unsigned int* entryOffset);
int AjoutDossier(const unsigned int cluster, dirEntry* file_to_add);
int getFile(const char* filePath, char** fileContents, dirEntry* fileMeta, unsigned int readInOffset);
int putFile(const char* filePath, char** fileContents, dirEntry* fileMeta);
unsigned short CurrentTime();
unsigned char CurrentTimeTenths();
unsigned short CurrentDate();
unsigned char CheckSum(unsigned char *pFcbName);
BOOL TestFormat(char * input);
char* ToFATFormat(char* input);
void FromFATFormat(char* input, char* output);

void printHexa( unsigned long num, int digits );
void printAllString( char *s );
void printString( char *s, int n );


#endif