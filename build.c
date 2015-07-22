#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extract.h"
#include "build.h"

int build(char *in, char *out){
	char c = 0;
	char R = 0, G = 0, B = 0;
	char x_pos[5] = { 0 };
	char y_pos[5] = { 0 };
	char *names[] = { "boot", "charger", "unlocked", "start", "bootloader", "recovery", "poweroff", "fastboot_op", "oem_unlock", "unlock_yes", "unlock_no", "downloadmode" ,"oem_laf", "laf_yes", "laf_no"};	// For checking pos.txt
	char IMGDATA_HEADER1[] = { 'I', 'M', 'G', 'D', 'A', 'T', 'A', '!', 0x01, 0x00, 0x00, 0x00 };
	char IMGDATA_HEADER2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };	// Header of imgdata.img
	char temp[256] = { 0 };
	char file_path[256] = { 0 };
	char input[256] = { 0 };	// I had to add these since on linux, it is not possible to change string in char*
	char output[256] = { 0 };	// http://c-faq.com/decl/strlitinit.html
	unsigned int i = 0, j = 0, k = 0;
	unsigned int entries;
	unsigned int extra = 0;		// Stands for the extra bytes to make the size of BMP lines multiple of 4 in bytes
	unsigned int for_check = 0;	// Helps to deremine whether the BMP file is correct or not.
	BMP_PIXEL *pixels = NULL;	// Stores the pixels of a BMP file without the extra bytes.
	raw_image_header *RAW_IMAGE_HEADERS = NULL; // Stores the header infos for the images in the imgdata.img
	raw_images **RAW_IMAGES = NULL;	// Stores the pixels in RLE format for each picture.
	raw_images *cur = NULL;	// Stores the actual data of the *RAW_IMAGES[12] list.
	FILE *I = NULL;
	FILE *O = NULL;

	strcpy(input, in);
	strcpy(output, out);
/////Changes backslah to slash on Linux. Changes slash to backslash on Windows.
#if defined(_WIN32) || defined(WIN32)
    if (strchr(input, '/') != NULL){
        i = 0;
        while (input[i] != 0) {
            if (input[i] == '/'){
                input[i] = '\\';
            }
            i++;
        }
    }
#else
    if (strchr(input, '\\') != NULL){
        i = 0;
        while (input[i] != 0) {
            if (input[i] == '\\'){
                input[i] = '/';
            }
            i++;
        }
    }
#endif
#if defined(_WIN32) || defined(WIN32)
    if (strchr(output, '/') != NULL){
        i = 0;
        while (output[i] != 0) {
            if (output[i] == '/'){
                output[i] = '\\';
            }
            i++;
        }
    }
#else
    if (strchr(output, '\\') != NULL){
        i = 0;
        while (output[i] != 0) {
            if (output[i] == '\\'){
                output[i] = '/';
            }
            i++;
        }
    }
#endif
    strcpy(file_path,input);
#if defined(_WIN32) || defined(WIN32)
    if (file_path[strlen(file_path)] != '\\'){
        strcat(file_path, "\\");
    }
#else
    if (file_path[strlen(file_path)] != '/'){
        strcat(file_path, "/");
    }
#endif
    strcat(file_path, "pos.txt");
	printf("Reading pos.txt...");
	I = fopen(file_path, "rb");
	if (I == NULL){
		printf("FAIL!\nCould not open %s\n", file_path);
		return -1;
	}
	c = fgetc(I);
	while (c != '='){
		c = fgetc(I);
	}
	c = fgetc(I);
	while (c != 0x0D && c != 0xA){
		temp[j] = c;
		j++;
		c = fgetc(I);
	}
	temp[j] = '\0';
	entries = atoi(temp);
	if (c == 0x0D){	// If finds CR then skip LF to get to the new line.
		fgetc(I);
	}
	j = 0;
	pixels = (BMP_PIXEL*)malloc(sizeof(BMP_PIXEL)*entries);	
	RAW_IMAGE_HEADERS = (raw_image_header*)malloc(sizeof(raw_image_header)*entries);
	for (i = 0; i < entries; i++){
		for (j = 0; j < 16; j++){
			RAW_IMAGE_HEADERS[i].name[j] = '\0';
		}
	}
	j = 0;
	RAW_IMAGES = (raw_images**)malloc(sizeof(raw_images)*entries);
	for (i = 0; i < entries; i++){
		RAW_IMAGES[i] = NULL;
	}
	for (i = 0; i < entries; i++){	// Reads pos.txt and does checks for it.
		c = fgetc(I);
		while (c != 0x20){	// Reads the name of the images from pos.txt to the actual RAW_IMAGE_HEADERS[i].name until it reaches space
			RAW_IMAGE_HEADERS[i].name[j] = c;
			j++;
			c = fgetc(I);
			if (j > 15){	// In imgdata.img the the name of the images can't be longer than 15 characters.
				RAW_IMAGE_HEADERS[i].name[15] = '\0';
				printf("FAIL!\nUnknown name \"%s\" at line %u in %s\n", RAW_IMAGE_HEADERS[i].name, i + 1, file_path);
				fclose(I);
				return -1;
			}
		}
		RAW_IMAGE_HEADERS[i].name[j] = '\0';
		if (!(strcmp(names[i], RAW_IMAGE_HEADERS[i].name) == 0)){ // If the name is wrong the building fails.
			printf("FAIL!\nUnknown name \"%s\" at line %u in %s\n", RAW_IMAGE_HEADERS[i].name, i + 1, file_path);
			fclose(I);
			return -1;
		}
		j = 0;
		c = fgetc(I);
		while (c != 'x'){	// Reads X position until it reaches the x character
			x_pos[j] = c;
			j++;
			c = fgetc(I);
			if (j > 4){	// If still not reached the x character then it fails.
				printf("FAIL!\nCheck \"%s\" in %s\nX position must be maximum 4 digits long\nCheck for the \"x\" too\n", RAW_IMAGE_HEADERS[i].name, file_path);
				fclose(I);
				return -1;
			}
		}
		x_pos[j] = '\0';
		RAW_IMAGE_HEADERS[i].x_pos = atoi(x_pos);
		if (RAW_IMAGE_HEADERS[i].x_pos > 1079){
			printf("FAIL\nX position %u is exceeded the maximum 1079 for %s\n", RAW_IMAGE_HEADERS[i].x_pos,RAW_IMAGE_HEADERS[i].name);
			fclose(I);
			return -1;
		}
		j = 0;
		c = fgetc(I);
		while (c != 0x0D && c != 0x0A && c != -1){	// Reads Y positin until it finds CR, LF or EOF.
			y_pos[j] = c;
			j++;
			c = fgetc(I);
			if (j > 4){
				printf("FAIL!\nCheck \"%s\" .%s\nY position must be maximum 4 digits long\nCheck for new line too\n", RAW_IMAGE_HEADERS[i].name, file_path);
				fclose(I);
				return -1;
			}
		}
		y_pos[j] = '\0';
		RAW_IMAGE_HEADERS[i].y_pos = atoi(y_pos);
		if (RAW_IMAGE_HEADERS[i].y_pos > 1919){
			printf("FAIL\nY position %u is exceeded the maximum 1919 for %s\n", RAW_IMAGE_HEADERS[i].y_pos, RAW_IMAGE_HEADERS[i].name);
			fclose(I);
			return -1;
		}
		j = 0;
		if (c == 0x0D){	// If finds CR then skip LF to get to the new line.
			fgetc(I);
		}
	}
	printf("Done\n");
	fclose(I);
	for (i = 0; i < entries; i++){	// Reads the the BMP files from the given path and stores them in the memory without the extra bytes
		strcpy(file_path, input);
#if defined(_WIN32) || defined(WIN32)
        if (file_path[strlen(file_path)] != '\\'){
            strcat(file_path, "\\");
        }
#else
        if (file_path[strlen(file_path)] != '/'){
            strcat(file_path, "/");
        }
#endif
		strcat(file_path, RAW_IMAGE_HEADERS[i].name);
		strcat(file_path, ".bmp");
		I = fopen(file_path, "rb");
		if (I == NULL){
			printf("\nCould not open %s\n", file_path);
			return -1;
		}
//////////////////// Checks the BMP file //////////////////////
		printf("\nChecking %s.bmp...", RAW_IMAGE_HEADERS[i].name);
		if ((c = fgetc(I)) == 'B'){
			if ((c = fgetc(I)) != 'M'){
				printf("FAIL!\n%s is not a Windows BMP file\n", file_path);
				fclose(I);
				return -1;
			}
		}
		else{
			printf("FAIL!\n%s is not a Windows BMP file\n", file_path);
			fclose(I);
			return -1;
		}
		fread(&for_check, sizeof(unsigned int), 1, I);
		fseek(I, 0L, SEEK_END);
		if (ftell(I) != for_check){
			printf("FAIL!\n%s is a damaged Windows BMP file\nFile size mismatch: %lu bytes, expected %u bytes\n", file_path,ftell(I),for_check);
			fclose(I);
			return -1;
		}
		fseek(I, 0x0E, SEEK_SET);
		c = fgetc(I);
		if (c == 0x0C){
            printf("FAIL!\nBITMAPCOREHEADER structure is not supported\nRe-save the editet pictures with Paint, Gimp or Photoshop\n");
            fclose(I);
            return -1;
		}
		fseek(I, 0x1C, SEEK_SET);
		if ((c = fgetc(I)) != 0x18){
			printf("FAIL!\n%s is not a 24 bit Windows BMP file\n", file_path);
			fclose(I);
			return -1;
		}
		fseek(I, 0x1E, SEEK_SET);
		fread(&for_check, sizeof(unsigned int), 1, I);
		if (for_check != 0){
			printf("FAIL!\n%s is a compressed Windows BMP file\nCompressed Windows BMP files are not supported\n", file_path);
			fclose(I);
			return -1;
		}
		fseek(I, 0x12, SEEK_SET);
		fread(&RAW_IMAGE_HEADERS[i].width, sizeof(unsigned int), 1, I);
		fread(&RAW_IMAGE_HEADERS[i].height, sizeof(unsigned int), 1, I);
		if ((RAW_IMAGE_HEADERS[i].width == 0) || (RAW_IMAGE_HEADERS[i].width > 1080)){
			printf("FAIL!\n%s is too wide\nImage width is %u pixels\nMaximum witdth is 1080 pixels\n", file_path, RAW_IMAGE_HEADERS[i].width);
			fclose(I);
			return -1;
		}
		if ((RAW_IMAGE_HEADERS[i].height == 0) || (RAW_IMAGE_HEADERS[i].height > 1920)){
			printf("FAIL!\n%s is too high\nImage height is %u pixels\nMaximum height is 1920 pixels\n", file_path, RAW_IMAGE_HEADERS[i].height);
			fclose(I);
			return -1;
		}
		if (RAW_IMAGE_HEADERS[i].width + RAW_IMAGE_HEADERS[i].x_pos > 1080){
			printf("FAIL!\n%s is off the screen by %u pixel(s) horizontaly\nCheck X position and the image dimensions\n", file_path, RAW_IMAGE_HEADERS[i].width + RAW_IMAGE_HEADERS[i].x_pos - 1080);
			if (RAW_IMAGE_HEADERS[i].height + RAW_IMAGE_HEADERS[i].y_pos > 1920){
				printf("%s is off the screen by %u pixel(s) verticaly\nCheck y position and the image dimensions\n", file_path, RAW_IMAGE_HEADERS[i].height + RAW_IMAGE_HEADERS[i].y_pos - 1920);
			}
			fclose(I);
			return -1;
		}
		if(RAW_IMAGE_HEADERS[i].height + RAW_IMAGE_HEADERS[i].y_pos > 1920){
			printf("FAIL!\n%s is off the screen by %u pixel(s) verticaly\nCheck y position and the image dimensions\n", file_path, RAW_IMAGE_HEADERS[i].height + RAW_IMAGE_HEADERS[i].y_pos - 1920);
			if (RAW_IMAGE_HEADERS[i].width + RAW_IMAGE_HEADERS[i].x_pos > 1080){
				printf("%s is off the screen by %u pixel(s) horizontaly\nCheck X position and the image dimensions\n", file_path, RAW_IMAGE_HEADERS[i].width + RAW_IMAGE_HEADERS[i].x_pos - 1080);
			}
			fclose(I);
			return -1;
		}
		printf("OK!");
//////////////////////////////////////////////////////////////////
		printf("\nReading %s...", RAW_IMAGE_HEADERS[i].name);
		fseek(I,0x0A, SEEK_SET); //| Seeks to the byte which tells the start of the pixel table.
		c = fgetc(I);            //| Reads that byte
		fseek(I, c, SEEK_SET);	 //| Seeks the position to the pixel table
		if (4 - ((RAW_IMAGE_HEADERS[i].width * 3) % 4) == 4){	// Calculates the extra bytes
			extra = 0;
		}
		else{
			extra = 4 - ((RAW_IMAGE_HEADERS[i].width * 3) % 4);
		}
		pixels[i].data = (char*)malloc(sizeof(char)*(3 * RAW_IMAGE_HEADERS[i].width*RAW_IMAGE_HEADERS[i].height));	// Allocate memory for the BMP pixels
		for (j = 0; j < (3*RAW_IMAGE_HEADERS[i].width*RAW_IMAGE_HEADERS[i].height); j++){	// Reads the pixel datas from BMP file
			pixels[i].data[j] = fgetc(I);
			k++;
			if (k == 3 * RAW_IMAGE_HEADERS[i].width){	// If reaches the end of the pixel line then skips the amount of extra bytes
				fseek(I, extra, SEEK_CUR);
				k = 0;
			}
		}
		fclose(I);
		printf("Done\n");
	}
	for (i = 0; i < entries; i++){	// Converts the BMP pixels to RLE format. Fills the IMAGE_HEADERS with infos
		printf("\nConverting %s.bmp...", RAW_IMAGE_HEADERS[i].name);
		if (i == 0){
			RAW_IMAGE_HEADERS[i].offset = 0x400;	// The offset of the first image is 0x400 in imgdata.img
		}
		else{
			if (RAW_IMAGE_HEADERS[i-1].size % 512 == 0){	// Calculates the offset for the current image from the previous image size with block size.
				RAW_IMAGE_HEADERS[i].offset = RAW_IMAGE_HEADERS[i - 1].offset + RAW_IMAGE_HEADERS[i - 1].size; // If the previous image file size is multiple of the block size
			}
			else{	// If the previous image file size is not multiple of the block size
				RAW_IMAGE_HEADERS[i].offset = RAW_IMAGE_HEADERS[i - 1].offset + RAW_IMAGE_HEADERS[i - 1].size + (512 - RAW_IMAGE_HEADERS[i-1].size % 512);
			}
		}
		RAW_IMAGE_HEADERS[i].size = 0;
		cur = RAW_IMAGES[i];
		// Codes the BMP pixels to the RLE format of the imgdata.img More info http://forum.xda-developers.com/showpost.php?p=52955278&postcount=3
		for (j = 3 * RAW_IMAGE_HEADERS[i].width*RAW_IMAGE_HEADERS[i].height - 3 * RAW_IMAGE_HEADERS[i].width; j != 0; j = j - 3 * RAW_IMAGE_HEADERS[i].width){	// Codes the BMP pixels to the RLE format of the imgdata.img More info http://forum.xda-developers.com/showpost.php?p=52955278&postcount=3
			k = 0;
			while (k != 3 * RAW_IMAGE_HEADERS[i].width){
				if (k != 3 * RAW_IMAGE_HEADERS[i].width){
					if (cur == NULL){
						cur = (raw_images*)malloc(sizeof(raw_images));
						cur->count = 1;
						cur->B = pixels[i].data[j + k];
						k++;
						cur->G = pixels[i].data[j + k];
						k++;
						cur->R = pixels[i].data[j + k];
						k++;
						cur->next = NULL;
						RAW_IMAGES[i] = cur;
						RAW_IMAGE_HEADERS[i].size = RAW_IMAGE_HEADERS[i].size + 4;
					}
				}
				if (k != 3 * RAW_IMAGE_HEADERS[i].width){
					B = pixels[i].data[j + k];
					k++;
					G = pixels[i].data[j + k];
					k++;
					R = pixels[i].data[j + k];
					k++;
					if (R == cur->R && G == cur->G && B == cur->B && cur->count != (char)0xFF){
						cur->count++;
					}
					else{
						cur->next = (raw_images*)malloc(sizeof(raw_images));
						cur = cur->next;
						cur->R = R;
						cur->G = G;
						cur->B = B;
						cur->count = 1;
						cur->next = NULL;
						RAW_IMAGE_HEADERS[i].size = RAW_IMAGE_HEADERS[i].size + 4;
					}
				}
			}
		}
		k = 0;
		while (k != 3 * RAW_IMAGE_HEADERS[i].width){
			if (k != 3 * RAW_IMAGE_HEADERS[i].width){
				if (cur == NULL){
					cur = (raw_images*)malloc(sizeof(raw_images));
					cur->count = 1;
					cur->B = pixels[i].data[j + k];
					k++;
					cur->G = pixels[i].data[j + k];
					k++;
					cur->R = pixels[i].data[j + k];
					k++;
					cur->next = NULL;
					RAW_IMAGES[i] = cur;
					RAW_IMAGE_HEADERS[i].size = RAW_IMAGE_HEADERS[i].size + 4;
				}
			}
			if (k != 3 * RAW_IMAGE_HEADERS[i].width){
				B = pixels[i].data[j + k];
				k++;
				G = pixels[i].data[j + k];
				k++;
				R = pixels[i].data[j + k];
				k++;
				if (R == cur->R && G == cur->G && B == cur->B && cur->count != (char)0xFF){
					cur->count++;
				}
				else{
					cur->next = (raw_images*)malloc(sizeof(raw_images));
					cur = cur->next;
					cur->R = R;
					cur->G = G;
					cur->B = B;
					cur->count = 1;
					cur->next = NULL;
					RAW_IMAGE_HEADERS[i].size = RAW_IMAGE_HEADERS[i].size + 4;
				}
			}
		}
		printf("Done\n");
	}
    strcpy(file_path, output);
#if defined(_WIN32) || defined(WIN32)
    if (file_path[strlen(file_path)] != '\\'){
        strcat(file_path, "\\");
    }
#else
    if (file_path[strlen(file_path)] != '/'){
        strcat(file_path, "/");
    }
#endif
    strcat(file_path, "new-imgdata.img");
	O = fopen(file_path, "wb");
	if (O == NULL){
		printf("\nCould not create %s\n", file_path);
		return -1;
	}
	printf("\nWriting new-imgdata.img...");
	fwrite(&IMGDATA_HEADER1, sizeof(char), sizeof(IMGDATA_HEADER1), O);	// Writes the header of the new imgdata.img file
	fwrite(&entries, sizeof(unsigned int), 1, O);
	fwrite(&IMGDATA_HEADER2, sizeof(char), sizeof(IMGDATA_HEADER2), O);
	fwrite(RAW_IMAGE_HEADERS, sizeof(raw_image_header)* entries, 1, O);	// Writes the image headers to imgdata
	for (i = 0; i < (1000 - sizeof(raw_image_header)* entries); i++){
		fputc(0x00, O);
	}
	for (i = 0; i < entries; i++){	//	Writes out the images coded in RLE
		cur = RAW_IMAGES[i];
		while (cur != NULL){
			fputc(cur->count, O);
			fputc(cur->R, O);
			fputc(cur->G, O);
			fputc(cur->B, O);
			cur = cur->next;
		}
		if (RAW_IMAGE_HEADERS[i].size % 512 != 0){	// If the size is not multiple of the block size then writes out a certaion amount of 0x00 to fill a block.
			for (j = 0; j < (512 - RAW_IMAGE_HEADERS[i].size % 512); j++){
				fputc(0x00, O);
			}
		}
	}
	fclose(O);

	printf("Done\n");
	for (i = 0; i < entries; i++){;
		while (RAW_IMAGES[i] != NULL){
			cur = RAW_IMAGES[i];
			RAW_IMAGES[i] = RAW_IMAGES[i]->next;
			free(cur);
		}
		free(pixels[i].data);
	}

    printf("\nChecking new-imgdata.img size...");
	O = fopen(file_path, "rb");
	if (O == NULL){
		printf("FAIL!\nCant open %s\n", file_path);
		return -1;
	}
	fseek(O, 0L, SEEK_END);
	if (ftell(O) > 3145728){	// Check the file size of the new imgadata.img. It can't be bigger than 3MB.
		printf("FAIL!\n%s is too big. Maximum size is 3145728 bytes\n", file_path);
		fclose(O);
		free(RAW_IMAGE_HEADERS);
		free(pixels);
		free(RAW_IMAGES);
		return 1;
	}
	free(RAW_IMAGE_HEADERS);
	free(pixels);
	free(RAW_IMAGES);
	fclose(O);
	printf("OK!\n");
	return 0;
}
