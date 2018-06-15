//
//  fs_fat.h
//  ProjetOS
//
//  Created by Alexis Jolin, Etienne Platini and Mathis Fasolo
//

#ifndef fs_fat_h
#define fs_fat_h

// Definition de la structure bootsector
// Elle donne les caractéristique de la partition
struct bootSector
{
    uint8_t jump_to_boot[3]; //0-2 bytes
    char OEMName[8]; //nom des logiciels de creation de partition : 3-10 bytes
    uint16_t byteBySector; //nombre de byte par secteur du cluster : 11-12 bytes
    uint8_t sectorByCluster; //nombre de secteurs dans un cluster : 13eme byte
    uint16_t nbReservedSector; //nombre de secteur réservés si 1 => FAT16 si 32 => FAT32 : 14-15 bytes
    uint8_t nbFatCopies; //nombre de copie de la File Attribution Table : 16 bytes
    uint16_t nbRootDirEntry; //nombre maximum de fichiers (512 : FAT16; 0 : FAT32) : 17-18 bytes
    uint16_t nbSectorFs; //taille des miximum des fichiers due à la taille des secteurs (pas defini pour FAT32) : 19-20 bytes
    uint8_t mediaDescriptor; //donne le type de stockage externe ou interne 21 bytes
    uint16_t nbSectorFAT; //nombre de secteur à couvrir avec une File Allocation Table : 22-23 bytes
    uint16_t nbSectorTrack; //¯\_(ツ)_/¯ // 24-25 bytes
    uint16_t nbHead; //nombre de tête de lecture, en cas de disquette : 2 26-27 bytes
    uint16_t nbHiddenSector; //nombre de secteur caché (défini le format entre autre, apporte de la precision)
};

struct fat16
{
    uint16_t nbSecteurFS; //si non défini dans 19-20 32-35 bytes
    uint8_t nbLogicalDrive; //numero attribue à la partition 36 bytes
    uint8_t windowsNTReserved; //réservé pour windows NT if 0 check disque if 1 scan surface 37 byte
    uint8_t extendedSignature; //indique que les 3 prochains champs sont remplis de 39-61 : 38 byte
    uint32_t volumeID; //identifiant du volume : 39-42 bytes
    char volumeLabel[11]; //nom du volume ou "NO NAME    "
    char fileSystemType[8]; //fat12, fat16 fat ou juste 0000000
};

struct fat32
{
    uint32_t nbSectorInFS;
    uint16_t flag;
    uint16_t fileSystemVersion;
    uint32_t firstClusterRoot;
    uint16_t fileSystemInformation; //reserved by FAT32
    char reserved[12]; //reserved by FAT
    //tout comme fat16
    uint8_t nbLogicalDrive;
    uint8_t windowsNTReserved; //réservé pour windows NT if 0 check disque if 1 scan surface 37 byte
    uint8_t extendedSignature; //indique que les 3 prochains champs sont remplis de 39-61 : 38 byte
    uint32_t volumeID; //identifiant du volume : 39-42 bytes
    char volumeLabel[11]; //nom du volume ou "NO NAME    "
    char fileSystemType[8];
};



#endif /* fs_fat_h */
