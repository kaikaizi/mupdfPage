#define _WITH_GETLINE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MaxLineSize 512
/* Use memory instead of temporary file if history_File size
 * less than 2Mb */
#define MaxMemSize 2097152
char line[MaxLineSize], *history_File, delim=0x1b; /* ESC-char */

inline int parseLine(const char* ln, const char delim){
   /* Delimits into 2 fields only, left to right */
   int ptr=0;
   while(ln[ptr] && ln[ptr++]!=delim);
   return ptr;
}

inline int seekPdf(const char* pdf_name, FILE* fp){
   /* fp opened with "r" mode and positioned at beginning */
   int pos;
   while(fgets(line,MaxLineSize,fp) && (pos=parseLine(line, 0x1b)))
	if(!strncmp(line,pdf_name,pos-1))
	   return strlen(line);
   return 0;
}

int writeEntry(const char* pdf_name, const int page_number){
   /* Format: <pdf_name>xxx */
   FILE* fp = fopen(history_File,"a+");
   if(!fp)
	return fprintf(stderr, "Error in writeFile: cannot open %s (page %d) file to write"
	     " history.\n", history_File, page_number);
   rewind(fp); int linec, pos;
   if((linec=seekPdf(pdf_name, fp))>0){			/* re-entry */
	long pos2=ftell(fp);
	fseek(fp,-linec,SEEK_CUR);
	fgets(line,MaxLineSize,fp);
	if(atoi(line+parseLine(line, 0x1b))==page_number)
	   return fclose(fp);
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp), curPos=0;
	char* mapFile;
	if(size+MaxLineSize<MaxMemSize && (mapFile=(char*)malloc(
			(size+MaxLineSize)*sizeof(char)))){
	   /* read line-edited file into memory */
	   fseek(fp, 0, SEEK_SET);
	   /* before edited line */
	   char format[16]={0};
	   if(pos2-linec){
		sprintf(format,"%%%ldc", pos2-linec);
		fscanf(fp, format, mapFile);
		curPos+=pos2-linec;
	   }
	   /* edited line */
	   curPos += snprintf(mapFile+curPos, (size_t)MaxMemSize,
		   "%s%c%d\n", pdf_name, delim, page_number);
	   fseek(fp, pos2, SEEK_SET);
	   /* after edited line */
	   if(size-pos2){
		memset(format,0,16*sizeof(char));
		sprintf(format,"%%%ldc", size-pos2);
		fscanf(fp, format, mapFile+curPos);
		curPos += size-pos2;
	   }
	   if(!(fp=freopen(history_File,"w",fp))){
		free(mapFile);
		return fprintf(stderr, "Cannot reopen file %s W.\n", history_File);
	   }
	   mapFile[curPos]=0;
	   fprintf(fp, "%s",mapFile);
	   free(mapFile);
	}
	else{ /* history file too large: create temporary file */
	   int len = strlen(history_File);
	   char history2[len+2];
	   strcpy(history2, history_File);
	   history2[len-1]='~'; history2[len]=0;
	   FILE* fp2 = fopen(history2,"w");
	   if(!fp2) return
		fprintf(stderr, "Error in writeFile: cannot create tmp file%s.\n",
			history2);
	   while(fgets(line,MaxLineSize,fp) &&
		   (pos=parseLine(line, 0x1b))>=0){
		if(!strncmp(line,pdf_name,pos-1))break;
		fputs(line, fp2);
	   }
	   fprintf(fp2, "%s%c%d\n", pdf_name, delim, page_number);
	   fseek(fp, pos2, SEEK_SET);
	   while(fgets(line,MaxLineSize,fp)) fputs(line, fp2);
	   fclose(fp2); fclose(fp);
	   rename(history2, history_File);
	}
	return fclose(fp);
   }
   else {
	fprintf(fp, "%s%c%d\n", pdf_name, delim, page_number);
	return fclose(fp);
   }
   return 0;
}

int readEntry(const char* pdf_name){
   FILE* fp = fopen(history_File,"r");
   if(!fp)return 1;
   int linec;
   if((linec=seekPdf(pdf_name, fp))>0){			/* re-entry */
	fseek(fp, -linec, SEEK_CUR);
	fgets(line,MaxLineSize,fp);
	fclose(fp);
	return atoi(line+parseLine(line, 0x1b));
   }
   fclose(fp);
   return 1;
}
