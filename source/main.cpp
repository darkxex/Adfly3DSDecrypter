#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <3ds.h>
#include <base64.h>
using namespace std;
Result http_download(const char *url)
{
	Result ret=0;
	httpcContext context;
	char *newurl=NULL;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *buf, *lastbuf;

	printf("Getting encrypted:\n %s\n Wait Please... \n",url);

	gfxFlushBuffers();

	do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
		//printf("return from httpcOpenContext: %"PRId32"\n",ret);
		gfxFlushBuffers();

		// This disables SSL cert verification, so https:// will be usable
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		//printf("return from httpcSetSSLOpt: %"PRId32"\n",ret);
		gfxFlushBuffers();

		// Enable Keep-Alive connections (on by default, pending ctrulib merge)
		// ret = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
		// printf("return from httpcSetKeepAlive: %"PRId32"\n",ret);
		// gfxFlushBuffers();

		// Set a User-Agent header so websites can identify your application
		ret = httpcAddRequestHeaderField(&context, "User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0");
		//printf("return from httpcAddRequestHeaderField: %"PRId32"\n",ret);
		gfxFlushBuffers();

		// Tell the server we can support Keep-Alive connections.
		// This will delay connection teardown momentarily (typically 5s)
		// in case there is another request made to the same server.
		ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
		//printf("return from httpcAddRequestHeaderField: %"PRId32"\n",ret);
		gfxFlushBuffers();

		ret = httpcBeginRequest(&context);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if(newurl==NULL) newurl = (char*)malloc(0x1000); // One 4K page for new URL
			if (newurl==NULL){
				httpcCloseContext(&context);
				return -1;
			}
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			url = newurl; // Change pointer to the url that we just learned
			printf("redirecting to url: %s\n",url);
			httpcCloseContext(&context); // Close this context before we try the next
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

	if(statuscode!=200){
		//printf("URL returned status: %"PRId32"\n", statuscode);
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -2;
	}

	// This relies on an optional Content-Length header and may be 0
	ret=httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return ret;
	}

	//printf("reported size: %"PRId32"\n",contentsize);
	gfxFlushBuffers();

	// Start with a single page buffer
	buf = (u8*)malloc(0x1000);
	if(buf==NULL){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -1;
	}

	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize;
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = (uint8_t *) realloc(buf, size + 0x1000);
				if(buf==NULL){
					httpcCloseContext(&context);
					free(lastbuf);
					if(newurl!=NULL) free(newurl);
					return -1;
				}
			}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		free(buf);
		return -1;
	}

	// Resize the buffer back down to our actual final size
	lastbuf = buf;
	buf = (uint8_t *)  realloc(buf, size);
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		if(newurl!=NULL) free(newurl);
		return -1;
	}

	//printf("downloaded size: %"PRId32"\n",size);

	gfxFlushBuffers();

	char* buf2 = (char *) buf ;


	string test = string(buf2);
int test1 = test.find("var ysmm = '") + 12;
int test2 = test.find("';",test1);
string ysmm = test.substr(test1, test2 - test1);


   int a =  ysmm.length();
  cout << "Decrypting:\n" << ysmm << endl;

      string left = "";
    string  right = "";

    for ( int i = 0; i < a ; i++ ) {
        if ( i % 2 == 0 ) {
            left = left + ysmm[i];
        } else {
            right = ysmm[i] + right;
        }
    }
string code = base64_decode(left + right).substr(2);


     cout << "Decrypted:\n" << code << endl;

	/*if(size>(240*400*3*2))size = 240*400*3*2;

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	*/

	httpcCloseContext(&context);
	free(buf);
	if (newurl!=NULL) free(newurl);

	return 0;
}

int main()
{

	Result ret=0;
	gfxInitDefault();
	httpcInit(0); // Buffer size when POST/PUT.
    consoleInit(GFX_TOP,NULL);
    cout << "\x1b[15;15HAdf.ly Decrypter"<<endl;
      cout << "\x1b[16;10HPress Y for open a Adfly URL."<<endl;
      cout << "\x1b[17;10HXex Code: 0.1"<<endl;
	consoleInit(GFX_BOTTOM,NULL);






	// Try the following for redirection to the above URL.
	// ret=http_download("http://tinyurl.com/hd8jwqx");
	//printf("return from http_download: %"PRId32"\n",ret);
	gfxFlushBuffers();

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		static SwkbdState swkbd;
		static char mybuf[60];
		static SwkbdStatusData swkbdStatus;
		static SwkbdLearningData swkbdLearning;
		SwkbdButton button = SWKBD_BUTTON_NONE;
		bool didit = false;


		// Your code goes here

		u32 kDown = hidKeysDown();


		if (kDown & KEY_Y)
		{
			didit = true;
			swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, -1);
			swkbdSetInitialText(&swkbd, "http://adf.ly/1V5NUR");
			swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);

			button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
		}

		if (didit)
		{
			if (button != SWKBD_BUTTON_NONE)
			{


				if (button == 2)
			{
				ret=http_download(mybuf);
			}

			} else
				printf("swkbd event: %d\n", swkbdGetResult(&swkbd));
		}


		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
	httpcExit();
	gfxExit();
	return 0;
}
