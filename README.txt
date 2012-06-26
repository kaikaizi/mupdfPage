MuPDF doesn't support page number saving/loading when exiting a
file and re-opening a file. The software is designed to be dead
simple without any config files, but I decided that saving page
numbers is a great convenience. So I added a couple of functions to
do this, in 'pdfpage.h', and modified a few lines in pdfapp.c to
achieve autosaving before exiting and autoloading before opening.

The hack was done on a svn version as of Jun 25 afternoon, and the
"pdfapp.c" file in "apps" directory was forever changing, thus it's
not feasible to create a patch. The edits in pdfapp.c are listed as
follows (the lines are inaccurate, but I try to make it easy to
find here. I am too lazy to use sed, which may not be safe after
revisions):

1. in about line 79:
void pdfapp_init(fz_context *ctx, pdfapp_t *app)
**add line**
history_File = strcat(getenv("HOME"),"/.mupdf_history");
**before the first statement:**
memset(app, 0, sizeof(pdfapp_t));
**This initializes history file.**

2. At the end of function (about line 151)
void pdfapp_open(pdfapp_t *app, char *filename, int reload)
**add line**
app->pageno = readEntry(app->doctitle);
**before last statement:**
pdfapp_showpage(app, 1, 1, 1);
**This tries to find an old entry of page number in the history file.**

3. About line 721, after
switch (c)
{
   case 'q':
	winclose(app);
	break;
**add line**
	writeEntry(app->doctitle, app->pageno);
**before line**
	winclose(app);
**This saves page number before exit application.**

4. Add header 
#include "pdfpage.h"

