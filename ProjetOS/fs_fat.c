//
//  fs_fat.h
//  ProjetOS
//
//  Created by Alexis Jolin, Etienne Platini and Mathis Fasolo
//

#include "fs_fat.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void InitFat(struct bootSector *bs,int fd,struct parameters param){
   read(fd,bs,35);
   if(bs->RootEntCnt == 0){
       read(fd,bs->f32,476);

       if (bs->f32->BootSign != 0xAA55){
           printf("BootSector not valid, corrupted FAT");
           exit(0);
       }
       param.FatStartSector = bs->RsvdSecCnt;
       param.FatSector = bs->f32->FatSz32*bs->NumFATs;
       param.RootDirStartSector=param.FatStartSector+param.FatSector;
       param.RootDirSector=(32 * bs->RootEntCnt + bs->BytsPerSec -1)/ bs->BytsPerSec;
       param.DataStartSector=param.RootDirStartSector+param.RootDirSector;
       param.DataSectors = bs->TotSec32 - param.DataStartSector;
       param.FatType=32;
   }else{
       read(fd,bs->f16,476);

       if(bs->f16->BootSign != 0xAA55){
           printf("BootSector not valid, corrupted FAT");
           exit(0);
       }
       param.FatStartSector = bs->RsvdSecCnt;
       param.FatSector = bs->FATSz16*bs->NumFATs;
       param.RootDirStartSector=param.FatStartSector+param.FatSector;
       param.RootDirSector=(32 * bs->RootEntCnt + bs->BytsPerSec -1)/ bs->BytsPerSec;
       param.DataStartSector=param.RootDirStartSector+param.RootDirSector;
       param.DataSectors = bs->TotSec16 - param.DataStartSector;

       if(param.DataSectors/bs->SecPerClus < 4086){
           printf("FAT12 not supported");
           exit(0);
       }
       if(param.DataSectors/bs->SecPerClus>65525){
           printf("Bad FAT16");
           exit(0);
       }
       param.FatType=16;
   }
}

void AccesToFAT(struct bootSector bs,struct parameters param){

}
