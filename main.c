#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600

enum Sector {
  CORE = 0,
  OUTER_CORE,
  BRANCH
};

const char* sector_names[] = {
  "Core",
  "Outer_Core",
  "Branch"
};

typedef struct Star {
  Vector2 pos;
  enum Sector sector;
} Star;

Star **random_stars(int max_radius, int size, enum Sector sector);
void jiggle_star(Star * star, int size);

double distance(Vector2 * v1, Vector2 * v2);

void write_stars(FILE * file, Star ** stars, int size);

int main(int argc, char** args) {

  Vector2 centre;
  centre.x = WIDTH / 2;
  centre.y = HEIGHT / 2;

  int core_radius = 50;
  int outer_core_radius = 100;
  int branch_radius = 250;

  int core_stars_count = 500;
  int outer_core_stars_count = 1000;
  int branch_stars_count = 10000;

  Star **core_stars = random_stars(core_radius, core_stars_count, CORE);
  Star **outer_core_stars = random_stars(outer_core_radius, outer_core_stars_count, OUTER_CORE);
  Star **branch_stars = random_stars(branch_radius, branch_stars_count, BRANCH);

  Image image = GenImageColor(WIDTH, HEIGHT, BLACK);
  //ImageDrawCircle(&image, centre.x, centre.y, core_radius, RED);
  //ImageDrawCircle(&image, centre.x, centre.y, outer_core_radius, YELLOW);
  //ImageDrawCircle(&image, centre.x, centre.y, branch_radius, GREEN);

  for(int i = 0; i < core_stars_count; i ++) {
    Star * star = core_stars[i];
    unsigned char val = GetRandomValue(100, 255);
    Color color;
    color.a = 255;
    color.r = val;
    color.g = val;
    color.b = val;

    int x = centre.x - star->pos.x;
    int y = centre.y - star->pos.y;
    ImageDrawPixel(&image, x, y, color);
  }
  for(int i = 0; i < outer_core_stars_count; i ++) {
    Star * star = outer_core_stars[i];
    unsigned char val = GetRandomValue(100, 255);
    Color color;
    color.a = 255;
    color.r = val;
    color.g = val;
    color.b = val;
    int x = centre.x - star->pos.x;
    int y = centre.y - star->pos.y;
    ImageDrawPixel(&image, x, y, color);
  }
  for(int i = 0; i < branch_stars_count; i ++) {
    Star * star = branch_stars[i];
    unsigned char val = GetRandomValue(100, 255);
    Color color;
    color.a = 255;
    color.r = val;
    color.g = val;
    color.b = val;
    int x = centre.x - star->pos.x;
    int y = centre.y - star->pos.y;
    ImageDrawPixel(&image, x, y, color);
  }

  if(!ExportImage(image, "test.png")) {
    printf("Failed to create image");
  }
  
  char * file_name = "test.txt";
  FILE * file = fopen(file_name, "w+");
  if(file == NULL) {
    fprintf(stderr, "Problem writing to %s", file_name);
    exit(1);
  }

  write_stars(file, core_stars, core_stars_count);
  write_stars(file, outer_core_stars, outer_core_stars_count);
  write_stars(file, branch_stars, branch_stars_count);
  fclose(file);

  return 0;
}

Star **random_stars(int max_radius, int size, enum Sector sector) {
  Star **stars = malloc(sizeof(Star *) * size);
  if(stars == NULL) exit(1);

  Vector2 centre;
  centre.x = 0;
  centre.y = 0;

  for(int i = 0; i < size; i ++) {
    Star *star = malloc(sizeof(Star));
    if(star == NULL) exit(1);
    star->sector = sector;
    stars[i] = star;
    do {
      star->pos.x = GetRandomValue(-max_radius, max_radius);
      star->pos.y = GetRandomValue(-max_radius, max_radius);
    } while(distance(&centre, &star->pos) > max_radius);
    jiggle_star(star, 10);
  }

  return stars;
}

void jiggle_star(Star * star, int size) {
  if(star == NULL) return;
  int x = star->pos.x;
  int y = star->pos.y;
  star->pos.x = GetRandomValue(x - size, x + size);
  star->pos.y = GetRandomValue(y - size, y + size);
}

double distance(Vector2 * v1, Vector2 * v2) {
  if(v1 == NULL || v2 == NULL) return 0;
  double a = pow(v2->x - v1->x, 2);
  double b = pow(v2->y - v1->y, 2);
  return sqrt(a + b);
}

void write_stars(FILE * file, Star** stars, int size) {
  if(file == NULL) return;

  for(int i = 0; i < size; i ++) {
    Star *star = stars[i];
    fprintf(
        file, 
        "%d, %d %s\n", 
        (int) star->pos.x, (int) star->pos.y, sector_names[star->sector]
    );
  }
}
