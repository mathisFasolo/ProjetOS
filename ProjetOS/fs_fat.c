#include <memory.h>
#include "fs_fat.h"

//Global variables
unsigned int fatType;
unsigned int FirsFatSector;
unsigned int FirstDataSector;
unsigned int TotCluster;
FATBs BootSector;

//Initialise la struct bootsector
int InitialisationFAT() {

    if (LectureBootMedia(0, 1) != 0) {
        printAllString("Can't read first sector of FAT\n");
        return -1;
    }

    FATBs *bootstruct = (FATBs *) DISK_READ_LOCATION;

    TotCluster = bootstruct->total_sectors_16 / bootstruct->sectors_per_cluster;

    if (TotCluster == 0) {
        TotCluster = bootstruct->total_sectors_32 / bootstruct->sectors_per_cluster;
    }

    if (TotCluster < 4085) {
        fatType = 12;

        FirstDataSector = bootstruct->reserved_sector_count + bootstruct->table_count * bootstruct->table_size_16 +
                            (bootstruct->root_entry_count * 32 + bootstruct->bytes_per_sector - 1) /
                            bootstruct->bytes_per_sector;
    } else {
        if (TotCluster < 65525) {
            fatType = 16;
            FirstDataSector =
                    bootstruct->reserved_sector_count + bootstruct->table_count * bootstruct->table_size_16 +
                    (bootstruct->root_entry_count * 32 + bootstruct->bytes_per_sector - 1) /
                    bootstruct->bytes_per_sector;
        } else {
            fatType = 32;
            FirstDataSector = bootstruct->reserved_sector_count +
                                bootstruct->table_count * ((FATBs32 *) (bootstruct->extended_section))->table_size_32;
        }
    }

    memcpy(&BootSector, bootstruct, 512);

    FirsFatSector = bootstruct->reserved_sector_count;

    return 0;
}

//Lecture de la table FAT du media
int ReadFAT(unsigned int clustNumber) {
    if (clustNumber < 2 || clustNumber >= TotCluster) {
        printAllString("Invalid number of cluster ReadFAT function\n");
        return -1;
    }

    if (fatType == 32) {
        unsigned int clusterSize = BootSector.bytes_per_sector * BootSector.sectors_per_cluster;
        unsigned char FATTable[32 * 1024] = {'\0'};
        unsigned int FAToffset = clustNumber * 4;
        unsigned int FATsector = FirsFatSector + (FAToffset / clusterSize);
        unsigned int offset = FAToffset % clusterSize;
        if (LectureBootMedia(FATsector, 1) != 0) {
            printAllString("can't read sector of FAT32\n");
            return -1;
        }
        memcpy(&FATTable, (char *) DISK_READ_LOCATION, BootSector.bytes_per_sector);
        unsigned int table_value = *(unsigned int *) &FATTable[offset] & 0x0FFFFFFF;
        return table_value;
    }
    else {
        printAllString("Invalid FAT Type : FAT32 only supported");
        printHexa(fatType, 8);
        printAllString("\n");
        return -1;
    }
}

int WriteFAT(unsigned int clustNumber, unsigned int clusterVal) {
    if (clustNumber < 2 || clustNumber >= TotCluster) {
        printAllString("Invalid cluster number\n");
        return -1;
    }

    if (fatType == 32) {
        unsigned int clusterSize = BootSector.bytes_per_sector * BootSector.sectors_per_cluster;
        unsigned char FATTable[32 * 1024] = {'\0'};
        unsigned int FAToffset = clustNumber * 4;
        unsigned int FATsector = FirsFatSector + (FAToffset / clusterSize);
        unsigned int offset = FAToffset % clusterSize;
        if (LectureBootMedia(FATsector, 1) != 0) {
            printAllString("Can't read FAT32 sector\n");
            return -1;
        }
        memcpy(&FATTable, (char *) DISK_READ_LOCATION, BootSector.bytes_per_sector);
        *(unsigned int *) &FATTable[offset] = clusterVal;
        memcpy((char *) DISK_WRITE_LOCATION, &FATTable, BootSector.bytes_per_sector);

        if (EcritureBootMedia(FATsector, 1) != 0) {
            printAllString("Failed to write new FAT32 entry\n");
            return -1;
        }
        return 0;
    }
    else {
        printAllString("Invalid FAT Type : FAT32 only supported");
        printHexa(fatType, 8);
        printAllString("\n");
        return -1;
    }
}

unsigned int FreeFATAllocation() {
    unsigned int freeClust;
    unsigned int badClust;
    unsigned int endClust;
    if (fatType == 32) {
        freeClust = FREE_CLUSTER_32;
        badClust = BAD_CLUSTER_32;
        endClust = END_CLUSTER_32;
        printAllString("Invalid FAT Type : FAT32 only supported\n");
        return 0x0FFFFFF7;
    }

    unsigned int cluster = 2;
    unsigned int clusterStatus = freeClust;
    while (cluster < TotCluster) {
        clusterStatus = ReadFAT(cluster);

        if (clusterStatus == freeClust) {
            if (WriteFAT(cluster, endClust) == 0)
                return cluster;
            else {
                printAllString("Error with WriteFAT\n");
                return badClust;
            }
        } else if (clusterStatus < 0) {
            printAllString("Error during ReadFAT\n");
            return badClust;
        }
        cluster++;
    }

    return badClust;
}

int LectureCluster(unsigned int clustNumber, unsigned int clusterOffset) {
    if (clustNumber < 2 || clustNumber >= TotCluster) {
        printAllString("Invalid cluster number\n");
        return -1;
    }
    unsigned int startSector = (clustNumber - 2) * (unsigned short) BootSector.sectors_per_cluster + FirstDataSector;
    if (LectureBootMediaAvecOffset(startSector, (unsigned short) BootSector.sectors_per_cluster, clusterOffset * (unsigned short) BootSector.sectors_per_cluster * (unsigned short) BootSector.bytes_per_sector) != 0) {
        printAllString("Error");
        printHexa(clusterOffset, 8);
        printAllString("Unknow states\n");
        return -1;
    } else
        return 0;
}

int EcritureCluster(void *contentsToWrite, unsigned int contentSize, unsigned int contentBuffOffset,
                    unsigned int clustNumber) {
    if (clustNumber < 2 || clustNumber >= TotCluster) {
        printAllString("Invalid cluster type\n");
        return -1;
    }
    unsigned int byteOffset = contentBuffOffset * (unsigned short) BootSector.sectors_per_cluster * (unsigned short) BootSector.bytes_per_sector;
    memcpy((char *) DISK_WRITE_LOCATION + byteOffset, contentsToWrite, contentSize);
    unsigned int startSector = (clustNumber - 2) * (unsigned short) BootSector.sectors_per_cluster + FirstDataSector;
    if (EcritureBootMediaAvecOffset(startSector, (unsigned short) BootSector.sectors_per_cluster, byteOffset) != 0) {
        printAllString("Error");
        printHexa(startSector, 8);
        printHexa(((unsigned short) BootSector.sectors_per_cluster) + startSector, 2);
        printAllString("Unknow state\n");
        return -1;
    } else
        return 0;
}

int ListeDossier(const unsigned int cluster, unsigned char attributesToAdd, BOOL exclusive) {
    if (cluster < 2 || cluster >= TotCluster) {
        printAllString("Invalid cluster number\n");
        return -1;
    }

    const unsigned char default_hidden_attributes = (FILE_HIDDEN | FILE_SYSTEM);

    unsigned char attributes_to_hide = default_hidden_attributes;

    if (exclusive == FALSE)
        attributes_to_hide &= (~attributesToAdd);
    else if (exclusive == TRUE)
        attributes_to_hide = (~attributesToAdd);
    if (LectureCluster(cluster, 0) != 0) {
        printAllString("Error with LectureCluster\n");
        return -1;
    }
    dirEntry *fMetadata = (dirEntry *) DISK_READ_LOCATION;
    unsigned int metaIterato = 0;
    while (1) {
        if (fMetadata->file_name[0] == ENTRY_END)
        {
            break;
        } else if (strncmp(fMetadata->file_name, "..", 2) == 0 || strncmp(fMetadata->file_name, ".", 1) == 0) {
            if (fMetadata->file_name[1] == '.')
                printAllString("..");
            else
                printAllString(".");
            printAllString("\t");
            printAllString("\t");
            printAllString("\t");
            printAllString("DIR");
            printAllString("\n");
            fMetadata++;
            metaIterato++;
        } else if (((fMetadata->file_name)[0] == ENTRY_FREE) ||
                   ((fMetadata->attributes & FILE_LONG_NAME) == FILE_LONG_NAME) ||
                   ((fMetadata->attributes & attributes_to_hide) != 0)){
            if (metaIterato < BootSector.bytes_per_sector * BootSector.sectors_per_cluster / sizeof(dirEntry) - 1){
                fMetadata++;
                metaIterato++;
            } else
            {
                unsigned int nextClust = ReadFAT(cluster);

                if ((nextClust >= END_CLUSTER_32 && fatType == 32))
                    break;
                else if (nextClust < 0) {
                    printAllString("Failed with ReadFAT\n");
                    return -1;
                } else
                    return ListeDossier(nextClust, attributesToAdd, exclusive);
            }
        } else {
            char conversion[13];
            FromFATFormat((char *) fMetadata->file_name, conversion);
            printAllString(conversion);
            printAllString("\t");
            if ((fMetadata->attributes & FILE_DIRECTORY) != FILE_DIRECTORY)
                printHexa(fMetadata->file_size, 8);
            else
                printAllString("\t");
            printAllString("\t");
            if ((fMetadata->attributes & FILE_DIRECTORY) == FILE_DIRECTORY) {
                printAllString("DIR");
            }

            printAllString("\n");

            fMetadata++;
            metaIterato++;
        }
    }

    return 0;
}

int FindDossier(const char *filepart, const unsigned int cluster, dirEntry *file, unsigned int *entryOffset) {
    if (cluster < 2 || cluster >= TotCluster) {
        printAllString("Invalid cluster\n");
        return -1;
    }

    char searchName[13] = {'\0'};
    strcpy(searchName, filepart);
    if (TestForma(searchName) != 0)
        ToFATFormat(searchName);
    if (LectureCluster(cluster, 0) != 0) {
        printAllString("Failed with LectureCluster.\n");
        return -1;
    }
    dirEntry *fMetadata = (dirEntry *) DISK_READ_LOCATION;
    unsigned int metaIterato = 0;
    while (1) {
        if (fMetadata->file_name[0] == ENTRY_END)
            break;
        else if (strncmp((char *) fMetadata->file_name, searchName, 11) != 0)
        {
            if (metaIterato <
                BootSector.bytes_per_sector * BootSector.sectors_per_cluster / sizeof(dirEntry) - 1) {
                fMetadata++;
                metaIterato++;
            } else {
                int nextClust = ReadFAT(cluster);

                if ((nextClust >= END_CLUSTER_32 && fatType == 32))
                    break;
                else if (nextClust < 0) {
                    printAllString("Error with ReadFAT\n");
                    return -1;
                } else
                    return FindDossier(filepart, nextClust, file, entryOffset);
            }
        } else
        {
            if (file != NULL) {
                memcpy(file, fMetadata, sizeof(dirEntry));
            }

            if (entryOffset != NULL)
                *entryOffset = metaIterato;

            return 0;
        }
    }

    return -2;
}


int AjoutDossier(const unsigned int cluster, dirEntry *fileToAdd) {
    if (TestForma(fileToAdd->file_name) != 0) {
        printAllString("File Name Invalid");
        return -1;
    }

    if (LectureCluster(cluster, 0) != 0) {
        printAllString("Error with LectureCluster\n");
        return -1;
    }
    dirEntry *fMetadata = (dirEntry *) DISK_READ_LOCATION;
    unsigned int metaIterato = 0;
    while (1) {
        if (fMetadata->file_name[0] != ENTRY_FREE &&
            fMetadata->file_name[0] != ENTRY_END)
        {
            if (metaIterato <
                BootSector.bytes_per_sector * BootSector.sectors_per_cluster / sizeof(dirEntry) - 1) {
                fMetadata++;
                metaIterato++;
            } else {
                unsigned int nextClust = ReadFAT(cluster);
                printHexa(nextClust, 8);
                printAllString("\n");

                if ((nextClust >= END_CLUSTER_32 && fatType == 32)) {
                    nextClust = FreeFATAllocation();

                    if ((nextClust == BAD_CLUSTER_32 && fatType == 32))
                    {
                        printAllString("Failed to allocate new cluster\n");
                        return -1;
                    }
                    if (WriteFAT(cluster, nextClust) != 0) {
                        printAllString(
                                "Failed to extend\n");
                        return -1;
                    }
                }

                return AjoutDossier(nextClust, fileToAdd);
            }
        } else {
            unsigned short dot_checker = 0;
            for (dot_checker = 0; dot_checker < 11; dot_checker++) {
                if (fileToAdd->file_name[dot_checker] == '.') {
                    printAllString("Invalid file name");
                    return -1;
                }
            }
            fileToAdd->creation_date = CurrentDate();
            fileToAdd->creation_time = CurrentTime();
            fileToAdd->creation_time_tenths = CurrentTimeTenths();
            fileToAdd->last_accessed = fileToAdd->creation_date;
            fileToAdd->last_modification_date = fileToAdd->creation_date;
            fileToAdd->last_modification_time = fileToAdd->creation_time;

            unsigned int nCluster = FreeFATAllocation();
            printHexa(nCluster, 8);
            printAllString("\n");

            if ((nCluster == BAD_CLUSTER_32 && fatType == 32))
            {
                printAllString("Failed to allocate new cluster\n");
                return -1;
            }

            fileToAdd->low_bits = GET_ENTRY_LOW_BITS(nCluster);
            fileToAdd->high_bits = GET_ENTRY_HIGH_BITS(nCluster);
            memcpy(fMetadata, fileToAdd, sizeof(dirEntry));

            if (EcritureCluster((void *) DISK_WRITE_LOCATION,
                                BootSector.bytes_per_sector * BootSector.sectors_per_cluster, 0, cluster) != 0) {
                printAllString("Failed to add new entry\n");
                return -1;
            }
            return 0;
        }
    }
}

int getFile(const char *filePath, char **fileContents, dirEntry *fileMeta, unsigned int readInOffset) {
    if (fileContents == NULL || fileMeta == NULL) {
        printAllString("Invalid argument\n");
        return -1;
    }

    char fileNamePart[256];
    unsigned short start = 3;
    unsigned int active_cluster;
    if (fatType == 32)
        active_cluster = ((FATBs32 *) BootSector.extended_section)->root_cluster;
    else {
        printAllString("FAT 16 and FAT 12 not supported\n");
        return -1;
    }

    dirEntry infoFile;

    unsigned int iterator = 3;
    for (iterator = 3; filePath[iterator - 1] != '\0'; iterator++) {
        if (filePath[iterator] == '\\' || filePath[iterator] == '\0') {
            memset(fileNamePart, '\0', 256);

            memcpy(fileNamePart, filePath + start, iterator - start);

            int retVal = FindDossier(fileNamePart, active_cluster, &infoFile,
                                     NULL);
            if (retVal == -2)
                return -2;
            else if (retVal == -1)
            {
                printAllString("Function getFile: An error occurred in FindDossier. Aborting...\n");
                return retVal;
            }

            start = iterator + 1;
            active_cluster = GET_CLUSTER_FROM_ENTRY(
                    infoFile);}
    }

    *fileMeta = infoFile;

    if ((infoFile.attributes & FILE_DIRECTORY) != FILE_DIRECTORY)
    {
        if (readInOffset < 1 || (readInOffset * (unsigned short) BootSector.bytes_per_sector * (unsigned short) BootSector.sectors_per_cluster) + infoFile.file_size > 262144)  return -3;
        int cluster = GET_CLUSTER_FROM_ENTRY(infoFile);
        unsigned int clusterReadCount = 0;
        while (cluster < END_CLUSTER_32) {
            LectureCluster(cluster, clusterReadCount + readInOffset);
            clusterReadCount++;
            cluster = ReadFAT(cluster);
            if (cluster == BAD_CLUSTER_32) {
                printAllString("Chain corrupted\n");
                return -1;
            } else if (cluster == -1) {
                printAllString("Error with ReadFAT\n");
                return -1;
            }
        }

        *fileContents = (char *) (DISK_READ_LOCATION + (unsigned short) BootSector.sectors_per_cluster * BootSector.bytes_per_sector *
                readInOffset);

        return 0; }
        else
        return -3;
}

int putFile(const char *filePath, char **fileContents, dirEntry *fileMeta) {
    if (TestForma(fileMeta->file_name) != 0) {
        printHexa(TestForma(fileMeta->file_name), 8);
        printAllString(fileMeta->file_name);
        printAllString("\nFile name not valid\n");
        return -2;
    }

    char fileNamePart[256];

    unsigned short start = 3;
    unsigned int active_cluster;
    if (fatType == 32)
        active_cluster = ((FATBs32 *) BootSector.extended_section)->root_cluster;
    else {
        printAllString("FAT16 and FAT12 not supported\n");
        return -1;
    }

    dirEntry infoFile;

    unsigned int iterator = 3;
    if (strcmp(filePath, "C:\\") == 0) {
        if (fatType == 32) {
            active_cluster = ((FATBs32 *) BootSector.extended_section)->root_cluster;
            infoFile.attributes = FILE_DIRECTORY | FILE_VOLUME_ID;
            infoFile.file_size = 0;
            infoFile.high_bits = GET_ENTRY_HIGH_BITS(active_cluster);
            infoFile.low_bits = GET_ENTRY_LOW_BITS(active_cluster);
        } else {
            printAllString("FAT16 and FAT12 not supported\n");
            return -1;
        }
    } else {
        for (iterator = 3; filePath[iterator - 1] != '\0'; iterator++) {
            if (filePath[iterator] == '\\' || filePath[iterator] == '\0') {
                memset(fileNamePart, '\0', 256);
                memcpy(fileNamePart, filePath + start, iterator - start);


                int retVal = FindDossier(fileNamePart, active_cluster, &infoFile, NULL);

                if (retVal == -2)
                {
                    printAllString("No matching directory found\n");
                    return -2;
                } else if (retVal == -1) //error occured
                {
                    printAllString("Error with FindDossier\n");
                    return retVal;
                }

                start = iterator + 1;
                active_cluster = GET_CLUSTER_FROM_ENTRY(infoFile);
            }
        }
    }

    char output[13];
    FromFATFormat((char *) fileMeta->file_name, output);
    int retVal = FindDossier(output, active_cluster, NULL, NULL);
    if (retVal == -1) {
        printAllString("Error with FindDossier\n");
        return -1;
    } else if (retVal != -2) {
        printAllString("Given name already exist\n");
        return -3;
    }

    if ((infoFile.attributes & FILE_DIRECTORY) == FILE_DIRECTORY)
    {
        printString(fileMeta->file_name, 11);
        printHexa(active_cluster, 8);
        printAllString("\n");
        if (AjoutDossier(active_cluster, fileMeta) != 0) {
            printAllString("Error with ajoutDossier\n");
            return -1;
        }
        char output[13];
        FromFATFormat((char *) fileMeta->file_name, output);
        printString(output, 11);
        printHexa(active_cluster, 8);
        printAllString("\n");
        retVal = FindDossier(output, active_cluster, &infoFile, NULL);
        if (retVal == -2) {
            printAllString("Error with ajoutDossier\n");
            return -2;
        } else if (retVal != 0) {
            printAllString("Error with FindDossier\n");
            return -1;
        }

        active_cluster = GET_CLUSTER_FROM_ENTRY(infoFile);

        printAllString("Cluster of Entry: ");
        printHexa(active_cluster, 8);
        printAllString("\n");
        unsigned int dataLeftToWrite = fileMeta->file_size;
        while (dataLeftToWrite > 0) {
            printHexa(dataLeftToWrite, 8);
            printAllString("\n");
            unsigned int dataWrite = 0;
            if (dataLeftToWrite >= BootSector.bytes_per_sector * BootSector.sectors_per_cluster)
                dataWrite = BootSector.bytes_per_sector * BootSector.sectors_per_cluster + 1;
            else
                dataWrite = dataLeftToWrite;

            if (EcritureCluster(*fileContents + (fileMeta->file_size - dataLeftToWrite), dataWrite, 1,
                                active_cluster) != 0) {
                printAllString("Error with PutFile\n");
                return -1;
            }

            dataLeftToWrite -= dataWrite;
            if (dataLeftToWrite == 0)
                break;
            else if (dataLeftToWrite < 0) {
                printAllString("Undefined value\n");
                return -1;
            }
            unsigned int nCluster = FreeFATAllocation();

            if ((nCluster == BAD_CLUSTER_32 && fatType == 32))
            {
                printAllString("Error FreeFATAllocation\n");
                return -1;
            }
            if (WriteFAT(active_cluster, nCluster) != 0) {
                printAllString("Error WriteFAT\n");
                return -1;
            }
            active_cluster = nCluster;
        }

        return 0;
    } else {
        printAllString("Function putFile: Invalid path!\n");
        return -2;
    }
}

char *ToFATFormat(char *input) {
    unsigned int counter = 0;

    unsigned int inputLength = strlen(input);

    while (counter < inputLength)
    {
        if ((short) input[counter] >= 97 && (short) input[counter] <= 122)
            input[counter] -= 32;
        counter++;
    }

    char searchName[13] = {'\0'};
    unsigned short dotPos = 0;

    counter = 0;
    while (counter <=
           8) {
        if (input[counter] == '.' || input[counter] == '\0') {
            dotPos = counter;
            counter++;
            break;
        } else {
            searchName[counter] = input[counter];
            counter++;
        }
    }

    if (counter > 9)
    {
        counter = 8;
        dotPos = 8;
    }

    unsigned short extCount = 8;
    while (extCount < 11)
    {
        if (input[counter] != '\0')
            searchName[extCount] = input[counter];
        else
            searchName[extCount] = ' ';

        counter++;
        extCount++;
    }

    counter = dotPos;

    while (counter <
           8)
    {
        searchName[counter] = ' ';
        counter++;
    }

    strcpy(input, searchName);

    return input;
}


BOOL TestForma(char *input) {
    short retVal = 0;


    unsigned short iterator;
    for (iterator = 0; iterator < 11; iterator++) {
        if (input[iterator] < 0x20 && input[iterator] != 0x05) {
            retVal = retVal | BAD_CHARACTER;
        }

        switch (input[iterator]) {
            case 0x2E: {
                if ((retVal & NOT_CONVERTED_YET) == NOT_CONVERTED_YET)
                    retVal |= TOO_MANY_DOTS;

                retVal ^= NOT_CONVERTED_YET;

                break;
            }
            case 0x22:
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2F:
            case 0x3A:
            case 0x3B:
            case 0x3C:
            case 0x3D:
            case 0x3E:
            case 0x3F:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x7C:
                retVal = retVal | BAD_CHARACTER;
        }

        if (input[iterator] >= 'a' && input[iterator] <= 'z') {
            retVal = retVal | LOWERCASE_ISSUE;
        }
    }
    return retVal;
}


void FromFATFormat(char *input, char *output) {

    if (input[0] == '.') {
        if (input[1] == '.') {
            strcpy (output, "..");
            return;
        }

        strcpy (output, ".");
        return;
    }

    unsigned short counter = 0;

    for (counter = 0; counter < 8; counter++) {
        if (input[counter] == 0x20) {
            output[counter] = '.';
            break;
        }

        output[counter] = input[counter];
    }

    if (counter == 8) {
        output[counter] = '.';
    }

    unsigned short counter2 = 8;

    for (counter2 = 8; counter2 < 11; counter2++) {
        ++counter;
        if (input[counter2] == 0x20 || input[counter2] == 0x20) {
            if (counter2 == 8)
                counter -= 2;
            break;
        }
        output[counter] = input[counter2];
    }

    ++counter;
    while (counter < 12) {
        output[counter] = ' ';
        ++counter;
    }

    output[12] = '\0';
}

void printHexa(unsigned long num, int digits) {

    int i, buf;
    i = 0;
    while (i < digits) {

        buf = (num >> ((digits - i - 1) * 4)) & 0xF;
        if (buf < 10)
            buf += '0';
        else
            buf += 'A' - 10;
        AfficheText(buf);
        i++;
    }

}
void printAllString(char *s) {

    while (*s) {

        AfficheText(*s);
        s++;

    }

}

void printString(char *s, int n) {

    while (n--) {

        if (*s > ' ')
            AfficheText(*s);
        else
            AfficheText('.');

        s++;

    }

}
