#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

enum GalaxyType {
  ELLIPTICAL = 0,
  RING,
  SPIRAL
};

typedef struct Galaxy {
  enum GalaxyType type;
  int stars_count;
  Star ** stars;
  int width;
  int height;
} Galaxy;

Galaxy generate_elliptical_galaxy(int width, int height, int star_count);

Star **random_stars(int max_radius, int size, enum Sector sector);
void jiggle_star(Star * star, int size);

double distance(Vector2 * v1, Vector2 * v2);

void render_galaxy(Galaxy * galaxy);
void write_galaxy_image(char * filename, Galaxy * galaxy);
void write_galaxy_stars(char * filename, Galaxy * galaxy);

void write_stars(FILE * file, Star ** stars, int size);


Galaxy generate_elliptical_galaxy(int width, int height, int star_count) {
  Galaxy galaxy;
  galaxy.type = ELLIPTICAL;
  galaxy.width = width;
  galaxy.height = height;
  galaxy.stars_count = star_count;
  galaxy.stars = NULL;

  int avg_dimension = width;
  if(height < width) avg_dimension = height;

  int core_radius = (avg_dimension / 10) / 2;
  int core_stars_count = star_count / 10;
  Star **core_stars = random_stars(core_radius, core_stars_count, CORE);

  int outer_core_radius = (avg_dimension / 3) / 2;
  int outer_core_stars_count = star_count / 10;
  Star **outer_core_stars = random_stars(outer_core_radius, outer_core_stars_count, OUTER_CORE);

  int branch_radius = (avg_dimension - 10) / 2;
  int branch_stars_count = star_count - outer_core_stars_count - core_stars_count;
  Star **branch_stars = random_stars(branch_radius, branch_stars_count, BRANCH);

  Star ** stars = malloc(sizeof(Star *) * star_count);
  if(stars == NULL) {
    fprintf(stderr, "Not enough memory to allocate %d stars", star_count);
    exit(1);
  }

  memcpy(stars, core_stars, core_stars_count * sizeof(Star *));
  memcpy(stars + core_stars_count, outer_core_stars, outer_core_stars_count * sizeof(Star *));
  memcpy(stars + core_stars_count + outer_core_stars_count, branch_stars, branch_stars_count * sizeof(Star *));

  // tidy up
  free(core_stars);
  free(outer_core_stars);
  free(branch_stars);
  
  galaxy.stars = stars;
  return galaxy;
}

int main(int argc, char** args) {

  Galaxy galaxy = generate_elliptical_galaxy(WIDTH, HEIGHT, 10000);
  render_galaxy(&galaxy);
  write_galaxy_image("test.png", &galaxy);
  write_galaxy_stars("test.txt", &galaxy);

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

void render_galaxy(Galaxy * galaxy) {
  if(galaxy == NULL) return;
  int width = galaxy->width;
  int height = galaxy->height;

  Vector2 centre;
  centre.x = width / 2;
  centre.y = height / 2;

  InitWindow(width, height, "Galaxy");
  SetTargetFPS(30);

  while(!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for(int i = 0; i < galaxy->stars_count; i ++) {
      Star* star = galaxy->stars[i];
      int x = centre.x - star->pos.x;
      int y = centre.y - star->pos.y;
      DrawPixel(x, y, WHITE);
    }

    EndDrawing();
  }

  CloseWindow();
}

void write_galaxy_image(char * filename, Galaxy * galaxy) {
  if(filename == NULL || galaxy == NULL) return;

  int width = galaxy->width;
  int height = galaxy->height;

  Image image = GenImageColor(width, height, BLACK);
  
  Vector2 centre;
  centre.x = width / 2;
  centre.y = height / 2;

  for(int i = 0; i < galaxy->stars_count; i ++) {
    Star * star = galaxy->stars[i];
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

  if(!ExportImage(image, filename)) {
    fprintf(stderr, "Failed to create image %s", filename);
    exit(1);
  }
  
}

void write_galaxy_stars(char * filename, Galaxy * galaxy) {
  if(filename == NULL || galaxy == NULL) return;
  FILE * file = fopen(filename, "w+");
  if(file == NULL) {
    fprintf(stderr, "Problem writing to %s", filename);
    exit(1);
  }
  write_stars(file, galaxy->stars, galaxy->stars_count);
  fclose(file);
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
