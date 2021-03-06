#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extract.h"

int write_background(char *out, char *option){
    char BMP_HEADER[] = { 0x42, 0x4D, 0x36, 0xEC, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
                        0x00, 0x00, 0x38, 0x04, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x02, 0xEC, 0x5E, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    char output[256] = { 0 };
    char file_path[256] = { 0 };
    unsigned int i = 0;
    FILE *O = NULL;
    strcpy(output, out);
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
    strcat(file_path, option);
    strcat(file_path, ".bmp");
    O = fopen(file_path, "wb");
    if (O == NULL){
        printf("FAIL!\nFailed to open %s\n", file_path);
        return 1;
    }
    fwrite(BMP_HEADER, sizeof(char), sizeof(BMP_HEADER), O);
	fseek(O, 1080 * 1920 * 3 - 1, SEEK_CUR);
    fputc(0x00, O);
    fclose(O);
    return 0;
}

int write_image(char *in, char* out, char *option, raw_image_header RAW_IMAGE_HEADER){
    char c = 0;
	char *background = NULL;
	char *lines = NULL;
	char input[256] = { 0 };
    char output[256] = { 0 };
    char file_path[256] = { 0 };
	unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int extra = 0;
    FILE *I = NULL;
    FILE *O = NULL;

    strcpy(input, in);
    strcpy(output, out);
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
    strcat(file_path, option);
    strcat(file_path, ".bmp");
    O = fopen(file_path, "rb+");
    if (O == NULL){
        printf("FAIL!\nFailed to open %s\n", file_path);
        return 1;
    }
	fseek(O, 0x36, SEEK_SET);
	background = (char*)malloc(sizeof(char)* 1080 * 1920 * 3);
	fread(background, sizeof(char)* 1080 * 1920 * 3, 1, O);
	fseek(O, 0x36, SEEK_SET);
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
    strcat(file_path, RAW_IMAGE_HEADER.name);
    strcat(file_path, ".bmp");
    I = fopen(file_path, "rb");
    if (I == NULL){
        printf("FAIL!\nFailed to open %s\n", file_path);
        fclose(O);
        return 1;
    }
    if (4 - ((RAW_IMAGE_HEADER.width * 3) % 4) == 4){	// Calculates the extra bytes
        extra = 0;
    }
    else{
        extra = 4 - ((RAW_IMAGE_HEADER.width * 3) % 4);
    }
	fseek(I, 0x0A, SEEK_SET); //| Seeks to the byte which tells the start of the pixel table.
	c = fgetc(I);            //| Reads that byte
	fseek(I, c, SEEK_SET);	 //| Seeks the position to the pixel table
	lines = (char*)malloc((sizeof(char)* RAW_IMAGE_HEADER.width * 3 + extra) * RAW_IMAGE_HEADER.height);
	fread(lines, (sizeof(char)* RAW_IMAGE_HEADER.width * 3 + extra) * RAW_IMAGE_HEADER.height, 1, I);
	j = 0;
	k = 0;
	l = 0;
	for (i = 0; i < RAW_IMAGE_HEADER.width*RAW_IMAGE_HEADER.height * 3; i++){
		if (i == 0){
			k = k + (1919 - RAW_IMAGE_HEADER.y_pos - RAW_IMAGE_HEADER.height + 1) * 1080 * 3 + RAW_IMAGE_HEADER.x_pos * 3;
		}
		if (i % (RAW_IMAGE_HEADER.width * 3) == 0 && i != 0){
			l = l + extra;
			j = j + 1080 * 3;
			k = j + (1919 - RAW_IMAGE_HEADER.y_pos - RAW_IMAGE_HEADER.height + 1) * 1080 * 3 + RAW_IMAGE_HEADER.x_pos * 3;
		}
		background[k] = lines[l];
		l++;
		k++;
	}
	fwrite(background, sizeof(char)* 1080 * 1920 * 3, 1, O);
	free(lines);
	free(background);
    fclose(I);
    fclose(O);
    return 0;
}
int preview(char *option, char *in, char *out){
    char c = 0;
    char x_pos[5] = { 0 };
	char y_pos[5] = { 0 };
    char input[256] = { 0 };
	char temp[256] = { 0 };
    char file_path[256] = { 0 };
    char *names[] = { "boot", "charger", "locked", "unlocked", "start", "bootloader", "recovery", "poweroff", "fastboot_op", "oem_unlock", "unlock_yes", "unlock_no", "downloadmode" ,"oem_lock", "lock_yes", "lock_no","orange_power", "orange_sleep", "red_power", "red_sleep", "yellow_power", "yellow_sleep"};	// For checking pos.txt
    int for_check = 0;
    unsigned int i = 0, j = 0;
	unsigned int entries = 0;
    raw_image_header *RAW_IMAGE_HEADERS = NULL;
    FILE *I = NULL;

    if ((strcmp(option, "downloadmode") == 0 || strcmp(option, "oem-unlock-no") == 0 || strcmp(option, "oem-unlock-yes") == 0 ||
         strcmp(option, "fastboot-poweroff") == 0 || strcmp(option, "fastboot-recovery") == 0 || strcmp(option, "fastboot-bootloader") == 0 ||
         strcmp(option, "fastboot-start") == 0 || strcmp(option, "charger") == 0 || strcmp(option, "unlocked-boot") == 0 ||
         strcmp(option, "locked-boot") == 0 || strcmp(option, "oem-lock-no") == 0 || strcmp(option, "oem-lock-yes") == 0 ||
         strcmp(option, "yellow-power") == 0 || strcmp(option, "yellow-sleep") == 0 || strcmp(option, "orange-power") == 0 ||
         strcmp(option, "orange-sleep") == 0 || strcmp(option, "red-power") == 0 || strcmp(option, "red-sleep") == 0 || strcmp(option, "all") == 0) == 0)
    {
            printf("\nUnknown option: %s\n", option);
            return -1;
    }
    strcpy(input, in);

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
	RAW_IMAGE_HEADERS = (raw_image_header*)malloc(sizeof(raw_image_header)*entries);
	for (i = 0; i < entries; i++){
		for (j = 0; j < 16; j++){
			RAW_IMAGE_HEADERS[i].name[j] = '\0';
		}
	}
	j = 0;
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
		printf("OK!\n");
		fclose(I);
    }
    if (strcmp(option, "locked-boot") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for locked-boot...");
        if (write_background(out, "locked-boot") ||
            write_image(in, out, "locked-boot", RAW_IMAGE_HEADERS[0]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "unlocked-boot") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for unlocked-boot...");
        if (write_background(out, "unlocked-boot") ||
            write_image(in, out, "unlocked-boot", RAW_IMAGE_HEADERS[0]) ||
            write_image(in, out, "unlocked-boot", RAW_IMAGE_HEADERS[3]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "charger") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for charger...");
        if (write_background(out, "charger") ||
            write_image(in, out, "charger", RAW_IMAGE_HEADERS[1]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "fastboot-start") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for fastboot-start...");
        if (write_background(out, "fastboot-start") ||
            write_image(in, out, "fastboot-start", RAW_IMAGE_HEADERS[8]) ||
            write_image(in, out, "fastboot-start", RAW_IMAGE_HEADERS[4]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "fastboot-bootloader") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for fastboot-bootloader...");
        if (write_background(out, "fastboot-bootloader") ||
            write_image(in, out, "fastboot-bootloader", RAW_IMAGE_HEADERS[8]) ||
            write_image(in, out, "fastboot-bootloader", RAW_IMAGE_HEADERS[5]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "fastboot-recovery") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for fastboot-recovery...");
        if (write_background(out, "fastboot-recovery") ||
            write_image(in, out, "fastboot-recovery", RAW_IMAGE_HEADERS[8]) ||
            write_image(in, out, "fastboot-recovery", RAW_IMAGE_HEADERS[6]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "fastboot-poweroff") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for fastboot-poweroff...");
        if (write_background(out, "fastboot-poweroff") ||
            write_image(in, out, "fastboot-poweroff", RAW_IMAGE_HEADERS[8]) ||
            write_image(in, out, "fastboot-poweroff", RAW_IMAGE_HEADERS[7]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "oem-unlock-yes") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for oem-unlock-yes...");
        if (write_background(out, "oem-unlock-yes") ||
            write_image(in, out, "oem-unlock-yes", RAW_IMAGE_HEADERS[9]) ||
            write_image(in, out, "oem-unlock-yes", RAW_IMAGE_HEADERS[10]) ||
			write_image(in, out, "oem-unlock-yes", RAW_IMAGE_HEADERS[3]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "oem-unlock-no") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for oem-unlock-no...");
        if (write_background(out, "oem-unlock-no") ||
            write_image(in, out, "oem-unlock-no", RAW_IMAGE_HEADERS[9]) ||
            write_image(in, out, "oem-unlock-no", RAW_IMAGE_HEADERS[11]) ||
			write_image(in, out, "oem-unlock-no", RAW_IMAGE_HEADERS[3]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "downloadmode") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for downloadmode...");
        if (write_background(out, "downloadmode") ||
            write_image(in, out, "downloadmode", RAW_IMAGE_HEADERS[12]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "oem-lock-yes") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for oem-lock-yes...");
        if (write_background(out, "oem-lock-yes") ||
            write_image(in, out, "oem-lock-yes", RAW_IMAGE_HEADERS[13]) ||
            write_image(in, out, "oem-lock-yes", RAW_IMAGE_HEADERS[14]) ||
            write_image(in, out, "oem-lock-yes", RAW_IMAGE_HEADERS[2]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "oem-lock-no") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for oem-lock-no...");
        if (write_background(out, "oem-lock-no") ||
            write_image(in, out, "oem-lock-no", RAW_IMAGE_HEADERS[13]) ||
            write_image(in, out, "oem-lock-no", RAW_IMAGE_HEADERS[15]) ||
            write_image(in, out, "oem-lock-no", RAW_IMAGE_HEADERS[2]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "yellow-power") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for yellow-power...");
        if (write_background(out, "yellow-power") ||
            write_image(in, out, "yellow-power", RAW_IMAGE_HEADERS[20]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "yellow-sleep") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for yellow-sleep...");
        if (write_background(out, "yellow-sleep") ||
            write_image(in, out, "yellow-sleep", RAW_IMAGE_HEADERS[21]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "orange-power") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for orange-power...");
        if (write_background(out, "orange-power") ||
            write_image(in, out, "orange-power", RAW_IMAGE_HEADERS[16]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "orange-sleep") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for orange-sleep...");
        if (write_background(out, "orange-sleep") ||
            write_image(in, out, "orange-sleep", RAW_IMAGE_HEADERS[17]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "red-power") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for red-power...");
        if (write_background(out, "red-power") ||
            write_image(in, out, "red-power", RAW_IMAGE_HEADERS[18]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
    if (strcmp(option, "red-sleep") == 0 || strcmp(option, "all") == 0){
        printf("\nCreating preview for red-sleep...");
        if (write_background(out, "red-sleep") ||
            write_image(in, out, "red-sleep", RAW_IMAGE_HEADERS[19]))
        {
            return -1;
        }
        else{
            printf("Done\n");
        }
    }
	free(RAW_IMAGE_HEADERS);
    return 0;
}
