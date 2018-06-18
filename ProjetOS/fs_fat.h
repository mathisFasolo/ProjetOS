//
//  fs_fat.h
//  ProjetOS
//
//  Created by Alexis Jolin, Etienne Platini and Mathis Fasolo
//

#ifndef fs_fat_h
#define fs_fat_h

#include <stdint.h>

// Definition de la structure bootsector
// Elle donne les caractéristique de la partition
struct bootSector {
    uint8_t JmpBoot[3]; //0-2 bytes
    char OEMName[8]; //nom des logiciels de creation de partition : 3-10 bytes
    uint16_t BytsPerSec; //nombre de byte par secteur du cluster : 11-12 bytes
    uint8_t SecPerClus; //nombre de secteurs dans un cluster : 13eme byte
    uint16_t RsvdSecCnt; //nombre de secteur réservés si 1 => FAT16 si 32 => FAT32 : 14-15 bytes
    uint8_t NumFATs; //nombre de copie de la File Attribution Table : 16 bytes
    uint16_t RootEntCnt; //nombre maximum de fichiers (512 : FAT16; 0 : FAT32) : 17-18 bytes
    uint16_t TotSec16; //taille des miximum des fichiers due à la taille des secteurs (pas defini pour FAT32) : 19-20 bytes
    uint8_t Media; //donne le type de stockage externe ou interne 21 bytes
    uint16_t FATSz16; //nombre de secteur à couvrir avec une File Allocation Table : 22-23 bytes
    uint16_t SecPerTrk; //utilisé seulement pour des media spéciaux// 24-25 bytes
    uint16_t NumHeads; //nombre de tête de lecture, en cas de disquette : 2 26-27 bytes
    uint64_t HiddSec; //nombre de secteur caché (défini le format entre autre, apporte de la precision) 28-31 bytes
    uint16_t TotSec32; //si non défini dans 19-20 32-35 bytes
    struct fat16 *f16;
    struct fat32 *f32;
};

struct fat16 {
    uint8_t DrvNum; //numero attribue à la partition 36 bytes
    uint8_t Reserved1; //réservé pour windows NT if 0 check disque if 1 scan surface 37 byte
    uint8_t BootSig; //indique que les 3 prochains champs sont remplis de 39-61 : 38 byte
    uint32_t VolID; //identifiant du volume : 39-42 bytes
    char VolLab[11]; //nom du volume ou "NO NAME    "
    char FilSysType[8]; //fat12, fat16 fat ou juste 0000000
    uint8_t BootCode[448]// bootstrap program. Remplie de 0 si dépendant de la plateforme
    uint16_t BootSign//signature permetant de vérifier que le secteur est valid
};

struct fat32 {
    uint32_t FatSz32;
    uint16_t ExtFlags;
    uint16_t FSVer;
    uint32_t RootClus;
    uint16_t FSInfo; //reserved by FAT32
    uint16_t BkBootSec
    char Reserved[12]; //reserved by FAT
    //tout comme fat16
    uint8_t DrvNum;
    uint8_t Reserved1; //réservé pour windows NT if 0 check disque if 1 scan surface 37 byte
    uint8_t BootSig; //indique que les 3 prochains champs sont remplis de 39-61 : 38 byte
    uint32_t VolId; //identifiant du volume : 39-42 bytes
    char VolLab[11]; //nom du volume ou "NO NAME    "
    char FilSysType[8];
    uint8_t BootCode32[420];
    uint16_t BootSign;
};

struct parameters {
    int FatType;
    int FatStartSector;
    int FatSector;
    int RootDirStartSector;
    int RootDirSector;
    int DataStartSector;
    int DataSectors;
};


#endif /* fs_fat_h */
