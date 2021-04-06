#include <unistd.h>

#include <stdlib.h>

#include <stdio.h>

#include <stdarg.h>

#include <wchar.h>

#define PNG_DEBUG 3

#include <png.h>

#include <stdio.h>

#include <locale.h>

#include <getopt.h>

#include <math.h>


struct Png {
  png_uint_32 width, height;
  int number_of_passes;
  png_byte color_type;
  png_byte bit_depth;

  png_infop info_ptr;
  png_structp png_ptr;
  png_byte ** row_pointers;

};

struct rgba {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};

void read_png_file(char *file_name, struct Png *image) {
    int x,y;
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        printf("file could not be opened");
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        printf("file is not recognized as a PNG");
    }

    /* initialize stuff */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("png_create_read_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during init_io");
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during read_image");
    }

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for (y = 0; y < image->height; y++)
        image->row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));

    png_read_image(image->png_ptr, image->row_pointers);

    fclose(fp);
}


void write_png_file(char *file_name, struct Png *image) {
    int x,y;
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        printf("file could not be opened");
    }

    /* initialize stuff */
    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("png_create_write_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf(" error during init_io");
    }

    png_init_io(image->png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during writing header");
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during writing bytes");
    }

    png_write_image(image->png_ptr, image->row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("error during end of write");
    }

    png_write_end(image->png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);

    fclose(fp);
}



void rectangle(int x1, int y1, int x2, int y2, int thickness, struct rgba * rgba,
  struct Png * image) {
  int c;
  if (x2 < x1) {
    c = x2;
    x2 = x1;
    x1 = c;
  }
  if (y2 < y1) {
    c = y2;
    y2 = y1;
    y1 = c;
  }
  int x, y;
  for (y = y1; y < y1 + thickness; y++) {
    for (x = x1; x < x2; x++) {
      image -> row_pointers[y][x] = (uint8_t) rgba -> red;
      image -> row_pointers[y][x] = (uint8_t) rgba -> green;
      image -> row_pointers[y][x] = (uint8_t) rgba -> blue;
    }
  }
  for (y = y1; y <= y2; y++) {
    for (x = x2; x > x2 - thickness; x--) {
      image -> row_pointers[y][x] = (uint8_t) rgba -> red;
      image -> row_pointers[y][x] = (uint8_t) rgba -> green;
      image -> row_pointers[y][x] = (uint8_t) rgba -> blue;
    }
  }

  for (y = y2; y > y2 - thickness; y--) {
    for (x = x1; x < x2; x++) {
      image -> row_pointers[y][x] = (uint8_t) rgba -> red;
      image -> row_pointers[y][x] = (uint8_t) rgba -> green;
      image -> row_pointers[y][x] = (uint8_t) rgba -> blue;
    }
  }
  for (y = y2; y > y1; y--) {
    for (x = x1; x < x1 + thickness; x++) {
      image -> row_pointers[y][x] = (uint8_t) rgba -> red;
      image -> row_pointers[y][x] = (uint8_t) rgba -> green;
      image -> row_pointers[y][x] = (uint8_t) rgba -> blue;
    }
  }
}


void rectangle_filled(int x1, int y1, int x2, int y2, int thickness,
  struct rgba * rgba, struct Png * image) {
  int c;
  if (x2 < x1) {
    c = x2;
    x2 = x1;
    x1 = c;
  }
  if (y2 < y1) {
    c = y2;
    y2 = y1;
    y1 = c;
  }
  for (int y = y1 + thickness; y <= y2 - thickness; y++) {
    for (int x = x1 + thickness; x <= x2 - thickness; x++) {
      image -> row_pointers[y][x] = (uint8_t) rgba -> red;
      image -> row_pointers[y][x] = (uint8_t) rgba -> green;
      image -> row_pointers[y][x] = (uint8_t) rgba -> blue;

    }

  }
}

void frame(int choice, int thickness, struct Png *image) {
  png_uint_32 x, y;
  if (choice == 1) {
    struct rgba color;
    double n_d = round(image -> width / 50);
    int n = n_d;
    int rand_r, rand_g, rand_b;
    double m_d = round(image -> height / 50);
    int m = m_d;
    // goriz
    for (int i = 0; i < n / 2; i++) {
      double tmp_d = round(image -> width / n);
      int tmp = tmp_d;
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
      color.red = rand_r;
      color.green = rand_g;
      color.blue = rand_b;

      rectangle(tmp * i, 0, tmp * (i + 1), 50, thickness, & color, &
        image);
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
        
    }
  }
    if (choice == 2) {
    struct rgba color;
    double n_d = round(image -> width / 50);
    int n = n_d;
    int rand_r, rand_g, rand_b;
    double m_d = round(image -> height / 50);
    int m = m_d;
    // goriz
    for (int i = 0; i < n / 2; i++) {
      double tmp_d = round(image -> width / n);
      int tmp = tmp_d;
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
      color.red = rand_r;
      color.green = rand_g;
      color.blue = rand_b;

      rectangle(tmp * i, 0, tmp * (i + 1), 50, thickness, & color, &
        image);
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
        
    }
  }
    if (choice == 3) {
    struct rgba color;
    double n_d = round(image -> width / 50);
    int n = n_d;
    int rand_r, rand_g, rand_b;
    double m_d = round(image -> height / 50);
    int m = m_d;
    // goriz
    for (int i = 0; i < n / 2; i++) {
      double tmp_d = round(image -> width / n);
      int tmp = tmp_d;
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
      color.red = rand_r;
      color.green = rand_g;
      color.blue = rand_b;

      rectangle(tmp * i, 0, tmp * (i + 1), 50, thickness, & color, &
        image);
      rand_r = random() % 255 + 1;
      rand_g = random() % 255 + 1;
      rand_b = random() % 255 + 1;
        
    }
  }
}



void rotate90(int x1, int y1, int x2, int y2, struct Png * image) {
  int c;
  int row;
  if (x2 < x1) {
    c = x2;
    x2 = x1;
    x1 = c;
  }
  if (y2 < y1) {
    c = y2;
    y2 = y1;
    y1 = c;
  }
  //image **row = png_byte(malloc(sizeof(image *) * image->height));
  for (png_uint_32 i = 0; i < image -> height; i++) {
    //row[i] = png_byte(calloc(sizeof(png_byte*), image->width));
  }

  for (png_uint_32 y = 0; y < image -> height; y++) {
    for (png_uint_32 x = 0; x < image -> width; x++) {
      png_byte ** row = image -> row_pointers[y][x];

    }
  }
}

void rotate180(int x1, int y1, int x2, int y2, struct Png * image) {
  rotate90(x1, y1, x2, y2, & image);
  rotate90(x1, y1, x2, y2, & image);
}

void rotate270(int x1, int y1, int x2, int y2, struct Png * image) {
  rotate90(x1, y1, x2, y2, & image);
  rotate90(x1, y1, x2, y2, & image);
  rotate90(x1, y1, x2, y2, & image);
}
/*
void INTERFACE(){
    struct Png image;
    struct rgba rgba;
    struct rgba rgba1;
    int x1, y1, x2, y2, choice, thickness;
    int a,c,f,b,n1,n2,t;
    scanf("%d", a);
    
    char* optString = ""
   switch(a){
        case 1:
           printf("Ваш выбор:\n");
           printf("Нарисовать прямоугольник.\n");
           printf("Выбор верен?(1-Y/0-N)\n");
           scanf("%d",c);
            switch(c){
                case 0:
                    main();
                    break;
                case 1:
                    printf("Прямоугольник должен быть залитым?(1-Y/0-N)");
                    scanf("%d", f);
                    switch(f){
                        case 0:
                            printf("Введите начальные координаты(вида x1 y1 x2 y2, где х1 у1 - координаты верхнего левого угла, а х2 у2 - нижнего правого): ");
                            scanf("%d %d %d %d\n", x1, y1, x2, y2);
                            printf("Введите ширину границ прямоугольника(px): ");
                            scanf("%d %d\n", thickness);
                            printf("Введите цвет границ прямоугольника: ");
                            scanf("%d\n", &rgba);
                            rectangle(x1, y1, x2, y2, thickness, &rgba, &image);
                            break;
                        case 1:
                            printf("Введите начальные координаты(вида x1 y1 x2 y2, где х1 у1 - координаты верхнего левого угла, а х2 у2 - нижнего правого): ");
                            scanf("%d %d %d %d\n", x1, y1, x2, y2);
                            printf("Введите ширину границ прямоугольника(px): ");
                            scanf("%d %d\n", thickness);
                            printf("Введите цвет границ прямоугольника: ");
                            scanf("%d\n", &rgba);
                            printf("Введите цвет заливки прямоугольника: ");
                            scanf("%d\n", &rgba1);
                            rectangle(x1, y1, x2, y2, thickness, &rgba, &image);
                            rectangle_filled(x1, y1, x2, y2, thickness, &rgba1, &image);
                            break;
                    }
                    break;
            }
            break;
        case 2:
           printf("Ваш выбор:\n");
           printf("Сделать рамку в виде узора.\n");
           printf("Выбор верен?(1-Y/0-N)\n");
           scanf("%d",c);
           switch(c){
               case 0:
                   main();
                   break;
               case 1:
                   printf("Виды узоров:\n");
                   printf("1.Узор из пикселей рандомного цвета.\n");
                   printf("2.Узор 2.\n");
                   printf("3.Узор 3.\n");
                   printf("Ваш выбор(1/2/3): ");
                   scanf("%d\n",t);
                   switch(f){
                       case 1:
                           printf("Вы выбрали узор номер 1. Верно?(1-Y,0-N)");
                           scanf("%d\n",n1);
                           switch(n1){
                               case 0:
                                   main();
                                   break;
                               case 1:
                                   printf("Введите ширину выбранного узора: ");
                                   scanf("%d", thickness);
                                   frame1(choice, thickness, &image);
                                   break;
                           }
                           break;
                       case 2:
                           printf("Вы выбрали узор номер 2. Верно?(1-Y,0-N)");
                           scanf("%d\n",n2);
                           switch(n2){
                               case 0:
                                   main();
                                   break;
                               case 1:
                                   printf("Введите цвет выбранного узора: ");
                                   scanf("%d", &rgba);
                                   printf("Введите ширину выбранного узора: ");
                                   scanf("%d", thickness);
                                   frame2(choice, thickness, &image);
                                   break;
                           }
                           break;
                       case 3:
                           printf("Вы выбрали узор номер 3. Верно?(1-Y,0-N)");
                           scanf("%d\n",n1);
                           switch(n1){
                               case 0:
                                   main();
                                   break;
                               case 1:
                                   printf("Введите цвет выбранного узора: ");
                                   scanf("%d", &rgba);
                                   printf("Введите ширину выбранного узора: ");
                                   scanf("%d", thickness);
                                   frame3(choice, thickness, &image);
                                   break;
                           }
                           break;
                   }
                   break;
           }
       case 3:
           printf("Ваш выбор:\n");
           printf("Повернуть изображение или его часть на 90 градусов.\n");
           printf("Выбор верен?(1-Y/0-N)\n");
           scanf("%d",c);
           switch(c){
               case 0:
                   main();
                   break;
               case 1:
                   printf("Введите начальные координаты области поворота(вида x1 y1 x2 y2, где х1 у1 - координаты верхнего левого угла, а х2 у2 - нижнего правого): ");
                   scanf("%d %d %d %d\n", x1, y1, x2, y2);
                   rotate90(x1, y1, x2, y2, &image);
                   break;
           }
       case 4:
           printf("Ваш выбор:\n");
           printf("Повернуть изображение или его часть на 180 градусов.\n");
           printf("Выбор верен?(1-Y/0-N)\n");
           scanf("%d",c);
           switch(c){
               case 0:
                   main();
                   break;
               case 1:
                   printf("Введите начальные координаты области поворота(вида x1 y1 x2 y2, где х1 у1 - координаты верхнего левого угла, а х2 у2 - нижнего правого): ");
                   scanf("%d %d %d %d\n", x1, y1, x2, y2);
                   rotate180(x1, y1, x2, y2, &image);
                   break;
           }
       case 5:
           printf("Ваш выбор:\n");
           printf("Повернуть изображение или его часть на 180 градусов.\n");
           printf("Выбор верен?(1-Y/0-N)\n");
           scanf("%d",c);
           switch(c){
               case 0:
                   main();
                   break;
               case 1:
                   printf("Введите начальные координаты области поворота(вида x1 y1 x2 y2, где х1 у1 - координаты верхнего левого угла, а х2 у2 - нижнего правого): ");
                   scanf("%d %d %d %d\n", x1, y1, x2, y2);
                   rotate180(x1, y1, x2, y2, &image);
                   break;
           }
       case 6:
           printf("До свидания!");
           break;
   }
}
*/

int main(int argc, const char** argv) {
    struct Png image;
    int opt, longIndex;
    read_png_file(argv[1], &image);
    
    char* optString = "r:f:o:t:h:n:e:t:p";
    
    struct  option longOpts[] = {
    {"rectangle", required_argument, NULL, 'r'},
    {"filled_rectangle", required_argument, NULL, 'f'},
    {"frame", required_argument, NULL, 'f'},
    {"rotate90", required_argument, NULL, 'n'},
    {"rotate180", required_argument, NULL, 'e'},
    {"rotate270", required_argument, NULL, 's'},
    {"help", required_argument, NULL, 'p'},
    {NULL, no_argument, NULL, 0}
    };
    
    if(argc <= 2){
            puts("Please, read help:");
            return 0;
    }
    
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(opt != -1){
        switch(opt){
        case 'r':
                break;
        case 'f':
                break;
        case 'o':
                break;
        case 'n':
                break;
        case 'e':
                break;
        case 's':
                break;
        case 'p':
                break;
            default:
                abort();
                
        }
    }
    
/*
    printf("Что вы хотите сделать?\n");
    printf("1. Нарисовать прямоугольник.\n");
    printf("2. Сделать рамку в виде узора.\n");
    printf("3. Повернуть изображение или его часть на 90 градусов.\n");
    printf("4. Повернуть изображение или его часть на 180 градсов.\n");
    printf("5. Повернуть изображение или его часть на 270 градусов.\n");
    printf("6. Ничего\n");
    printf("Введите номер команды для выполнения: ");
    
    //INTERFACE();
    */
    write_png_file(argv[2], &image);

    return 0;
}
