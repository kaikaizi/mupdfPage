#!/bin/sh

if [ -f apps/pdfpage.h ]; then
   echo Patch already applied.
   exit 2
fi
if awk 'BEGIN{p1=p2=p3=0;printf("#include \"pdfpage.h\"\n")}\
/^void pdfapp_init\(.*)[	 ]*$/{if(p1==0)p1=NR}\
/memset\(app,[ 	]*0,[	 ]*sizeof\(pdfapp_t));[ 	]*$/{if(p1>0){\
p1=-NR;printf("\thistory_File = strcat(getenv(\"HOME\"),\"/.mupdf_history\");\n");}}\
/^void pdfapp_open(.*)[	 ]*$/{if(p2==0)p2=NR}\
/pdfapp_showpage\(.*);[	 ]*$/{if(p2>0){p2=-NR;printf("\tapp->pageno = readEntry(app->doctitle);\n");}}\
/^void pdfapp_onkey\(.*)[ 	]*/{if(p3==0)p3=NR}\
/winclose\(app);/{if(p3>0){p3=-NR;printf("\twriteEntry(app->doctitle, app->pageno);\n");}}\
{print}\
   END{if(p1>=0 && p2>=0 && p3>=0)exit 1}' apps/pdfapp.c > apps/pdfapp.c~; then
   mv apps/pdfapp.c~ apps/pdfapp.c
   cp mupdfPage/pdfpage.h apps
   echo Patch applied.
else
   echo Patch failed.
   echo Please read README.txt and manually patch pdfapp.c.
   exit 1
fi
