#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

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

Galaxy generate_spiral_galaxy(
    int width, 
    int height, 
    int branches, 
    int star_count, 
    float spin_factor
);

Star **random_stars(int max_radius, int size, enum Sector sector);
Star **random_branch_stars(int max_radius, int max_width, int count, float angle);

void jiggle_star(Star * star, int size);
void spin_stars(Star ** stars, int count, Vector2 centre, float spin_factor);
void spin_star(Star * star, Vector2 centre, float spin_factor);

double distance(Vector2 * v1, Vector2 * v2);
void rotate_about(Vector2 * point, Vector2 * origin, float angle);

void render_galaxy(Galaxy * galaxy);
void write_galaxy_image(char * filename, Galaxy * galaxy);
void write_galaxy_stars(char * filename, Galaxy * galaxy);

void write_stars(FILE * file, Star ** stars, int size);
void write_star(FILE * file, Star * start);


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

  memcpy(
      stars, 
      core_stars, 
      core_stars_count * sizeof(Star *)
  );
  memcpy(
      stars + core_stars_count, 
      outer_core_stars, 
      outer_core_stars_count * sizeof(Star *)
  );
  memcpy(
      stars + core_stars_count + outer_core_stars_count,
      branch_stars, 
      branch_stars_count * sizeof(Star *)
  );

  // tidy up
  free(core_stars);
  free(outer_core_stars);
  free(branch_stars);
  
  galaxy.stars = stars;
  return galaxy;
}

Galaxy generate_spiral_galaxy(
    int width, 
    int height, 
    int branches, 
    int star_count,
    float spin_factor
) {
  if(branches < 1) {
    fprintf(stderr, "branches must be a positive number, was %d", branches);
    exit(1);
  }
  Galaxy galaxy;
  galaxy.type = SPIRAL;
  galaxy.width = width;
  galaxy.height = height;
  galaxy.stars_count = star_count;
  galaxy.stars = NULL;

  int min_dim = height;
  if(height < width) min_dim = height;

  int core_radius = (min_dim / 10) / 2;
  int core_stars_count = star_count / 10;
  Star **core_stars = random_stars(core_radius, core_stars_count, CORE);

  int outer_core_radius = (min_dim / 5) / 2;
  int outer_core_stars_count = star_count / 30;
  Star **outer_core_stars = random_stars(outer_core_radius, outer_core_stars_count, OUTER_CORE);
  
  int total_branch_stars = star_count - core_stars_count - outer_core_stars_count;
  int stars_per_branch = total_branch_stars / branches;
  total_branch_stars = stars_per_branch * branches;
  float angle_per_branch = (PI * 2.f) / branches;
  int arm_width = min_dim / branches / 3;

  Star ** all_branch_stars = malloc(sizeof(Star *) * total_branch_stars);
  if(all_branch_stars == NULL) {
    fprintf(stderr, "Not enough memory for branches");
    exit(1);
  }
  
  Vector2 centre;
  centre.x = 0;
  centre.y = 0;

  for(int branch = 0; branch < branches; branch ++) {
    float angle = angle_per_branch * branch;
    Star ** branch_stars = random_branch_stars(min_dim / 2, arm_width, stars_per_branch, angle);
    //spin_stars(branch_stars, stars_per_branch, centre, spin_factor);

    // merge into single branch stars array
    memcpy(
        all_branch_stars + (branch * stars_per_branch), 
        branch_stars, 
        stars_per_branch * sizeof(Star *)
    );
    free(branch_stars);
  }

  int total_stars = core_stars_count + outer_core_stars_count + total_branch_stars;

  Star ** stars = malloc(sizeof(Star *) * total_stars);
  if(stars == NULL) {
    fprintf(stderr, "Not enough memory to allocate %d stars", total_stars);
    exit(1);
  }

  memcpy(
      stars, 
      core_stars, 
      core_stars_count * sizeof(Star *)
  );
  memcpy(
      stars + core_stars_count, 
      outer_core_stars, 
      outer_core_stars_count * sizeof(Star *)
  );
  memcpy(
      stars + core_stars_count + outer_core_stars_count, 
      all_branch_stars, 
      total_branch_stars * sizeof(Star *)
  );

  // tidy up
  free(core_stars);
  free(outer_core_stars);
  free(all_branch_stars);
  
  spin_stars(stars, total_stars, centre, spin_factor);
  
  galaxy.stars = stars;
  galaxy.stars_count = total_stars;
  return galaxy;
}

int main(int argc, char** args) {

  //Galaxy galaxy = generate_elliptical_galaxy(WIDTH, HEIGHT, 10000);
  Galaxy galaxy = generate_spiral_galaxy(WIDTH, HEIGHT, 4, 10000, 0.01f);
  render_galaxy(&galaxy);
  write_galaxy_image("test.png", &galaxy);
  write_galaxy_stars("test.txt", &galaxy);

  return 0;
}

Star **random_stars(int max_radius, int size, enum Sector sector) {
  Star **stars = malloc(sizeof(Star *) * size);
  if(stars == NULL){
    fprintf(stderr, "Could not allocate memory for %d stars", size);
    exit(1);
  }

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

Star **random_branch_stars(int max_radius, int max_width, int count, float angle) {

  Star **stars = malloc(sizeof(Star *) * count);
  if(stars == NULL){
    fprintf(stderr, "Could not allocate memory for %d stars", count);
    exit(1);
  }

  Vector2 centre;
  centre.x = 0;
  centre.y = 0;
  
  for(int i = 0; i < count; i ++) {
    Star *star = malloc(sizeof(Star));
    if(star == NULL) exit(1);
    star->sector = BRANCH;
    stars[i] = star;
    do {
      star->pos.x = GetRandomValue(-max_width, max_width);
      star->pos.y = GetRandomValue(0, max_radius);
    } while(distance(&centre, &star->pos) > max_radius);
    jiggle_star(star, 10);
    rotate_about(&star->pos, &centre, angle);
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

void spin_stars(Star ** stars, int count, Vector2 centre, float spin_factor) {
  if(stars == NULL) return;
  for(int i = 0; i < count; i ++) {
    spin_star(stars[i], centre, spin_factor);
  }
}

void spin_star(Star * star, Vector2 centre, float spin_factor) {
  if(star == NULL) return;
  double dist = distance(&star->pos, &centre);
  float angle = dist * spin_factor;
  rotate_about(&star->pos, &centre, angle);
}

double distance(Vector2 * v1, Vector2 * v2) {
  if(v1 == NULL || v2 == NULL) return 0;
  double a = pow(v2->x - v1->x, 2);
  double b = pow(v2->y - v1->y, 2);
  return sqrt(a + b);
}

void rotate_about(Vector2 * point, Vector2 * origin, float angle) {
  if(point == NULL || origin == NULL) return;
  float s = sin(angle);
  float c = cos(angle);
  point->x -= origin->x;
  point->y -= origin->y;
  float new_x = point->x * c - point->y * s;
  float new_y = point->x * s + point->y * c;
  point->x = new_x + origin->x;
  point->y = new_y + origin->y;
}

void render_galaxy(Galaxy * galaxy) {
  if(galaxy == NULL) return;

  int width = galaxy->width;
  int height = galaxy->height;

  bool rotate = false;
  float rotation_speed = 0.1f;

  Vector2 centre;
  centre.x = width / 2;
  centre.y = height / 2;

  Camera2D camera = {0};
  camera.target.x = 0;
  camera.target.y = 0;
  camera.rotation = 0;
  camera.zoom = 1.f;
  camera.offset.x = centre.x;
  camera.offset.y = centre.y;
  
  InitWindow(width, height, "Galaxy");
  SetTargetFPS(30);

  while(!WindowShouldClose()) {
    BeginDrawing();
    BeginMode2D(camera);
    ClearBackground(BLACK);

    for(int i = 0; i < galaxy->stars_count; i ++) {
      Star* star = galaxy->stars[i];
      int x = star->pos.x;
      int y = star->pos.y;
      DrawPixel(x, y, WHITE);
    }

    EndMode2D();
    EndDrawing();

    if(IsKeyPressed(KEY_Q)) {
      break;
    }
    if(IsKeyPressed(KEY_R)) {
      rotate = !rotate;
    }
    if(IsKeyDown(KEY_T)) {
      rotation_speed += 0.1f;
      if(rotation_speed > 360.f) rotation_speed = 360.f;
    }
    if(IsKeyDown(KEY_E)) {
      rotation_speed -= 0.1f;
      if(rotation_speed < 0.1f) rotation_speed = 0.1f;
    }

    if(rotate) {
      camera.rotation += rotation_speed;
      while(camera.rotation >= 360.f)
        camera.rotation = camera.rotation - 360.f;
    }
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
  fprintf(file, "%d\n", size);
  for(int i = 0; i < size; i ++) {
    Star *star = stars[i];
    write_star(file, star);
  }
}

void write_star(FILE * file, Star * star) {
  if(file == NULL || star == NULL) return;
  fprintf(
      file, 
      "%d, %d %s\n", 
      (int) star->pos.x, (int) star->pos.y, sector_names[star->sector]
  );
}
